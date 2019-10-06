// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	//Если оставить 0, то поверап деактивируется мгновенно
	PowerupInterval = 0.0f;

	TotalNrOfTicks = 0;

	bIsPowerupActive = false;

	//Разрешить репликацию
	SetReplicates(true);
}

void ASPowerupActor::OnTickPowerup()
{
	//Увеличить счетчик тиков
	TicksProcessed++;

	//Выполнить блюпринт-евент (сделать один тик)
	OnPowerupTicked();

	//Если количество тиков достигло максимума
	if (TicksProcessed >= TotalNrOfTicks)
	{
		//Выполнить блюпринт-евент (деактивировать поверап)
		OnExpired();

		//Установить состояние поверапа на "деактивировано".
		bIsPowerupActive = false;

		//Выполнить логику на сервере, которая параллельно выполняется на клиентах
		OnRep_PowerupActive();

		//Остановить таймер
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::OnRep_PowerupActive()
{
	//Выполнить блюпринт-евент (Передает состояние поверапа (активен или не активен))
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerupActor::ActivatePowerup(AActor* ActiveFor)
{
	//Выполнить блюпринт-евент активации поверапа, передать актора, который получил поверап
	OnActivated(ActiveFor);

	//Установить состояние поверапа на "активировано"
	bIsPowerupActive = true;

	//Выполнить логику на сервере, которая параллельно выполняется на клиентах
	OnRep_PowerupActive();

	//Если интервал таймера больше 0
	if (PowerupInterval > 0.0f)
	{
		//Запустить таймер
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	//Если интервал таймера 0 или меньше
	else
	{
		//Выполнить метод один раз.
		OnTickPowerup();
	}

}

//Реплицировать данные
void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}