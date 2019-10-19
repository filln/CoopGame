// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameState.h"
#include "Net/UnrealNetwork.h"

void ASGameState::OnRep_WaveState(EWaveState OldState)
{
	//Вызвать блюпринт-евент.
	WaveStateChanged(WaveState, OldState);
}

void ASGameState::SetWaveState(EWaveState NewState)
{
	//Если это сервер.
	if (Role == ROLE_Authority)
	{
		//Запомнить текущее состояние.
		EWaveState OldState = WaveState;

		//Изменить состояние.
		WaveState = NewState;
		
		//Передать состояние.
		OnRep_WaveState(OldState);
	}

}

//Реплицировать данные
void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASGameState, WaveState);
}