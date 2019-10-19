// Fill out your copyright notice in the Description page of Project Settings.

//Меняет состояния игры с помощью enum class EWaveState и реплицирует эти
//состояния на клиенты.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

//Объявить перечесление состояний игры.
UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,	   //Ожидание старта новой волны. Отсчитывается время до старта новой волны.

	WaveInProgress,	   //Волна в прогрессе. Боты спавнятся таймером &ASGameMode::SpawnBotTimerElapsed.

	WaitingToComplete, //Ожидание готовности. Заспавнилось максимальное для волны количество ботов.

	WaveComplete,	   //Волна окончена и на карте нет ботов.

	GameOver		   //Конец игры.
};


/**
 *
 */
UCLASS()
class COOPGAME_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:

	//Определить перечисление состояний игры. Реплицируется с вызовом void OnRep_WaveState() на клиентах.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
		EWaveState WaveState;

protected:

	//Вызывается на клиентах при репликации EWaveState WaveState и на сервере в методе void SetWaveState().
	//Передает состояния игры.
	UFUNCTION()
		void OnRep_WaveState(EWaveState OldState);

	//Блюпринт-евент для передачи состояний игры. Вызывается из void OnRep_WaveState()
	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
		void WaveStateChanged(EWaveState NewState, EWaveState OldState);

public:

	//Установка состояний игры.
	void SetWaveState(EWaveState NewState);

};
