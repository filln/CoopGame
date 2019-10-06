// Fill out your copyright notice in the Description page of Project Settings.

//Класс площадки для SPowerupActor (Поверапа). Управляет спавном (частота спавна) Поверапа, его классом, дает команду на срабатывание Поверапа, 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ASPowerupActor;

UCLASS()
class COOPGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Коллизия для оверлапа перса и вызова NotifyActorBeginOverlap
	UPROPERTY(VisibleAnywhere, Category = "Components")
		USphereComponent* SphereComp;

	//Частицы для визуального представления площадки
	UPROPERTY(VisibleAnywhere, Category = "Components")
		UDecalComponent* DecalComp;
	
	//Класс определенного поверапа, который будет спавниться
	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
		TSubclassOf<ASPowerupActor> PowerupClass;

	//Время до следующего спавна Поверапа
	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
		float CooldownDuration;

	//Объект текущего Поверапа
	ASPowerupActor* PowerupInstance;

	//Таймер респавна Поверапа
	FTimerHandle TimerHandle_RespawnTimer;

	//Респавн Поверапа
	void Respawn();


public:

	//Активирует Поверап и запускает таймер его респавна
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
