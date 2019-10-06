// Fill out your copyright notice in the Description page of Project Settings.

//Класс поверапов (абилки, улучшающие показатели перса)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"

UCLASS()
class COOPGAME_API ASPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerupActor();

protected:

	//Интервал в секундах, через который срабатывает таймер TimerHandle_PowerupTick и изменяет какой-то показатель перса. 
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
		float PowerupInterval;

	//Максимальное количество тиков таймера TimerHandle_PowerupTick
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
		int32 TotalNrOfTicks;

	//Активен ли паверап? Когда значение меняется, тогда происходит репликация и запускается OnRep_PowerupActive на клиентах
	UPROPERTY(ReplicatedUsing = OnRep_PowerupActive)
		bool bIsPowerupActive;

	//Таймер срабатывания метода OnTickPowerup, который изменяет какой-либо показатель перса с интервалом PowerupInterval
	FTimerHandle TimerHandle_PowerupTick;

	//Текущее количество тиков таймера TimerHandle_PowerupTick
	int32 TicksProcessed;

	//Изменяет какой-либо параметр перса. Вызывается из ActivatePowerup(). Считает количество собственных вызовов (тиков TicksProcessed), сравнивает с TotalNrOfTicks.
	//Вызывается по таймеру TimerHandle_PowerupTick, если PowerupInterval > 0, или вызывается один раз, если иначе.
	UFUNCTION()
		void OnTickPowerup();

	//Выполняется на клиенте при репликации bIsPowerupActive. Вызывает метод OnPowerupStateChanged(). 
	//Также вызывается из ActivatePowerup() для выполнения логики на сервере.
	UFUNCTION()
		void OnRep_PowerupActive();

	//Передает состояние поверапа (активен или не активен)
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnPowerupStateChanged(bool bNewIsActive);


public:	
	
	//Вызывается из ActivatePowerup(), когда поверап активируется
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnActivated(AActor* ActiveFor);

	//Вызывается из OnTickPowerup(), когда поверап деактивируется
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnExpired();

	//Вызывается из OnTickPowerup(), когда срабатывает таймер  
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnPowerupTicked();

	//Начинает всю логику работы поверапа. Выполнится на сервере из ASPickupActor::NotifyActorBeginOverlap. 
	//Все методы внутри этого метода также выполнятся на сервере
	void ActivatePowerup(AActor* ActiveFor);

};
