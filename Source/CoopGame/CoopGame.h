// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameEngine.h" //UE_LOG()

#define SUFRACE_FLESHDEFAULT		SurfaceType1
#define SURFACE_FLESHVULNERABLE		SurfaceType2

#define COLLISION_WEAPON			ECC_GameTraceChannel1

#define DEBUGMESSAGE(CustomString) UE_LOG(LogTemp, Log, TEXT(CustomString)); if(GEngine) {GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT(CustomString));}
