#include "PhysicsLabPlayerController.h"

#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CollisionShape.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

namespace
{
constexpr int32 MaxGrabSweepIterations = 3;
constexpr float GrabCollisionSkin = 1.0f;
// RAGDOLL_GRAB_V1
// RAGDOLL_FIRM_GRIP_V2
}

APhysicsLabPlayerController::APhysicsLabPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;

    PhysicsHandle =
        CreateDefaultSubobject<UPhysicsHandleComponent>(
            TEXT("PhysicsHandle")
        );
}

void APhysicsLabPlayerController::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;

    InputMode.SetLockMouseToViewportBehavior(
        EMouseLockMode::DoNotLock
    );

    InputMode.SetHideCursorDuringCapture(false);

    SetInputMode(InputMode);
}

void APhysicsLabPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!InputComponent)
    {
        return;
    }

    // --------------------------------------------------------
    // Cube interaction
    // --------------------------------------------------------

    InputComponent->BindKey(
        EKeys::LeftMouseButton,
        IE_Pressed,
        this,
        &APhysicsLabPlayerController::BeginGrab
    );

    InputComponent->BindKey(
        EKeys::LeftMouseButton,
        IE_Released,
        this,
        &APhysicsLabPlayerController::EndGrab
    );


    // --------------------------------------------------------
    // Camera look
    // --------------------------------------------------------

    InputComponent->BindKey(
        EKeys::RightMouseButton,
        IE_Pressed,
        this,
        &APhysicsLabPlayerController::BeginCameraLook
    );

    InputComponent->BindKey(
        EKeys::RightMouseButton,
        IE_Released,
        this,
        &APhysicsLabPlayerController::EndCameraLook
    );


    // --------------------------------------------------------
    // Camera zoom / dolly
    // --------------------------------------------------------

    InputComponent->BindKey(
        EKeys::MouseScrollUp,
        IE_Pressed,
        this,
        &APhysicsLabPlayerController::CameraZoomIn
    );

    InputComponent->BindKey(
        EKeys::MouseScrollDown,
        IE_Pressed,
        this,
        &APhysicsLabPlayerController::CameraZoomOut
    );
}

AActor* APhysicsLabPlayerController::GetControlledCamera() const
{
    AActor* ViewTarget = GetViewTarget();

    if (!ViewTarget)
    {
        return nullptr;
    }

    if (!ViewTarget->ActorHasTag(
        FName(TEXT("PhysicsLabCamera"))
    ))
    {
        return nullptr;
    }

    return ViewTarget;
}

void APhysicsLabPlayerController::BeginCameraLook()
{
    bCameraLookActive = true;

    bShowMouseCursor = false;

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
}

void APhysicsLabPlayerController::EndCameraLook()
{
    bCameraLookActive = false;

    bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;

    InputMode.SetLockMouseToViewportBehavior(
        EMouseLockMode::DoNotLock
    );

    InputMode.SetHideCursorDuringCapture(false);

    SetInputMode(InputMode);
}

void APhysicsLabPlayerController::UpdateCameraLook()
{
    AActor* Camera = GetControlledCamera();

    if (!Camera)
    {
        return;
    }

    float MouseDeltaX = 0.0f;
    float MouseDeltaY = 0.0f;

    GetInputMouseDelta(
        MouseDeltaX,
        MouseDeltaY
    );

    if (
        FMath::IsNearlyZero(MouseDeltaX) &&
        FMath::IsNearlyZero(MouseDeltaY)
    )
    {
        return;
    }

    FRotator Rotation =
        Camera->GetActorRotation();

    Rotation.Yaw +=
        MouseDeltaX * CameraLookSensitivity;

    Rotation.Pitch -=
        MouseDeltaY * CameraLookSensitivity;

    Rotation.Pitch = FMath::Clamp(
        Rotation.Pitch,
        CameraMinPitch,
        CameraMaxPitch
    );

    Rotation.Roll = 0.0f;

    Camera->SetActorRotation(Rotation);
}

void APhysicsLabPlayerController::UpdateCameraMovement(
    float DeltaSeconds
)
{
    AActor* Camera = GetControlledCamera();

    if (!Camera)
    {
        return;
    }

    FVector Movement = FVector::ZeroVector;

    if (IsInputKeyDown(EKeys::W))
    {
        Movement += Camera->GetActorForwardVector();
    }

    if (IsInputKeyDown(EKeys::S))
    {
        Movement -= Camera->GetActorForwardVector();
    }

    if (IsInputKeyDown(EKeys::D))
    {
        Movement += Camera->GetActorRightVector();
    }

    if (IsInputKeyDown(EKeys::A))
    {
        Movement -= Camera->GetActorRightVector();
    }

    if (IsInputKeyDown(EKeys::E))
    {
        Movement += FVector::UpVector;
    }

    if (IsInputKeyDown(EKeys::Q))
    {
        Movement -= FVector::UpVector;
    }

    if (Movement.IsNearlyZero())
    {
        return;
    }

    float Speed = CameraMoveSpeed;

    if (
        IsInputKeyDown(EKeys::LeftShift) ||
        IsInputKeyDown(EKeys::RightShift)
    )
    {
        Speed *= CameraFastMultiplier;
    }

    Movement.Normalize();

    Camera->AddActorWorldOffset(
        Movement * Speed * DeltaSeconds,
        false
    );
}

