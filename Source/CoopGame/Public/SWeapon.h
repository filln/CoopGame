 // Fill out your copyright notice in the Description page of Project Settings.

//������������ ����� ���� ������� ������. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShake;

//��������� ��� �������� ������ ���������, ������� �������������
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:
//���������� �������� �����������, ������� ��������� ��������
	UPROPERTY()
		TEnumAsByte<EPhysicalSurface> SurfaceType;

//�������� ����� ���������
	UPROPERTY()
		FVector_NetQuantize TraceTo;
};

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	virtual void BeginPlay() override;

	//���
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* MeshComp;

	//����� ���������� ��� �������� ����������� - �������� � ��������� ��� ��� �����������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;

	//�������� ������ �� ������
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName MuzzleSocketName;

	//��� ��������� ������ ������� ��������� ��� �������� TracerEffect
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName TracerTargetName;

	//������� ������� �������� �� ������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* MuzzleEffect;

	//��������� ������� �������� �� �����������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* DefaultImpactEffect;

	//������� �������� �� ����������� � ������������ ����������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* FleshImpactEffect;

	//������ ��������� ��� �������� - ����� �� ���� ���������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* TracerEffect;

	//����� ���������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float LenghtTracing;

	//������� �����
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float BaseDamage;

	//�������� ������� �� ��������
	void PlayFireEffects(FVector TracerEndPoint);

	//�������� ������� �� �������� �� �����������
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	//������ ������ ��� ��������
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<UCameraShake> FireCamShake;

	//�������
	virtual void Fire();

	//������� �� �������
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire();

	//������ ��� ��������������� ��������
	FTimerHandle TimerHandle_TimeBetweenShots;

	//����� ���������� ��������
	float LastFireTime;

	//������� �������� - ���������� ���� � 1 ������
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float RateOfFire;

	//����� ����� ����������
	float TimeBetweenShots;

	//��������� ��� ���������� 
	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
		FHitScanTrace HitScanTrace;

	//����������� �� �������, ����� HitScanTrace ������������� 
	UFUNCTION()
		void OnRep_HitScanTrace();

public:	

	//������ ��������
	void StartFire();

	//��������� ��������
	void StopFire();

};
