#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HomeworkCharacter.generated.h"

class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractionTriggered);

UCLASS()
class UEHOMEWORK1_API AHomeworkCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AHomeworkCharacter();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* SprintAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction = nullptr;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FInteractionTriggered OnInteraction;

    UPROPERTY(BlueprintReadOnly, Category = "Sprint")
    bool bIsSprinting = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint", meta = (ClampMin = "0.0"))
    float SprintSpeed = 900.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "1.0"))
    float MaxStamina = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    float Stamina = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
    float StaminaDrainRate = 20.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
    float StaminaRecoveryRate = 15.0f;

protected:
    virtual void BeginPlay() override;

private:
    float WalkSpeed = 600.0f;
    float NextNoiseTime = 0.0f;

    void StartSprint();
    void StopSprint();
    void Interact();
};
