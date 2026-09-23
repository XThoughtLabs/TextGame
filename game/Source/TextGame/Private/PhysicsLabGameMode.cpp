#include "PhysicsLabGameMode.h"

#include "PhysicsLabCube.h"

#include "Camera/CameraActor.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerController.h"

APhysicsLabGameMode::APhysicsLabGameMode()
{
    DefaultPawnClass = nullptr;
}

void APhysicsLabGameMode::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    constexpr float FloorTopZ = 0.0f;

    // --------------------------------------------------------
    // Floor
    // --------------------------------------------------------

    AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(
        FVector(0.0f, 0.0f, FloorTopZ - 25.0f),
        FRotator::ZeroRotator
    );

    if (Floor)
    {
        UStaticMeshComponent* FloorMesh = Floor->GetStaticMeshComponent();

        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(
            nullptr,
            TEXT("/Engine/BasicShapes/Cube.Cube")
        );

        if (CubeMesh)
        {
            FloorMesh->SetStaticMesh(CubeMesh);
        }

        FloorMesh->SetWorldScale3D(FVector(12.0f, 12.0f, 0.5f));
        FloorMesh->SetCollisionProfileName(TEXT("BlockAll"));
        FloorMesh->SetSimulatePhysics(false);
    }

    // --------------------------------------------------------
    // Physics cube
    // --------------------------------------------------------

    World->SpawnActor<APhysicsLabCube>(
        FVector(0.0f, 0.0f, FloorTopZ + 350.0f),
        FRotator(20.0f, 25.0f, 10.0f)
    );

    // --------------------------------------------------------
    // Directional light
    // --------------------------------------------------------

    ADirectionalLight* Light = World->SpawnActor<ADirectionalLight>(
        FVector::ZeroVector,
        FRotator(-45.0f, -35.0f, 0.0f)
    );

    if (Light && Light->GetLightComponent())
    {
        Light->GetLightComponent()->SetMobility(EComponentMobility::Movable);
        Light->GetLightComponent()->SetIntensity(8.0f);
    }

    // --------------------------------------------------------
    // Camera
    // --------------------------------------------------------

    const FVector CameraLocation(-750.0f, -550.0f, 400.0f);
    const FVector LookAtLocation(0.0f, 0.0f, 100.0f);

    const FRotator CameraRotation =
        (LookAtLocation - CameraLocation).Rotation();

    ACameraActor* Camera = World->SpawnActor<ACameraActor>(
        CameraLocation,
        CameraRotation
    );

    if (Camera)
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PC->SetViewTarget(Camera);
        }
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Physics Lab scene created from C++.")
    );
}
