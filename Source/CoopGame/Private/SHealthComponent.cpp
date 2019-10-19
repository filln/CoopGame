﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "SGameMode.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100;

	bIsDead = false;

	//Объект можно передавать по сети
	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	//Если владелец компонента на сервере, то задать евент MyOwner->OnTakeAnyDamage функцией HandleTakeAnyDamage
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}


	Health = DefaultHealth;
}

float USHealthComponent::GetHealth() const
{
	return Health;
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	//Рассчитать дамаг
	float Damage = Health - OldHealth;

	//Вызвать диспатчер OnHealthChanged
	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::Heal(float HealAmount)
{
	//Если единиц лечения или ХП меньше нуля, то ничего не делать
	if (HealAmount <= 0.0f || Health <= 0.0f)
	{
		return;
	}

	//Добавить ХП, ограничив его между 0 и DefaultHealth
	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	//Вывести в лог хп и лечение 
	UE_LOG(LogTemp, Log, TEXT("Health changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

	//Вызвать диспатчер OnHealthChanged. -HealAmount с минусом, потому что изначально был задан Damage
	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);

}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//Если дамаг отрицательный или владелец мертв.
	if (Damage < 0.0f || bIsDead)
	{
		//Ничего не делать.
		return;
	}

//Изменить ХП получением дамага, Clamp ограничит итоговое ХП между 0.0f и DefaultHealth
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health));

	//Установить состояние владельца: мертв или нет.
	bIsDead = Health <= 0.0f;

//Вызвать диспатчер OnHealthChanged
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	//Если владелец компонента мертв.
	if (bIsDead)
	{
		//Получить ГеймМод.
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());

		//Если валиден.
		if (GM)
		{
			//Вызвать евент.
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}

}

//Реплицировать данные
void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USHealthComponent, Health);
}

