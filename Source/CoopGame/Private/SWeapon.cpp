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

//Консольная команда для отрисовки трейсинга (дебаг-линии)
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDriwing(
	TEXT("COOP.DebugWeaponDrawing"), 
	DebugWeaponDrawing, 
	TEXT("Draw debug trace line"), 
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	//Создать меш
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));

	//Назначить рутом
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;
	RateOfFire = 600;
	LenghtTracing = 10000;
	
	//Объект можно передавать по сети
	SetReplicates(true);

	//Как часто этот объект будет реплицироваться
	NetUpdateFrequency = 66.0f;

	//Как часто этот объект будет реплицироваться при нечастом изменении реплицируемых свойств
	MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}


void ASWeapon::Fire()
{
	//Если выполняется на клиенте
	if (Role < ROLE_Authority)
	{
		//Выполнить на сервере
		ServerFire();
	}

	//Взять овнера - владельца оружия
	AActor* MyOwner = GetOwner();
	if (MyOwner == nullptr)
	{
		return;
	}
	   
	//Структура для данных, получаемых при трейсинге
	FHitResult Hit;

	//точка "глаз" овнера
	FVector EyeStart;

	//Направление взгляда
	FRotator EyeRotation;

	//Определить вектор и ротатор EyeStart и EyeRotation
	MyOwner->GetActorEyesViewPoint(EyeStart, EyeRotation);

	//Единичный вектор, направленный в сторону ротатора EyeRotation
	FVector ShotDirection = EyeRotation.Vector();

	//Найти конечную точку трейсинга
	FVector TraceEnd = EyeStart + (ShotDirection * LenghtTracing);

	//Параметры коллизии
	FCollisionQueryParams QueryParams;

	//Игнорировать Овнера
	QueryParams.AddIgnoredActor(MyOwner);

	//Игнорировать себя
	QueryParams.AddIgnoredActor(this);

	//Трейсить комплексную коллизию (собственную коллизию меша)
	QueryParams.bTraceComplex = true;

	//Возвращать физический материал при результате
	QueryParams.bReturnPhysicalMaterial = true;

	//Запомнить для изменения
	FVector TracerEndPoint = TraceEnd;

	//Определить физ. материал по умолчанию
	EPhysicalSurface SurfaceType = SurfaceType_Default;

	//Трейсинг, если удался
	if (GetWorld()->LineTraceSingleByChannel(Hit, EyeStart, TraceEnd, COLLISION_WEAPON, QueryParams))
	{
		//Получить попавшегося актора
		AActor* HitActor = Hit.GetActor();

		//Получить физ. материал 
		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		//Текущий урон
		float CurrentDamage = BaseDamage;

		//Если заданный материал, то увеличить урон (хедшот)
		if (SurfaceType == SURFACE_FLESHVULNERABLE)
		{
			CurrentDamage *= 4.0f;
		}

		//Применить урон к HitActor
		UGameplayStatics::ApplyPointDamage(HitActor, CurrentDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

		//Получить текущую точку трейсинга
		TracerEndPoint = Hit.ImpactPoint;

		//Показать эффекты от выстрела на поверхности
		PlayImpactEffects(SurfaceType, TracerEndPoint);
	}

	//Если задано в консоли, отрисовать дебаг-линию
	if (DebugWeaponDrawing > 0)
	{
		DrawDebugLine(GetWorld(), EyeStart, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
	}

	//Показать эффекты от выстрела на оружии
	PlayFireEffects(TracerEndPoint);

	//Если это сервер, заполнить структуру для передачи по клиентам
	if (Role == ROLE_Authority)
	{
		HitScanTrace.TraceTo = TracerEndPoint;
		HitScanTrace.SurfaceType = SurfaceType;
	}

	//Запомнить время последнего выстрела
	LastFireTime = GetWorld()->TimeSeconds;
}

//Выполнять на сервере
void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

//Выполнять на сервере?
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
	/*
	Рассчитать задержку перед работой таймера, ограничить результат >=0.0f. Если перерыв между стрельбой (LastFireTime - GetWorld()->TimeSeconds)
	больше, чем время между выстрелами TimeBetweenShots, то задержка = 0. 
	Если перерыв меньше, то будет небольшая задержка больше 0 и меньше, чем TimeBetweenShots
	*/
	float FirstDelay = FMath::Max( TimeBetweenShots + LastFireTime - GetWorld()->TimeSeconds, 0.0f);

	//Запустить таймер, по которому выполняется Fire() - стрельба
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	//Удалить таймер
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	//Спавнить приатаченные к MeshComp частицы в сокете
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	//Спавнить частицы эффекта трейсинга
	if (TracerEffect)
	{
		//Взять точку сокета
		FVector MuzzleSocketLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		//Спавнить частицы в сокете
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleSocketLocation);

		if (TracerComp)
		{
			//Вытянуть частицы до точки конца трейсинга
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	//Создать эффект тряски камеры
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
	//Объявить частицы
	UParticleSystem* SelectImpact = nullptr;

//Задать частицы в зависимости от физ. материала поверхности
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

		//Получить ротатор в сторону стрельбы
		FRotator ShotRotatorDirection = ShotVectorDirection.Rotation();

		//Спавн частиц на поверхности 
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectImpact, ImpactPoint, ShotRotatorDirection);
	}

}

//Реплицировать переменные на клиенты
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Реплицировать на всех, кроме клиента-овнера оружия
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
