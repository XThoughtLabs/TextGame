#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsLabCube.generated.h"

class UStaticMeshComponent;

UCLASS()
class TEXTGAME_API APhysicsLabCube : public AActor
{
    GENERATED_BODY()

public:
    APhysicsLabCube();

private:
    UPROPERTY(VisibleAnywhere, Category = "Physics Lab")
    TObjectPtr<UStaticMeshComponent> Mesh;
};