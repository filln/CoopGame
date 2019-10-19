// Fill out your copyright notice in the Description page of Project Settings.

/*
Описывает правила игры. По каким правилам спавнить ботов. Проверяет состояние игроков (по ХП). 
По состоянию игроков определяет конец игры и респавн игроков.
Боты спавнятся волнами. Первая волна - минимальное количество ботов. Каждая следующая волна - еще больше ботов.
Волна закончится, когда все боты волны будут уничтожены.

*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

/**
 *
 */

//Перечисление состояний спавна ботов. Определено в SGameState.h.
enum class EWaveState : uint8;

//Объявление диспатчера. Вызывается, когда актор убит.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
FOnActorKilled,					//Тип структуры диспатчера.
AActor*, VictimActor,			//Жертва.
AActor*, KillerActor,			//Киллер.
AController*, KillerController  //Контроллер киллера.
);

UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASGameMode();

protected:

	//Время между волнами спавна ботов. Используется в TimerHandle_NextWaveStart.
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
		float TimeBetweenWaves;

	//Максимальное количество ботов, которое может быть спавнено за 1 волну.
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
		uint32 MaxNrOfBotsToSpawn;

public:

	//Определение диспатчера. Вызывается, когда актор убит.
	UPROPERTY(BlueprintAssignable, Category = "GameMode")
		FOnActorKilled OnActorKilled;

protected:

	//Таймер запуска метода void SpawnBotTimerElapsed() - спавн новых ботов.
	FTimerHandle TimerHandle_BotSpawner;

	//Таймер запуска метода void StartWave() - отсчитывает время до новой волны.
	FTimerHandle TimerHandle_NextWaveStart;

	//Количество ботов, которое необходимо заспавнить в текущей волне. Рассчитывается в зависимости от WaveCount.
	uint32 NrOfBotsToSpawn;

	//Порядковый номер текущей волны (или количество прошедших волн). От него зависит количество ботов в 
	//текущей волне, поэтому при достижении MaxNrOfBotsToSpawn количество волн перестает считаться.
	uint32 WaveCount;

protected:

	//Блюпринт-евент, в котором происходит создание объекта класса бота.
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
		void SpawnNewBot();

protected:

	//Старт новой волны.
	void StartWave();

	//Остановка волны (остановка спавна ботов волны).
	void EndWave();

	//Подготавливает к следующей волне. Запускает таймер TimerHandle_NextWaveStart, респавнит умерших персов методом RestartDeadPlayers().
	//Запускается из CheckWaveState().
	void PrepareForNextWave();

	//Запускается таймером TimerHandle_BotSpawner. Спавнит ботов в количестве NrOfBotsToSpawn.
	void SpawnBotTimerElapsed();

	//Проверяет, остались ли боты на карте. Если нет, то вызывает PrepareForNextWave(). Вызывается из таймера Tick().
	void CheckWaveState();

	//Проверяет, остались ли персы на карте. Если нет, то запускает GameOver(). Вызывается из таймера Tick().
	void CheckAnyPlayerAlive();

	//Конец игры. Запускает EndWave() и выводит сообщение на экран. Персы не заспавнятся заново из CheckWaveState(), 
	//если все они убиты и на карте есть боты.
	void GameOver();

	//Устанавливает состояние игры через enum class EWaveState в классе ASGameState. 
	void SetGameState(EWaveState NewState);

	//Респавнит персов. Вызывается из PrepareForNextWave().
	void RestartDeadPlayers();

public:

	//Перегружает Super::StartPlay(), которая вызывает BeginPlay(). Вызывает PrepareForNextWave().
	virtual void StartPlay() override;

	//Перегружает Super::Tick(DeltaSeconds).
	virtual void Tick(float DeltaSeconds) override;
};
