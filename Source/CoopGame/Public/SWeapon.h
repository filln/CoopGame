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
	virtual void BeginPlay() override;

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

	//Показать эффекты от выстрела
	void PlayFireEffects(FVector TracerEndPoint);

	//Показать эффекты от выстрела на поверхности
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	//Тряска камеры при выстреле
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<UCameraShake> FireCamShake;

	//Выстрел
	virtual void Fire();

	//Выстрел на сервере
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire();

	//Таймер для автоматического выстрела
	FTimerHandle TimerHandle_TimeBetweenShots;

	//Время последнего выстрела
	float LastFireTime;

	//Частота выстрела - количество пуль в 1 минуту
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float RateOfFire;

	//Время между выстрелами
	float TimeBetweenShots;

	//Структура для репликации 
	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
		FHitScanTrace HitScanTrace;

	//Выполняется на клиенте, когда HitScanTrace реплицируется 
	UFUNCTION()
		void OnRep_HitScanTrace();

public:	

	//Начать стрельбу
	void StartFire();

	//Закончить стрельбу
	void StopFire();

};
