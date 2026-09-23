#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PhysicsLabPlayerController.generated.h"

class AActor;
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
    // --------------------------------------------------------
    // Physics interaction
    // --------------------------------------------------------

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
    float MaxThrowSpeed = 1600.0f;
    float ThrowStrength = 8.0f;
    float VelocitySmoothing = 12.0f;


    // --------------------------------------------------------
    // Free camera
    // --------------------------------------------------------

    void BeginCameraLook();
    void EndCameraLook();

    void CameraZoomIn();
    void CameraZoomOut();

    void UpdateCameraMovement(float DeltaSeconds);
    void UpdateCameraLook();

    AActor* GetControlledCamera() const;

    bool bCameraLookActive = false;

    float CameraMoveSpeed = 1800.0f;
    float CameraFastMultiplier = 4.0f;

    float CameraLookSensitivity = 0.60f;

    float CameraZoomStep = 500.0f;

    float CameraMinPitch = -89.0f;
    float CameraMaxPitch = 89.0f;
};
