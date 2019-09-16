// Fill out your copyright notice in the Description page of Project Settings.

//Класс для оружия, которое стреляет снарядами класса ProjectileClass

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()

protected:

	//Стрельба
	virtual void Fire() override;

	//Класс снаряда, задается в редакторе как тип снаряда
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
		TSubclassOf<AActor> ProjectileClass;
	
};
