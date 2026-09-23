#include "PhysicsLabGameMode.h"

#include "PhysicsLabPlayerController.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

APhysicsLabGameMode::APhysicsLabGameMode()
{
    DefaultPawnClass = nullptr;

    PlayerControllerClass =
        APhysicsLabPlayerController::StaticClass();
}

void APhysicsLabGameMode::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> LabCameras;

    UGameplayStatics::GetAllActorsWithTag(
        World,
        FName(TEXT("PhysicsLabCamera")),
        LabCameras
    );

    if (LabCameras.Num() > 0)
    {
        if (APlayerController* PC =
            World->GetFirstPlayerController())
        {
            PC->SetViewTarget(LabCameras[0]);
        }
    }
    else
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Physics Lab camera was not found.")
        );
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Physics Lab runtime started.")
    );
}
