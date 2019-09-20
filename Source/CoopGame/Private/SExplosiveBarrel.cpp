// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"
#include "CoopGame.h"


// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	//Создать меш
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));

	//Включить физику
	MeshComp->SetSimulatePhysics(true);

	//Назначить тип коллизии 
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);

	//Сделать рут-компонентом
	RootComponent = MeshComp;

	//Создать Компонент для хранения и изменения ХП
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	//Задать функцию для перегрузки евента
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

	//Создать объект компонент для применения силы к окружающим объектам
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));

	//Приаттачить к мешу (руту)
	RadialForceComp->SetupAttachment(MeshComp);

	//не активировать автоматически, только по событию
	RadialForceComp->bAutoActivate = false;

	//импульс игнорирует массу объектов и всегда приводит к изменению скорости
	RadialForceComp->bImpulseVelChange = true;

	//Игнорировать себя (актора, владеющего компонентом URadialForceComponent)
	RadialForceComp->bIgnoreOwningActor = true;

	//Радиус применения силы
	RadialForceComp->Radius = 250;

	ExplosionImpulse = 400;

	//Разрешить репликацию
	SetReplicates(true);

	//Разрешить репликацию движения
	SetReplicateMovement(true);

}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//Если бочка уже взорвана, то не реагировать
	if (bExploded)
	{
		return;
	}

//	UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health));

	//Если ХП меньше 0
	if (Health <= 0.0f)
	{
		bExploded = true;

		OnRep_Exploded();

		//Применить импульс к себе
		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		//Применить импульс к окружающим объектам
		RadialForceComp->FireImpulse();


	}
}

void ASExplosiveBarrel::OnRep_Exploded()
{
	//Спавн частиц взрыва
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffects, GetActorLocation());

	//Изменить материал
	MeshComp->SetMaterial(0, ExplodedMaterial);
}

//Реплицировать данные
void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}

