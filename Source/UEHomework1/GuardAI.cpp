#include "GuardAI.h"

#include "Animation/AnimInstance.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "UObject/ConstructorHelpers.h"

AGuardCharacter::AGuardCharacter()
{
    AIControllerClass = AGuardController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    bUseControllerRotationYaw = false;
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 450.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 240.0f;

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> Mesh(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    static ConstructorHelpers::FClassFinder<UAnimInstance> Animation(
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
    GetMesh()->SetSkeletalMesh(Mesh.Object);
    GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -96), FRotator(0, -90, 0));
    GetMesh()->SetAnimInstanceClass(Animation.Class);

    StateLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StateLabel"));
    StateLabel->SetupAttachment(GetRootComponent());
    StateLabel->SetRelativeLocation(FVector(0, 0, 130));
    StateLabel->SetHorizontalAlignment(EHTA_Center);
    StateLabel->SetWorldSize(24.0f);
    StateLabel->SetText(FText::FromString(TEXT("PATROL")));
    StateLabel->SetTextRenderColor(FColor::Green);
}

AGuardController::AGuardController()
{
    UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
    SetPerceptionComponent(*Perception);

    UAISenseConfig_Hearing* Hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing"));
    Hearing->HearingRange = 2200.0f;
    Hearing->SetMaxAge(7.0f);
    Hearing->DetectionByAffiliation.bDetectEnemies = true;
    Hearing->DetectionByAffiliation.bDetectFriendlies = true;
    Hearing->DetectionByAffiliation.bDetectNeutrals = true;
    Perception->ConfigureSense(*Hearing);
    Perception->OnTargetPerceptionUpdated.AddDynamic(this, &AGuardController::HearPlayer);
}

void AGuardController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    UBehaviorTree* Tree = LoadObject<UBehaviorTree>(nullptr, TEXT("/Game/AI/BT_Guard.BT_Guard"));
    if (Tree && RunBehaviorTree(Tree))
    {
        GetBlackboardComponent()->SetValueAsVector(TEXT("HomeLocation"), InPawn->GetActorLocation());
    }
}

void AGuardController::HearPlayer(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor != UGameplayStatics::GetPlayerPawn(this, 0) || !Stimulus.WasSuccessfullySensed())
    {
        return;
    }

    LastNoiseLocation = Stimulus.StimulusLocation;
    LastNoiseTime = GetWorld()->GetTimeSeconds();
    Suspicion = FMath::Min(1.0f, Suspicion + 0.35f);
}

UBTService_GuardAwareness::UBTService_GuardAwareness()
{
    NodeName = TEXT("Update sight, hearing and suspicion");
    Interval = 0.2f;
    RandomDeviation = 0.02f;
    bNotifyTick = true;
    bCallTickOnSearchStart = true;
}

void UBTService_GuardAwareness::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AGuardController* Guard = Cast<AGuardController>(OwnerComp.GetAIOwner());
    AGuardCharacter* Character = Guard ? Cast<AGuardCharacter>(Guard->GetPawn()) : nullptr;
    APawn* Player = UGameplayStatics::GetPlayerPawn(&OwnerComp, 0);
    UBlackboardComponent* Board = OwnerComp.GetBlackboardComponent();
    if (!Guard || !Character || !Player || !Board)
    {
        return;
    }

    const float Now = Character->GetWorld()->GetTimeSeconds();
    const FVector ToPlayer = Player->GetActorLocation() - Character->GetActorLocation();
    const bool bWasChasing = Board->GetValueAsObject(TEXT("TargetActor")) != nullptr;
    const float SightRadius = bWasChasing ? 1800.0f : 1300.0f;
    const bool bInView = bWasChasing || FVector::DotProduct(
        Character->GetActorForwardVector(), ToPlayer.GetSafeNormal2D()) > 0.35f;
    const bool bVisible = ToPlayer.SizeSquared2D() <= FMath::Square(SightRadius)
        && bInView && Guard->LineOfSightTo(Player);

    if (bVisible)
    {
        Guard->LastSeenTime = Now;
        Guard->Suspicion = 1.0f;
        Board->SetValueAsObject(TEXT("TargetActor"), Player);
    }
    else if (!bWasChasing || Now - Guard->LastSeenTime > 0.7f || ToPlayer.Size2D() > 1800.0f)
    {
        Board->ClearValue(TEXT("TargetActor"));
        Guard->Suspicion = FMath::Max(0.0f, Guard->Suspicion - DeltaSeconds * 0.18f);
    }

    const bool bChasing = Board->GetValueAsObject(TEXT("TargetActor")) != nullptr;
    const bool bSearching = !bChasing && Now - Guard->LastNoiseTime < 7.0f && Guard->Suspicion > 0.0f;
    Board->SetValueAsBool(TEXT("Investigating"), bSearching);
    Board->SetValueAsVector(TEXT("NoiseLocation"), Guard->LastNoiseLocation);
    Board->SetValueAsFloat(TEXT("Suspicion"), Guard->Suspicion);
    float Speed = 240.0f;
    FString State = TEXT("PATROL");
    FColor Color = FColor::Green;
    if (bChasing)
    {
        Speed = 600.0f;
        State = TEXT("CHASE");
        Color = FColor::Red;
    }
    else if (bSearching)
    {
        Speed = 330.0f;
        State = TEXT("SEARCH");
        Color = FColor::Yellow;
    }

    Character->GetCharacterMovement()->MaxWalkSpeed = Speed;
    Character->StateLabel->SetText(FText::FromString(FString::Printf(
        TEXT("%s  %d%%"), *State, FMath::RoundToInt(Guard->Suspicion * 100))));
    Character->StateLabel->SetTextRenderColor(Color);
    if (APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(Character, 0))
    {
        Character->StateLabel->SetWorldRotation(
            (Camera->GetCameraLocation() - Character->StateLabel->GetComponentLocation()).Rotation());
    }
}

UBTTask_GuardDestination::UBTTask_GuardDestination()
{
    NodeName = TEXT("Choose reachable patrol or search point");
}

EBTNodeResult::Type UBTTask_GuardDestination::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* Board = OwnerComp.GetBlackboardComponent();
    UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(OwnerComp.GetWorld());
    if (!Controller || !Controller->GetPawn() || !Board || !Navigation)
    {
        return EBTNodeResult::Failed;
    }

    FVector Center = Board->GetValueAsVector(bInvestigateNoise ? TEXT("NoiseLocation") : TEXT("HomeLocation"));
    float Radius = 1100.0f;
    if (bInvestigateNoise)
    {
        const float Distance = FVector::Dist2D(Controller->GetPawn()->GetActorLocation(), Center);
        const float Suspicion = Board->GetValueAsFloat(TEXT("Suspicion"));
        Radius = FMath::Clamp(Distance * 0.25f + (1.0f - Suspicion) * 350.0f, 100.0f, 650.0f);
    }

    FNavLocation ProjectedCenter;
    FNavLocation Destination;
    if (Navigation->ProjectPointToNavigation(Center, ProjectedCenter, FVector(250, 250, 300))
        && Navigation->GetRandomReachablePointInRadius(ProjectedCenter.Location, Radius, Destination))
    {
        Board->SetValueAsVector(TEXT("Destination"), Destination.Location);
        return EBTNodeResult::Succeeded;
    }
    return EBTNodeResult::Failed;
}
