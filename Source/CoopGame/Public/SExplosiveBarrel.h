// Fill out your copyright notice in the Description page of Project Settings.

//Класс для описания взрываемых бочек. При получении повреждения бочка взрывается:
	//меняется материал, спавнятся спецэффекты, бочка подпрыгивает, а к окружающим предметам применяется сила.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class UStaticMeshComponent;
class USHealthComponent;
class URadialForceComponent;
class UParticleSystem;
class UMaterialInterface;


UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:

	//Меш
	UPROPERTY(VisibleAnywhere, Category = "Components")
		UStaticMeshComponent* MeshComp;

	//Компонент для хранения и изменения ХП
	UPROPERTY(VisibleAnywhere, Category = "Components")
		USHealthComponent* HealthComp;

	//Компонент для применения силы к окружающим объектам
	UPROPERTY(VisibleAnywhere, Category = "Components")
		URadialForceComponent* RadialForceComp;

	//Эффект взрыва
	UPROPERTY(EditDefaultsOnly, Category = "FX")
		UParticleSystem* ExplosionEffects;

	//Материал, применяемый на взорванных бочках
	UPROPERTY(EditDefaultsOnly, Category = "FX")
		UMaterialInterface* ExplodedMaterial;

	//Импульс, применяемый к бочке во время взрыва
	UPROPERTY(EditDefaultsOnly, Category = "FX")
		float ExplosionImpulse;

	//Была взорвана? При репликации выполнить на клиенте OnRep_Exploded()
	UPROPERTY(ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

protected:

	//Функция для перегрузки евента HealthComp->OnHealthChanged
	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//Эффекты взрыва
	UFUNCTION()
	void OnRep_Exploded();


};
