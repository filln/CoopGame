// Fill out your copyright notice in the Description page of Project Settings.

//Класс бота, атакующего перса. ИИ реализовано в основном за счет поиска точки на НавМеше и движения в сторону точки с помощью метода AddForce() в Tick().

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Меш
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		UStaticMeshComponent* MeshComp;

	//Компонент для управления дамагом и ХП
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		USHealthComponent* HealthComp;

	//Коллизия для поиска перса в методе virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		USphereComponent* SphereComp;

	//Сила движения. Отвечает также за скорость бота
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float MovementForce;

	//Менять ли скорость при приложении силы?
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		bool bUseVelocityChange;

	//Минимальная дистанция до точки, при которой точка считается достигнутой
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float RequiredDistanceToTarget;

	//Эффект взрыва бота
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		UParticleSystem* ExplosionEffect;

	//Дамаг внешним объектам при взрыве бота
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float ExplosionDamage;

	//Радиус взрыва бота
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float ExplosionRadius;

	//Звук перед взрывом бота, играет когда найден перс
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		USoundCue* SelfDestructSound;

	//Звук взрыва бота
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		USoundCue* ExplosionSound;

	//Интервал времени в секундах, при котором бот наносит себе повреждения для самовзрыва. После того, как найдет перса. 
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float SelfDamageInterval;

	//Макс. мощность взрыва
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		int32 MaxPowerLevel;

	//Текущая мощность взрыва. Увеличивается, если рядом найдена копия бота
	int32 PowerLevel;

	//Следующая точка, к которой будет двигаться бот
	FVector NextPathPoint;

	//ИнстансМатериал, накладываемый, когда бот найдет перса или другого бота
	UMaterialInstanceDynamic* MatInst;

	//Будет ли взорван бот или был ли взорван? Для того, чтобы не выполнять цепочку поведения (искать - двигаться - взрываться) после того как 
	//подан сигнал самовзрыва 	void SelfDestruct()
	bool bExploded;

	//Запущен ли таймер TimerHandle_DamageSelf нанесения самоповреждений void DamageSelf().
	//Показывает также, был ли найден перс
	bool bStartedSelfDestruction;

	//Таймер нанесения самоповреждений void DamageSelf()
	FTimerHandle TimerHandle_DamageSelf;


	//Евент для нанесения повреждений боту от перса
	UFUNCTION()
		void HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType,
			class AController* InstigatedBy, AActor* DamageCauser);

	//Поиск следующей точки следования по пути к персу
	FVector GetNextPathPoint();

	//Применить дамаг к боту
	void DamageSelf();

	//Саморазрушение бота с нанесением повреждений окружающим объектам
	void SelfDestruct();

	//Поиск других копий бота для изменения мощности взрыва и изменения материалИнстанса (сигнал о том, что копия найдена)
	void OnCheckNearbyBots();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Перегрузка евента оверлапа перса ботом. Запускает таймер самоповреждений и звук.
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;


};