void APhysicsLabPlayerController::CameraZoomIn()
{
    AActor* Camera = GetControlledCamera();

    if (!Camera)
    {
        return;
    }

    Camera->AddActorWorldOffset(
        Camera->GetActorForwardVector()
            * CameraZoomStep,
        false
    );
}

void APhysicsLabPlayerController::CameraZoomOut()
{
    AActor* Camera = GetControlledCamera();

    if (!Camera)
    {
        return;
    }

    Camera->AddActorWorldOffset(
        Camera->GetActorForwardVector()
            * -CameraZoomStep,
        false
    );
}

bool APhysicsLabPlayerController::GetMouseRay(
    FVector& OutOrigin,
    FVector& OutDirection
) const
{
    return DeprojectMousePositionToWorld(
        OutOrigin,
        OutDirection
    );
}

FVector APhysicsLabPlayerController::GetCollisionSafeComponentCenter(
    const FVector& DesiredCenter,
    const FQuat& ComponentRotation
) const
{
    if (!GrabbedComponent)
    {
        return DesiredCenter;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return DesiredCenter;
    }

    FVector SafeCenter = GrabbedComponent->GetComponentLocation();
    FVector BodySweepExtent = FVector::ZeroVector;

    if (!GrabbedBoneName.IsNone())
    {
        FBodyInstance* Body =
            GrabbedComponent->GetBodyInstance(GrabbedBoneName);

        if (!Body || !Body->IsValidBodyInstance())
        {
            return SafeCenter;
        }

        const FBox BodyBounds = Body->GetBodyBounds();
        SafeCenter = BodyBounds.GetCenter();
        BodySweepExtent = BodyBounds.GetExtent();
    }

    const FCollisionResponseParams ResponseParams(
        GrabbedComponent->GetCollisionResponseToChannels()
    );

    FVector RemainingMove = DesiredCenter - SafeCenter;

    const AActor* GrabbedActor =
        GrabbedComponent->GetOwner();

    FComponentQueryParams QueryParams(
        SCENE_QUERY_STAT(PhysicsLabGrabSweep),
        GrabbedActor
    );

    for (
        int32 SweepIteration = 0;
        SweepIteration < MaxGrabSweepIterations;
        ++SweepIteration
    )
    {
        if (RemainingMove.IsNearlyZero())
        {
            break;
        }

        TArray<FHitResult> Hits;

        const FVector SweepEnd =
            SafeCenter + RemainingMove;

        bool bHasBlockingHit = false;

        if (GrabbedBoneName.IsNone())
        {
            bHasBlockingHit = World->ComponentSweepMulti(
                Hits,
                GrabbedComponent,
                SafeCenter,
                SweepEnd,
                ComponentRotation,
                QueryParams
            );
        }
        else
        {
            // Conservative proxy for this body, not the whole ragdoll.
            bHasBlockingHit = World->SweepMultiByChannel(
                Hits,
                SafeCenter,
                SweepEnd,
                FQuat::Identity,
                GrabbedComponent->GetCollisionObjectType(),
                FCollisionShape::MakeBox(BodySweepExtent),
                QueryParams,
                ResponseParams
            );
        }

        if (!bHasBlockingHit)
        {
            SafeCenter = SweepEnd;
            break;
        }

        const FHitResult* FirstBlockingHit = nullptr;

        for (const FHitResult& Hit : Hits)
        {
            if (!Hit.bBlockingHit)
            {
                continue;
            }

            const float MoveAlongHitNormal =
                FVector::DotProduct(
                    RemainingMove,
                    Hit.ImpactNormal
                );

            const bool bMovingAwayFromInitialContact =
                (Hit.bStartPenetrating ||
                    Hit.Time <= KINDA_SMALL_NUMBER) &&
                MoveAlongHitNormal >= -KINDA_SMALL_NUMBER;

            if (bMovingAwayFromInitialContact)
            {
                continue;
            }

            if (
                !FirstBlockingHit ||
                Hit.Time < FirstBlockingHit->Time
            )
            {
                FirstBlockingHit = &Hit;
            }
        }

        if (!FirstBlockingHit)
        {
            SafeCenter = SweepEnd;
            break;
        }

        const float MoveDistance =
            RemainingMove.Size();

        const float SafeMoveDistance =
            FMath::Max(
                0.0f,
                MoveDistance * FirstBlockingHit->Time -
                    GrabCollisionSkin
            );

        SafeCenter +=
            RemainingMove.GetSafeNormal() * SafeMoveDistance;

        const FVector RemainingAfterHit =
            RemainingMove * (
                1.0f - FMath::Clamp(
                    FirstBlockingHit->Time,
                    0.0f,
                    1.0f
                )
            );

        RemainingMove =
            FVector::VectorPlaneProject(
                RemainingAfterHit,
                FirstBlockingHit->ImpactNormal
            );
    }

    return SafeCenter;
}

