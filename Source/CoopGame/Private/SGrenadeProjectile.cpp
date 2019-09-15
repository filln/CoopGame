// Fill out your copyright notice in the Description page of Project Settings.

#include "SGrenadeProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "DrawDebugHelpers.h"
#include "CoopGame.h"

// Sets default values
ASGrenadeProjectile::ASGrenadeProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	//Создать меш
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));

	//Включить физику
	MeshComp->SetSimulatePhysics(true);

	//Назначить рутом
	RootComponent = MeshComp;

	//Создать объект компонента 
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

	//Назначить компонент, к которому применяется движение
	ProjectileMovement->UpdatedComponent = MeshComp;

	//Начальная скорость
	ProjectileMovement->InitialSpeed = 3000.f;

	//Максимальная скорость
	ProjectileMovement->MaxSpeed = 3000.f;

	//Вращать снаряд при движении
	ProjectileMovement->bRotationFollowsVelocity = true;

	//Симулировать отскок снаряда от поверхности
	ProjectileMovement->bShouldBounce = true;

	DamageRadius = 100.0f;

	//Назначить объект снаряда реплицируемым
	SetReplicates(true);

	//Реплицировать движение объекта (ProjectileMovement)
	SetReplicateMovement(true);

}

//// Called when the game starts or when spawned
//void ASGrenadeProjectile::BeginPlay()
//{
//	Super::BeginPlay();
//	
//}
//
//// Called every frame
//void ASGrenadeProjectile::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

void ASGrenadeProjectile::Explode()
{
	//Назначить Овнера для метода ApplyRadialDamage - тот, кто выпустил снаряд
	AActor* MyOwner = GetOwner();
	if (MyOwner == nullptr)
	{
		DEBUGMESSAGE("MyOwner == nullptr");
		return;
	}
	if (ExplodeEffect == nullptr)
	{
		return;
	}

	//Спавн частиц взрыва
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeEffect, GetActorLocation());

	//Применить урон к объектам в радиусе DamageRadius
	UGameplayStatics::ApplyRadialDamage(GetWorld(), 100.0f, GetActorLocation(), DamageRadius, DamageType, IgnoreActors, this, MyOwner->GetInstigatorController());

	//Нарисовать сферу для дебага - показывает сферу повреждений
	DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 12, FColor::Yellow, false, 1.0f);
	
	//Уничтожить снаряд
	Destroy();

}

