// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"
#include "SHealthComponent.h"
#include "SGameState.h"
#include "SPlayerState.h"

ASGameMode::ASGameMode()
{

	//Разрешить Tick().
	PrimaryActorTick.bCanEverTick = true;

	//Интервал тика.
	PrimaryActorTick.TickInterval = 1.0f;

	//Время в секундах.
	TimeBetweenWaves = 2.0f;

	//Количество штук.
	MaxNrOfBotsToSpawn = 50;

	//Взять ссылку на класс ASGameState, для метода GetGameState<ASGameState>().
	GameStateClass = ASGameState::StaticClass();

	//Взять ссылку на класс ASPlayerState, для совместной работы ASPlayerState и ASGameMode.
	PlayerStateClass = ASPlayerState::StaticClass();

}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();

	CheckAnyPlayerAlive();
}

void ASGameMode::StartWave()
{
	//Рассчитать количество ботов для текущей волны.
	NrOfBotsToSpawn = 2 * (WaveCount + 1);

	//Если количество ботов меньше максимума, то увеличить счетчик волн.
	if (NrOfBotsToSpawn < MaxNrOfBotsToSpawn)
	{
		WaveCount++;
	}

	//Запустить таймер TimerHandle_BotSpawner.
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);

	//Установить состояние WaveInProgress, когда боты спавнятся в текущей волне.
	SetGameState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave()
{
	//Удалить таймер TimerHandle_BotSpawner, останавливается спавн ботов.
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	//Установить состояние WaitingToComplete, когда заспавнилось максимальное для волны количество ботов.
	SetGameState(EWaveState::WaitingToComplete);

}

void ASGameMode::PrepareForNextWave()
{
	//Запустить таймер TimerHandle_NextWaveStart. Отсчет времени до старта новой волны.
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);

	//Установить состояние WaitingToStart, когда отсчитывается время до старта новой волны.
	SetGameState(EWaveState::WaitingToStart);

	//Респавнить умерших персов.
	RestartDeadPlayers();
}

void ASGameMode::SpawnBotTimerElapsed()
{
	//Спавнить новых ботов.
	SpawnNewBot();

	//Уменьшить счетчик заспавненых ботов.
	NrOfBotsToSpawn--;

	//Если счетчик достиг 0
	if (NrOfBotsToSpawn <= 0)
	{
		//Остановить волну.
		EndWave();
	}
}

void ASGameMode::CheckWaveState()
{
	//Установить состояние подготовки к новой волне.
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	//Если не все боты в волне заспавнены или идет отсчет до старта новой волны.
	if (NrOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		//Ничего не делать.
		return;
	}

	//Установить по умолчанию, что на карте нет живых ботов.
	bool bIsAnyBotAlive = false;

	//Пройтись итератором по всем объектам класса APawn на карте.
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		//Взять павна.
		APawn* TestPawn = It->Get();

		//Если павн невалиден или управляется контроллером игрока.
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			//Перейти к следующей итерации.
			continue;
		}

		//Получить указатель на компонент HealthComp в объекте TestPawn.
		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		//Если ХП-компонент валиден и ХП больше 0.
		if (HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			//Установить, что на карте есть живые боты.
			bIsAnyBotAlive = true;

			//Выйти из цикла.
			break;
		}
	}

	//Если на карте нет живых ботов.
	if (!bIsAnyBotAlive)
	{
		//Готовиться к след. волне.
		PrepareForNextWave();

		//Установить состояние WaveComplete, когда волна окончена и на карте нет ботов.
		SetGameState(EWaveState::WaveComplete);
	}
}

void ASGameMode::CheckAnyPlayerAlive()
{
	//Пройтись итератором по всем объектам класса APlayerController на карте.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		//Взять контроллер.
		APlayerController* PC = It->Get();
		
		//Взять подконтрольного павна.
		APawn* MyPawn = PC->GetPawn();

		//Если они валидны.
		if (PC && MyPawn)
		{
			//Получить указатель на компонент HealthComp в объекте MyPawn.
			USHealthComponent* HealtComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));

			//Если компонент валиден и ХП больше 0.
			if (ensure(HealtComp) && HealtComp->GetHealth() > 0.0f)
			{
				//Ничего не делать.
				return;
			}
		}
	}
	
	//Конец игры, если все персы мертвы.
	GameOver();

}

void ASGameMode::GameOver()
{
	//Остановить спавн ботов волны.
	EndWave();

	//Установить состояние GameOver, когда игра окончена.
	SetGameState(EWaveState::GameOver);

	UE_LOG(LogTemp, Log, TEXT("Game over! All Players died!"));
}

void ASGameMode::SetGameState(EWaveState NewState)
{
	//Получить указатель на объект ASGameState.
	ASGameState* GS = GetGameState<ASGameState>();

	//Если валиден.
	if (ensureAlways(GS))
	{
		//Установить состояние игры.
		GS->SetWaveState(NewState);
	}
}

void ASGameMode::RestartDeadPlayers()
{
	//Пройтись итератором по всем объектам класса APlayerController на карте.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		//Получить контроллер.
		APlayerController* PC = It->Get();

		//Получить подконтрольного павна.
		APawn* MyPawn = PC->GetPawn();

		//Если контроллер валиден и павн невалиден.
		if (PC && MyPawn == nullptr)
		{
			//Спавнить перса.
			RestartPlayer(PC);
		}
	}
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

