// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "NavigationSystem/Public/NavigationPath.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"


//Консольная команда для отрисовки дебаг-сфер бота.
static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDriwing(
	TEXT("COOP.DebugTrackerBotDrawing"),
	DebugTrackerBotDrawing,
	TEXT("Draw debug TrackerBot"),
	ECVF_Cheat);


// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Создать меш
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));

	//Запретить мешу оказывать воздействие на НавМеш
	MeshComp->SetCanEverAffectNavigation(false);

	//Симулировать физику
	MeshComp->SetSimulatePhysics(true);

	//Задать тип коллизии
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Block);

	//Сделать меш рут-компонентом
	RootComponent = MeshComp;

	//Создать ХП-компонент
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	//Перегрузить евент OnHealthChanged методом HandleTakeDamage()
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	//Создать коллизию
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));

	//Задать радиус коллизии
	SphereComp->SetSphereRadius(200);

	//Задать тип коллизии
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	//Присоединить к руту
	SphereComp->SetupAttachment(RootComponent);


	bUseVelocityChange = false;
	bExploded = false;
	bStartedSelfDestruction = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 50;
	ExplosionRadius = 350;
	ExplosionDamage = 60;
	SelfDamageInterval = 0.5f;
	MaxPowerLevel = 4;

}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	//Выполнять на сервере. Если не сделать этого в мультиплеере, то редактор может крашится, т.к. на клиенте отсутствует NavMesh
	if (Role == ROLE_Authority)
	{
		//Найти следующую точку движения
		RefreshPath();

		//Запустить таймер поиска копий бота. Выполнять OnCheckNearbyBots() 1 раз в сек.
		FTimerHandle FTimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(FTimerHandle_CheckPowerLevel, this, &ASTrackerBot::OnCheckNearbyBots, 1.0f, true);
	}

}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//Если МатериалИнстанс еще не был создан
	if (MatInst == nullptr)
	{
		//Создать и применить на меше
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	//Если материалИснт был создан
	if (MatInst)
	{
		//Изменить параметр материала
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	//Если ХП закончилось, то запустить саморазрушение
	if (Health <= 0.0f)
	{
		SelfDestruct();
	}

	//UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
}

FVector ASTrackerBot::GetNextPathPoint()
{
	//Объявить указатель на лучшую цель бота.
	AActor* BestTarget = nullptr;

	//Объявить ближайшую (минимальную) дистанцию до BestTarget и определить как максимальную для float.
	float NearestTargerDistance = FLT_MAX;

	//Пройтись по павнам на карте.
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		//Взять павна из итератора.
		APawn* TestPawn = It->Get();

		//Если павн невалиден или в своей команде, то перейти к 
		//следующей итерации.
		if (TestPawn == nullptr || USHealthComponent::isFriendly(TestPawn, this))
		{
			continue;
		}

		//Взять ХП-компонент павна.
		USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		//Если ХП-компонент валиден и ХП больше 0.
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f)
		{
			//Получить дистанцию между собой и павном итератора.
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			//Если дистанция меньше минимальной.
			if (Distance < NearestTargerDistance)
			{
				//Определить лучшую цель.
				BestTarget = TestPawn;

				//Определить минимальную дистанцию.
				NearestTargerDistance = Distance;
			}
		}
	}

	//Если определена лучшая цель.
	if (BestTarget)
	{
		//Найти точки пути к персу.
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		//Подготовить к нахождению следующей точки назначения.
		GetWorldTimerManager().SetTimer(FTimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 5.0f, false);

		//Если путь существует и точек пути болше 1. 
		//Проверка существования пути нужна, т.к. при выполнении в BeginPlay() НавМеша может еще не существовать.
		//Проверка на больше 1 нужна, т.к. первая точка - в точке расположения самого бота.
		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			//Взять вторую точку пути.
			return NavPath->PathPoints[1];
		}
	}


	//Если пути не существует или точек 1 или меньше, то взять точку расположения самого бота.
	return GetActorLocation();

}

void ASTrackerBot::SelfDestruct()
{
	//Если саморазрушение уже вызывалось, то ничего не делать
	if (bExploded)
	{
		return;
	}

	//Поставить флаг на начало саморазрушения
	bExploded = true;

	//Спавнить частицы взрыва
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	//Проиграть звук взрыва
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());

	//Сделать меш невидимым и проходимым
	MeshComp->SetVisibility(false, true);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Выполнять на сервере
	if (Role == ROLE_Authority)
	{
		//Объявить массив игнорируемых акторов при дамаге по внешним объектам
		TArray<AActor*> IgnoredActors;

		//Добавить себя в игнор
		IgnoredActors.Add(this);

		//Рассчитать дамаг в зависимости от количества копий бота, находящихся рядом
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);

		//Нанести дамаг
		UGameplayStatics::ApplyRadialDamage(GetWorld(), ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController());

		//Нарисовать дебаг-сферу взрыва
		if (DebugTrackerBotDrawing)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, true, 5.0f, 0, 1.0f);
		}


		//Задать время жизни бота
		SetLifeSpan(2.0f);


	}
}