void APhysicsLabPlayerController::BeginGrab()
{
    if (!PhysicsHandle || GrabbedComponent)
    {
        return;
    }

    FVector RayOrigin;
    FVector RayDirection;

    if (!GetMouseRay(RayOrigin, RayDirection))
    {
        return;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    const FVector TraceEnd =
        RayOrigin + RayDirection * MaxGrabDistance;

    FHitResult HitResult;

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    bool bHit =
        World->LineTraceSingleByChannel(
            HitResult,
            RayOrigin,
            TraceEnd,
            ECC_Visibility,
            QueryParams
        );

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

    FHitResult PhysicsHit;
    const bool bPhysicsHit = World->LineTraceSingleByObjectType(
        PhysicsHit,
        RayOrigin,
        TraceEnd,
        ObjectParams,
        QueryParams
    );

    if (bPhysicsHit && (!bHit || PhysicsHit.Time < HitResult.Time))
    {
        HitResult = PhysicsHit;
        bHit = true;
    }

    if (!bHit)
    {
        return;
    }

    UPrimitiveComponent* HitComponent =
        HitResult.GetComponent();

    if (!HitComponent)
    {
        return;
    }

    const bool bSkeletalGrab =
        Cast<USkeletalMeshComponent>(HitComponent) != nullptr;

    const FName HitBone =
        bSkeletalGrab ? HitResult.BoneName : NAME_None;

    if (bSkeletalGrab && HitBone.IsNone())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Physics Lab: skeletal hit has no physics bone.")
        );
        return;
    }

    if (!HitComponent->IsSimulatingPhysics(HitBone))
    {
        return;
    }

    GrabDistance = FVector::Distance(
        RayOrigin,
        HitResult.ImpactPoint
    );

    FTransform GrabFrame = HitComponent->GetComponentTransform();

    if (bSkeletalGrab)
    {
        FBodyInstance* HitBody = HitComponent->GetBodyInstance(HitBone);

        if (!HitBody || !HitBody->IsValidBodyInstance())
        {
            return;
        }

        GrabFrame = HitBody->GetUnrealWorldTransform();
    }

    const FVector HitLocalPoint =
        GrabFrame.InverseTransformPosition(HitResult.ImpactPoint);

    bSavedHandleSoftLinearConstraint =
        PhysicsHandle->bSoftLinearConstraint;

    bSavedHandleInterpolateTarget =
        PhysicsHandle->bInterpolateTarget;

    if (bSkeletalGrab)
    {
        // Hold the selected point firmly; do not lock its rotation.
        PhysicsHandle->bSoftLinearConstraint = false;
        PhysicsHandle->bInterpolateTarget = false;
    }

    PhysicsHandle->GrabComponentAtLocation(
        HitComponent,
        HitBone,
        HitResult.ImpactPoint
    );

    if (PhysicsHandle->GetGrabbedComponent() != HitComponent)
    {
        PhysicsHandle->bSoftLinearConstraint =
            bSavedHandleSoftLinearConstraint;

        PhysicsHandle->bInterpolateTarget =
            bSavedHandleInterpolateTarget;

        return;
    }

    GrabbedComponent = HitComponent;
    GrabbedBoneName = HitBone;

    LocalGrabPoint = HitLocalPoint;

    PreviousTargetLocation =
        HitResult.ImpactPoint;

    SmoothedThrowVelocity =
        FVector::ZeroVector;

    bHasPreviousTarget = false;

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Physics Lab: grabbed %s; bone=%s"),
        *GetNameSafe(HitComponent->GetOwner()),
        *GrabbedBoneName.ToString()
    );
}

