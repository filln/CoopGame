﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "SPlayerState.h"

void ASPlayerState::AddScore(float ScoreDelta)
{
	//Изменить очки перса. Score берется из родительского класса APlayerState.
	Score += ScoreDelta;
}
