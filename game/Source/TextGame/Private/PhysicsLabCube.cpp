#include "PhysicsLabCube.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APhysicsLabCube::APhysicsLabCube()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube")
    );

    if (CubeMesh.Succeeded())
    {
        Mesh->SetStaticMesh(CubeMesh.Object);
    }

    Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    Mesh->SetSimulatePhysics(true);

    Mesh->SetMassOverrideInKg(
        NAME_None,
        50.0f,
        true
    );

    Mesh->SetLinearDamping(0.15f);
    Mesh->SetAngularDamping(0.10f);
}