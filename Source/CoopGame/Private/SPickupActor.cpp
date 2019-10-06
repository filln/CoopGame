// Fill out your copyright notice in the Description page of Project Settings.

#include "SPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "SPowerupActor.h"
#include "TimerManager.h"


// Sets default values
ASPickupActor::ASPickupActor()
{
	//Создать коллизию
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));

	//Задать радиус коллизии
	SphereComp->SetSphereRadius(75.0f);

	//Сделать рут-компонентом
	RootComponent = SphereComp;

	//Создать частицы
	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));

	//Ротация частиц
	DecalComp->SetRelativeRotation(FRotator(90, 0.0f, 0.0f));

	//Размер частиц
	DecalComp->DecalSize = FVector(64, 75, 75);

	//Присоединить к руту
	DecalComp->SetupAttachment(RootComponent);

	CooldownDuration = 5.0f;

	//Разрешить реплицироваться
	SetReplicates(true);

}

// Called when the game starts or when spawned
void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();

	//Делать респавн только на сервере
	if (Role == ROLE_Authority)
	{
		Respawn();
	}

}

void ASPickupActor::Respawn()
{
	//Если класс поверапа не задан, то ничего не делать
	if (PowerupClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerupClass is nullptr in %s. Please update your Blueprint."), *GetName());

		return;
	}

	//ЗАдать параметры спавна
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//Спавнить Поверап
	PowerupInstance = GetWorld()->SpawnActor<ASPowerupActor>(PowerupClass, GetActorTransform(), SpawnParams);
}

void ASPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	//Вызвать метод родительского класса
	Super::NotifyActorBeginOverlap(OtherActor);

	//Выполнять на сервере
	if (Role == ROLE_Authority)
	{
		//Если объект Поверапа валиден
		if (PowerupInstance)
		{
			//Активировать его
			PowerupInstance->ActivatePowerup(OtherActor);

			//Обнулить указатель на него, т.к. больше не нужен.
			PowerupInstance = nullptr;

			//Запустить таймер респавна Поверапа
			GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickupActor::Respawn, CooldownDuration);
		}
	}


}


