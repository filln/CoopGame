 // Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDriwing(
	TEXT("COOP.DebugWeaponDrawing"), 
	DebugWeaponDrawing, 
	TEXT("Draw debug trace line"), 
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;
	RateOfFire = 600;
	
	//Объект можно передавать по сети
	SetReplicates(true);

	//Частота обновления сети - от этого зависит скорость передачи данных
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60 / RateOfFire;
}


void ASWeapon::Fire()
{

	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner == nullptr)
	{
		return;
	}
	   
	FHitResult Hit;

	FVector EyeStart;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeStart, EyeRotation);

	FVector ShotDirection = EyeRotation.Vector();
	FVector TraceEnd = EyeStart + (ShotDirection * 10000);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(MyOwner);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	FVector TracerEndPoint = TraceEnd;

	EPhysicalSurface SurfaceType = SurfaceType_Default;

	if (GetWorld()->LineTraceSingleByChannel(Hit, EyeStart, TraceEnd, COLLISION_WEAPON, QueryParams))
	{
		AActor* HitActor = Hit.GetActor();

		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
		float CurrentDamage = BaseDamage;
		if (SurfaceType == SURFACE_FLESHVULNERABLE)
		{
			CurrentDamage *= 4.0f;
		}

		UGameplayStatics::ApplyPointDamage(HitActor, CurrentDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

		TracerEndPoint = Hit.ImpactPoint;

		//Показать эффекты от выстрела на поверхности
		PlayImpactEffects(SurfaceType, TracerEndPoint);
	}

	if (DebugWeaponDrawing > 0)
	{
		DrawDebugLine(GetWorld(), EyeStart, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
	}

	//Показать эффекты от выстрела на оружии
	PlayFireEffects(TracerEndPoint);

	//Если это сервер, заполнить структуру для передачи на него
	if (Role == ROLE_Authority)
	{
		HitScanTrace.TraceTo = TracerEndPoint;
		HitScanTrace.SurfaceType = SurfaceType;
	}

	LastFireTime = GetWorld()->TimeSeconds;
}

//Выполнять на сервере
void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

//Проверять доступность сервера?
bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::OnRep_HitScanTrace()
{
	//Показать эффекты от выстрела на оружии
	PlayFireEffects(HitScanTrace.TraceTo);

	//Показать эффекты от выстрела на поверхности
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);

}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleSocketLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleSocketLocation);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectImpact = nullptr;

//Задать частицы в зависимости от материала поверхности
	switch (SurfaceType)
	{
	case SUFRACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectImpact = FleshImpactEffect;
		break;
	default:
		SelectImpact = DefaultImpactEffect;
		break;
	}

	if (SelectImpact)
	{
		//Положение сокета на оружии
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		//Вектор, направленный в сторону стрельбы
		FVector ShotVectorDirection = ImpactPoint - MuzzleLocation;
		ShotVectorDirection.Normalize();
		//Ротатор в сторону стрельбы
		FRotator ShotRotatorDirection = ShotVectorDirection.Rotation();
		//Спавн частиц на поверхности 
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectImpact, ImpactPoint, ShotRotatorDirection);
	}

}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
