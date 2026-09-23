#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PhysicsLabGameMode.generated.h"

UCLASS()
class TEXTGAME_API APhysicsLabGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APhysicsLabGameMode();

protected:
    virtual void BeginPlay() override;
};
