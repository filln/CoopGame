 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameEngine.h" //UE_LOG()

//Типы для физ. материалов
#define SUFRACE_FLESHDEFAULT		SurfaceType1
#define SURFACE_FLESHVULNERABLE		SurfaceType2

//Тип трейс-канала коллизии
#define COLLISION_WEAPON			ECC_GameTraceChannel1

//Напечатать сообщение в окно игры и в лог
#define DEBUGMESSAGE(CustomString) UE_LOG(LogTemp, Log, TEXT(CustomString)); if(GEngine) {GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT(CustomString));}
