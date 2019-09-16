// Fill out your copyright notice in the Description page of Project Settings.

//Компонент служит для хранения и изменения ХП, отслеживает получение дамага, меняет ХП после дамага и вызывает диспатчер OnHealthChanged

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

//объявить евент-диспатчер, который вызывается, когда меняется Health
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangeSignature, USHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, 
class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
//текущее ХП 
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "HealthComponent")
		float Health;
//дефолтное ХП
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
		float DefaultHealth;
//получение дамага
	UFUNCTION()
		void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:
//объявить евент-диспатчер, который вызывается, когда меняется Health
	UPROPERTY(BlueprintAssignable, Category = "Event")
		FOnHealthChangeSignature OnHealthChanged;
};
