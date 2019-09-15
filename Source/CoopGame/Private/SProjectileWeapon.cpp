// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectileWeapon.h"

void ASProjectileWeapon::Fire()
{
	//Проверить на овнера и класс снаряда. Овнер задается, когда оружие спавнится
	AActor* MyOwner = GetOwner();
	if (MyOwner == nullptr || ProjectileClass == nullptr)
	{
		return;
	}

	//Точка, в которых расположены "глаза" овнера
	FVector EyeStart;

	//Направление, в котором смотрит овнер
	FRotator EyeRotation;

	//Заполнить вектор и ротатор актуальными значениями
	MyOwner->GetActorEyesViewPoint(EyeStart, EyeRotation);

	//Взять точку сокета (сокет задан в SWeapon)
	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

	//Задать параметры спавна
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//Спавнить снаряд
	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);

	//Назначить овнером для снаряда этот объект
	Projectile->SetOwner(this);
}
