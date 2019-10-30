// Fill out your copyright notice in the Description page of Project Settings.

//Компонент служит для хранения и изменения ХП, отслеживает получение дамага, меняет ХП после дамага и вызывает диспатчер OnHealthChanged.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

//Объявить евент-диспатчер, который вызывается, когда меняется Health.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangeSignature, USHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType,
class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties.
	USHealthComponent();


protected:

	//Текущее ХП. 
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
		float Health;

	//Дефолтное ХП.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
		float DefaultHealth;

public:

	//Объявить евент-диспатчер, который вызывается, когда меняется Health.
	UPROPERTY(BlueprintAssignable, Category = "Event")
		FOnHealthChangeSignature OnHealthChanged;

	//Номер команды владельца.
	UPROPERTY(EditDefaultsOnly, Category = "HealthComponent")
		uint8 TeamNum;

protected:

	//Состояние владельца компонента: мертв или нет.
	bool bIsDead;


protected:

	//получение дамага.
	UFUNCTION()
		void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//Выполнить на клиенте при репликации Health.
	UFUNCTION()
		void OnRep_Health(float OldHealt);

public:

	//Лечение перса на HealAmount единиц.
	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		void Heal(float HealAmount);

	//Проверка, какой команде принадлежит (своей или вражеской).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComponent")
		static bool isFriendly(AActor* ActorA, AActor* ActorB);

	//Вернуть Health.
	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	float GetHealth() const;

protected:
	// Called when the game starts.
	virtual void BeginPlay() override;

};
