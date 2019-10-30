// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "SWeapon.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacter::ASCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//Разрешить евентТик
	PrimaryActorTick.bCanEverTick = true;

//создать спрингАрм
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));

//Разрешить управление поворотом перса
	SpringArmComp->bUsePawnControlRotation = true;

//Присоединить спрингАрм к рут-компоненту
	SpringArmComp->SetupAttachment(RootComponent);

//Создать камеру
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));

//Присоединить камеру к спрингАрму
	CameraComp->SetupAttachment(SpringArmComp);

//Создать компонент
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

//Назначить реакцию капсулы на трейс-канал коллизии - игнорировать
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

//Разрешить красться
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	DefaultFOV = CameraComp->FieldOfView;
	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;

	WeaponSocket = "WeaponSocket";
	

}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Задать функцию для перегрузки евента
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	//Если выполняется на сервере
	if (Role == ROLE_Authority)
	{
		//Задать параметры спавна оружия
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		//Спавнить оружие
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (CurrentWeapon)
		{
			//назначить овнера оружия классом перса
			CurrentWeapon->SetOwner(this);

			//Присоединить оружие к мешу перса
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
		}
	}
}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
}

void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{

	//Если ХП кончилось и перс не был мертв
	if (Health <= 0.0f && !bIsDied)
	{
		//Назначить перса мертвым
		bIsDied = true;

		//Прекратить движение
		GetMovementComponent()->StopMovementImmediately();

		//Выключить коллизии
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Запретить управление персом
		DetachFromControllerPendingDestroy();

		//Уничтожить объект перса через ... сек
		SetLifeSpan(3.0f);

		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
			CurrentWeapon->SetLifeSpan(3.0f);
		}
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Назначить конечный угол обзора
	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	//Плавно изменить угол обзора от CameraComp->FieldOfView до TargetFOV
	float CurrentFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(CurrentFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

//Реплицировать данные
void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bIsDied);
}