void ASTrackerBot::OnCheckNearbyBots()
{
	//Задать радиус определения копий бота
	const float Radius = 600;

	//Создать коллизию для определения копий бота
	FCollisionShape CollShape;

	//Задать размер коллизии
	CollShape.SetSphere(Radius);

	//Объявить структуру параметров оверлапа
	FCollisionObjectQueryParams QueryParams;

	//Добавить типы определямых колизий
	QueryParams.AddObjectTypesToQuery(MeshComp->GetCollisionObjectType());
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	//Оъявить массив структур результата оверлапа
	TArray<FOverlapResult> Overlaps;

	//Сделать оверлап
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	//Отрисовать дебаг-сферу оверлапа
	if (DebugTrackerBotDrawing)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);
	}


	//Объявить счетчик находящихся рядом копий бота
	int32 NearOfBots = 0;

	//Пройтись по массиву оверлапа
	for (FOverlapResult Result : Overlaps)
	{
		//Взять объект копии бота
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Result.GetActor());

		//Если объект копии бота валиден и не является этим объектом
		if (Bot && Bot != this)
		{
			//Увеличить счетчик на 1
			NearOfBots++;
		}
	}

	//Рассчитать мощность взрыва по количеству копий бота - как NearOfBots, но в границах от 0 до MaxPowerLevel
	PowerLevel = FMath::Clamp(NearOfBots, 0, MaxPowerLevel);

	//Если материала не было создано
	if (MatInst == nullptr)
	{
		//Создать и применить материал
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	//Если материал был создан
	if (MatInst)
	{
		//Рассчитать параметр для материала
		float Alpha = PowerLevel / (float)MaxPowerLevel;

		//Применит параметр материала
		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}

	//Отрисовать дебаг-строку значения PowerLevel
	if (DebugTrackerBotDrawing)
	{
		DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
	}

}

void ASTrackerBot::DamageSelf()
{
	//Применить дамаг к себе
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Если это сервер и бот не был взорван
	if (Role == ROLE_Authority && !bExploded)
	{

		//Рассчитать расстояние до цели
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		//Если расстояние до цели меньше, чем минимальное или равно
		if (DistanceToTarget <= RequiredDistanceToTarget)
		{

			//Отрисовать подтверждение, что точка найдена
			if (DebugTrackerBotDrawing)
			{
				DrawDebugString(GetWorld(), GetActorLocation(), "Target reached!");
			}

		}
		//Если расстояние до цели больше минимального
		else
		{
			//Найти вектор, направленный в сторону цели
			FVector ForceDirection = NextPathPoint - GetActorLocation();

			//Нормализовать вектор (сделать единичным)
			ForceDirection.Normalize();

			//Увеличить вектор в MovementForce раз
			ForceDirection *= MovementForce;

			//Применить силу к себе в сторону цели
			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			//Отрисовать дебаг-линию до цели - будет примерной траекторией движения
			if (DebugTrackerBotDrawing)
			{
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
			}

		}

		//Отрисовать дебаг-сферу в точке цели
		if (DebugTrackerBotDrawing)
		{
			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 1.0f);
		}

	}

}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	//Вызвать метод родительского класса
	Super::NotifyActorBeginOverlap(OtherActor);

	//Если оверлапа перса и саморазрушения еще не было
	if (!bStartedSelfDestruction && !bExploded)
	{
		//Скастовать найденного актора к персу
		ACharacter* PlayerPawn = Cast<ACharacter>(OtherActor);

		//Если каст прошел (указатель каста валиден) и найденный актор в другой команде
		if (PlayerPawn && !USHealthComponent::isFriendly(OtherActor, this))
		{
			//Выполнять на сервере
			if (Role = ROLE_Authority)
			{
				//Запустить таймер самоповреждения бота
				GetWorldTimerManager().SetTimer(TimerHandle_DamageSelf, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}

			//Поставить флаг, что был оверлап перса
			bStartedSelfDestruction = true;

			//Проиграть звук готовности к самовзрыву
			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);

		}

	}
}

void ASTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}

