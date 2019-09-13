// Fill out your copyright notice in the Description page of Project Settings.

//Класс персонажа

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

//Двигаться вперед
	void MoveForward(float Value);

//Двигаться вправо
	void MoveRight(float Value);

//Камера
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UCameraComponent* CameraComp;

//Связь между камерой и мешем
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USpringArmComponent* SpringArmComp;

//Начать красться
	void BeginCrouch();

//Закончить красться
	void EndCrouch();

//Изменить угол обзора
	void BeginZoom();

//Вернуть угол обзора
	void EndZoom();

//Включить изменение угла обзора?
	bool bWantsToZoom;

//Дефолтный угол обзора
	float DefaultFOV;

//Измененный угол обзора
	UPROPERTY(EditDefaultsOnly, Category = "Player")
		float ZoomedFOV;

//Скорость изменения угла обзора
	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 1.0f, ClampMax = 100.0f))
		float ZoomInterpSpeed;

//Начать стрельбу
	void StartFire();

//Закончить стрельбу
	void StopFire();

//Текущее оружие
	UPROPERTY(Replicated)
		ASWeapon* CurrentWeapon;

//Класс текущего оружия
	UPROPERTY(EditDefaultsOnly, Category = "Player")
		TSubclassOf<ASWeapon> WeaponClass;

//Сокет на оружии, для начальной точки стрельбы
	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
		FName WeaponSocket;

//Компонент для хранения и изменения ХП
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USHealthComponent* HealthComp;

//Функция для перегрузки евента HealthComp->OnHealthChanged
	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

//Персонаж мертв?
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
		bool bIsDied;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

//Взять координаты камеры или координты "глаз" персонажа
	virtual FVector GetPawnViewLocation() const override;

};
