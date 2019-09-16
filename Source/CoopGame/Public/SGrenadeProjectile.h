// Fill out your copyright notice in the Description page of Project Settings.

//Класс снарядов для оружия SProjectileWeapon

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGrenadeProjectile.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;
class UParticleSystem;

UCLASS()
class COOPGAME_API ASGrenadeProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASGrenadeProjectile();

//protected:
//	// Called when the game starts or when spawned
//	virtual void BeginPlay() override;
//
//public:	
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;

protected:

	//Меш
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* MeshComp;

	//Компонент для описания движения снарядов
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UProjectileMovementComponent* ProjectileMovement;

	//Еффекты взрыва
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* ExplodeEffect;

	//Класс компонента для описания повреждений - задается в редакторе как тип повреждений
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;

	//Радиус повреждений при взрыве
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float DamageRadius;

	//Массив акторов, которые игнорируются при взрыве
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TArray<AActor*> IgnoreActors;

	//Взрыв снаряда
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void Explode();

};
