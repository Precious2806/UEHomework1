#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Perception/AIPerceptionTypes.h"
#include "GuardAI.generated.h"

class UTextRenderComponent;

UCLASS()
class UEHOMEWORK1_API AGuardCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AGuardCharacter();

    UPROPERTY(VisibleAnywhere)
    UTextRenderComponent* StateLabel;
};

UCLASS()
class UEHOMEWORK1_API AGuardController : public AAIController
{
    GENERATED_BODY()

public:
    AGuardController();
    virtual void OnPossess(APawn* InPawn) override;

    FVector LastNoiseLocation = FVector::ZeroVector;
    float LastNoiseTime = -1000.0f;
    float LastSeenTime = -1000.0f;
    float Suspicion = 0.0f;

private:
    UFUNCTION()
    void HearPlayer(AActor* Actor, FAIStimulus Stimulus);
};

UCLASS()
class UEHOMEWORK1_API UBTService_GuardAwareness : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_GuardAwareness();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};

UCLASS()
class UEHOMEWORK1_API UBTTask_GuardDestination : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_GuardDestination();

    UPROPERTY(EditAnywhere, Category = "Search")
    bool bInvestigateNoise = false;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
