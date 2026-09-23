#include "PhysicsLabPlayerController.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

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

    const bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        RayOrigin,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

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

    if (!HitComponent->IsSimulatingPhysics())
    {
        return;
    }

    GrabDistance = FVector::Distance(
        RayOrigin,
        HitResult.ImpactPoint
    );

    PhysicsHandle->GrabComponentAtLocation(
        HitComponent,
        HitResult.BoneName,
        HitResult.ImpactPoint
    );

    GrabbedComponent = HitComponent;

    PreviousTargetLocation = HitResult.ImpactPoint;
    SmoothedThrowVelocity = FVector::ZeroVector;
    bHasPreviousTarget = false;

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Physics Lab: cube grabbed.")
    );
}

void APhysicsLabPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

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

    const FVector TargetLocation =
        RayOrigin + RayDirection * GrabDistance;

    PhysicsHandle->SetTargetLocation(
        TargetLocation
    );

    if (
        bHasPreviousTarget &&
        DeltaSeconds > SMALL_NUMBER
    )
    {
        FVector RawVelocity =
            (TargetLocation - PreviousTargetLocation)
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

    PreviousTargetLocation = TargetLocation;
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

    if (
        ReleasedComponent &&
        ReleasedComponent->IsSimulatingPhysics()
    )
    {
        ReleasedComponent->WakeAllRigidBodies();

        ReleasedComponent->AddImpulse(
            SmoothedThrowVelocity * ThrowStrength,
            NAME_None,
            true
        );
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Physics Lab: cube released.")
    );

    GrabbedComponent = nullptr;

    GrabDistance = 0.0f;
    PreviousTargetLocation = FVector::ZeroVector;
    SmoothedThrowVelocity = FVector::ZeroVector;
    bHasPreviousTarget = false;
}
