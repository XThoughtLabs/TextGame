#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PhysicsLabPlayerController.generated.h"

class UPhysicsHandleComponent;
class UPrimitiveComponent;

UCLASS()
class TEXTGAME_API APhysicsLabPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    APhysicsLabPlayerController();

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    void BeginGrab();
    void EndGrab();

    bool GetMouseRay(
        FVector& OutOrigin,
        FVector& OutDirection
    ) const;

    UPROPERTY(VisibleAnywhere, Category = "Physics Lab|Interaction")
    TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;

    UPROPERTY(Transient)
    TObjectPtr<UPrimitiveComponent> GrabbedComponent;

    float GrabDistance = 0.0f;

    FVector PreviousTargetLocation = FVector::ZeroVector;
    FVector SmoothedThrowVelocity = FVector::ZeroVector;

    bool bHasPreviousTarget = false;

    float MaxGrabDistance = 5000.0f;
    float MaxThrowSpeed = 2500.0f;
    float ThrowStrength = 0.65f;
    float VelocitySmoothing = 12.0f;
};
