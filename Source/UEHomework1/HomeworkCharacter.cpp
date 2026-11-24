#include "HomeworkCharacter.h"

#include "EnhancedInputComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "Perception/AISense_Hearing.h"

AHomeworkCharacter::AHomeworkCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AHomeworkCharacter::BeginPlay()
{
    Super::BeginPlay();

    WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    Stamina = MaxStamina;
}

void AHomeworkCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsSprinting)
    {
        if (GetVelocity().SizeSquared2D() > 100.0f && GetWorld()->GetTimeSeconds() >= NextNoiseTime)
        {
            UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 1.0f, this, 2200.0f, TEXT("Sprint"));
            NextNoiseTime = GetWorld()->GetTimeSeconds() + 0.6f;
        }
        Stamina = FMath::Max(0.0f, Stamina - StaminaDrainRate * DeltaTime);

        if (Stamina <= 0.0f)
        {
            StopSprint();
        }
    }
    else
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRecoveryRate * DeltaTime);
    }

    if (GEngine && IsLocallyControlled())
    {
        GEngine->AddOnScreenDebugMessage(
            static_cast<uint64>(GetUniqueID()),
            1.0f,
            FColor::Green,
            FString::Printf(TEXT("Stamina: %.0f / %.0f"), Stamina, MaxStamina)
        );
    }
}

void AHomeworkCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);

    if (!Input)
    {
        return;
    }

    if (SprintAction)
    {
        Input->BindAction(SprintAction, ETriggerEvent::Started, this, &AHomeworkCharacter::StartSprint);
        Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHomeworkCharacter::StopSprint);
        Input->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AHomeworkCharacter::StopSprint);
    }

    if (InteractAction)
    {
        Input->BindAction(InteractAction, ETriggerEvent::Started, this, &AHomeworkCharacter::Interact);
    }
}

void AHomeworkCharacter::StartSprint()
{
    if (Stamina <= 0.0f)
    {
        return;
    }

    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AHomeworkCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AHomeworkCharacter::Interact()
{
    OnInteraction.Broadcast();
}