void APhysicsLabPlayerController::Tick(
    float DeltaSeconds
)
{
    Super::Tick(DeltaSeconds);

    // --------------------------------------------------------
    // Camera
    // --------------------------------------------------------

    UpdateCameraMovement(DeltaSeconds);

    if (bCameraLookActive)
    {
        UpdateCameraLook();
    }


    // --------------------------------------------------------
    // Grabbed physics object
    // --------------------------------------------------------

    if (
        !PhysicsHandle ||
        !GrabbedComponent ||
        !PhysicsHandle->GetGrabbedComponent()
    )
    {
        return;
    }

    FVector RayOrigin;
    FVector RayDirection;

    if (!GetMouseRay(RayOrigin, RayDirection))
    {
        return;
    }

    const FVector DesiredGrabPoint =
        RayOrigin + RayDirection * GrabDistance;

    FVector SafeTargetLocation = PreviousTargetLocation;

    if (!GrabbedBoneName.IsNone())
    {
        FBodyInstance* Body =
            GrabbedComponent->GetBodyInstance(GrabbedBoneName);

        if (!Body || !Body->IsValidBodyInstance() ||
            !GrabbedComponent->IsSimulatingPhysics(GrabbedBoneName))
        {
            EndGrab();
            return;
        }

        const FBox BodyBounds = Body->GetBodyBounds();
        const FVector Extent = BodyBounds.GetExtent();

        if (!BodyBounds.IsValid || Extent.ContainsNaN() ||
            Extent.X <= 0.0 || Extent.Y <= 0.0 || Extent.Z <= 0.0)
        {
            EndGrab();
            return;
        }

        const FVector CurrentGrabPoint =
            Body->GetUnrealWorldTransform().TransformPosition(LocalGrabPoint);

        const FVector GrabPointOffset =
            CurrentGrabPoint - BodyBounds.GetCenter();

        // No extra cursor-following delay for the ragdoll.
        // Blocking geometry still limits the target.
        const FVector SafeCenter = GetCollisionSafeComponentCenter(
            DesiredGrabPoint - GrabPointOffset,
            FQuat::Identity
        );

        SafeTargetLocation = SafeCenter + GrabPointOffset;
    }
    else
    {
        // Existing single-body cube targeting is unchanged.
        const FQuat ComponentRotation =
            GrabbedComponent->GetComponentQuat();

        const FVector GrabPointOffset =
            ComponentRotation.RotateVector(LocalGrabPoint);

        const FVector DesiredCenter =
            DesiredGrabPoint - GrabPointOffset;

        const FVector SafeCenter = GetCollisionSafeComponentCenter(
            DesiredCenter,
            ComponentRotation
        );

        SafeTargetLocation = SafeCenter + GrabPointOffset;
    }

    PhysicsHandle->SetTargetLocation(
        SafeTargetLocation
    );

    if (
        bHasPreviousTarget &&
        DeltaSeconds > SMALL_NUMBER
    )
    {
        FVector RawVelocity =
            (SafeTargetLocation - PreviousTargetLocation)
            / DeltaSeconds;

        RawVelocity =
            RawVelocity.GetClampedToMaxSize(
                MaxThrowSpeed
            );

        SmoothedThrowVelocity =
            FMath::VInterpTo(
                SmoothedThrowVelocity,
                RawVelocity,
                DeltaSeconds,
                VelocitySmoothing
            );

        SmoothedThrowVelocity =
            SmoothedThrowVelocity.GetClampedToMaxSize(
                MaxThrowSpeed
            );
    }

    PreviousTargetLocation = SafeTargetLocation;
    bHasPreviousTarget = true;
}

void APhysicsLabPlayerController::EndGrab()
{
    if (!PhysicsHandle || !GrabbedComponent)
    {
        return;
    }

    UPrimitiveComponent* ReleasedComponent =
        GrabbedComponent;

    PhysicsHandle->ReleaseComponent();

    PhysicsHandle->bSoftLinearConstraint =
        bSavedHandleSoftLinearConstraint;

    PhysicsHandle->bInterpolateTarget =
        bSavedHandleInterpolateTarget;

    if (
        ReleasedComponent &&
        ReleasedComponent->IsSimulatingPhysics(GrabbedBoneName)
    )
    {
        ReleasedComponent->WakeAllRigidBodies();

        if (GrabbedBoneName.IsNone())
        {
            ReleasedComponent->AddImpulse(
                SmoothedThrowVelocity * ThrowStrength,
                NAME_None,
                false
            );
        }
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Physics Lab: object released.")
    );

    GrabbedComponent = nullptr;
    GrabbedBoneName = NAME_None;

    GrabDistance = 0.0f;

    LocalGrabPoint = FVector::ZeroVector;

    PreviousTargetLocation =
        FVector::ZeroVector;

    SmoothedThrowVelocity =
        FVector::ZeroVector;

    bHasPreviousTarget = false;
}
