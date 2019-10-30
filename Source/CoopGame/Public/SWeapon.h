// Fill out your copyright notice in the Description page of Project Settings.

//родительский класс всех классов оружия. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShake;

//Структура для хранения данных трейсинга, которые реплицируются
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:
	//физический материал поверхности, которую достигает трейсинг
	UPROPERTY()
		TEnumAsByte<EPhysicalSurface> SurfaceType;

	//конечная точка трейсинга
	UPROPERTY()
		FVector_NetQuantize TraceTo;
};

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASWeapon();

protected:

	//Меш
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* MeshComp;

	//Класс компонента для описания повреждений - задается в редакторе как тип повреждений
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;

	//Название сокета на оружии
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName MuzzleSocketName;

	//Имя параметра частиц эффекта трейсинга при выстреле TracerEffect
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName TracerTargetName;

	//Частицы эффекта выстрела на оружии
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* MuzzleEffect;

	//Дефолтные эффекты выстрела на поверхности
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* DefaultImpactEffect;

	//Эффекты выстрела на поверхности с определенным материалом
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* FleshImpactEffect;

	//Эффект трейсинга при выстреле - линия по лучу трейсинга
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* TracerEffect;

	//Длина трейсинга
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float LenghtTracing;

	//Базовый дамаг
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float BaseDamage;

	//Тряска камеры при выстреле
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<UCameraShake> FireCamShake;

	//Частота выстрела - количество пуль в 1 минуту
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float RateOfFire;

	//Структура для репликации 
	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
		FHitScanTrace HitScanTrace;

	//Угол разброса пуль оружия (в градусах). Для того, чтобы выстрел не был идеально точным.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin=0.0f))
		float BulletSpread;

protected:

	//Таймер для автоматического выстрела
	FTimerHandle TimerHandle_TimeBetweenShots;

	//Время последнего выстрела
	float LastFireTime;

	//Время между выстрелами
	float TimeBetweenShots;

protected:

	//Выстрел на сервере
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire();

	//Выполняется на клиенте, когда HitScanTrace реплицируется 
	UFUNCTION()
		void OnRep_HitScanTrace();

protected:

	virtual void BeginPlay() override;

	//Показать эффекты от выстрела
	void PlayFireEffects(FVector TracerEndPoint);

	//Показать эффекты от выстрела на поверхности
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	//Выстрел
	virtual void Fire();

public:

	//Начать стрельбу
	void StartFire();

	//Закончить стрельбу
	void StopFire();

};
