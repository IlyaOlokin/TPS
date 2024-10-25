// Fill out your copyright notice in the Description page of Project Settings.


#include "GooEnemy.h"
#include "Components/InstancedStaticMeshComponent.h"

AGooEnemy::AGooEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM"));
	ISM->SetupAttachment(RootComponent);

	Bones = { "spine_03", "spine_02", "head",
		"upperarm_r", "lowerarm_r", "hand_r",
		"upperarm_l", "lowerarm_l", "hand_l",
		"thigh_r", "calf_r", "foot_r",
		"thigh_l", "calf_l", "foot_l" };
}

void AGooEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	ParticleSystem = new GooParticleSystem(ISM, SkeletalMesh, &Bones);
	const std::function<FVector()> CalculatePosDelegate = std::bind(&AGooEnemy::CalculateSpawnLocation, this);

	ParticleSystem->SetInitialPool(InitialPoolSize, GooParams,  CalculatePosDelegate, GetWorld());
	StartSpawning();
}

void AGooEnemy::StartSpawning()
{
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AGooEnemy::SpawnParticleGroup, SpawnInterval, true);
}

FVector AGooEnemy::CalculateSpawnLocation()
{
	const FVector Start = GetActorLocation();
	const FVector RandomDirection = FMath::VRand();
	const FVector End = Start + RandomDirection * RaycastDistance;

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams))
	{
		return HitResult.Location;
	}
	else
	{
		return End;
	}
}

void AGooEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ParticleSystem->Update(DeltaTime);
}

void AGooEnemy::Hit(int32 InstanceIndex) const
{
	ParticleSystem->ObjectPool->ReturnInstance(InstanceIndex, GooParams.healDelay, GetWorld());
}

void AGooEnemy::ReceiveImpulse(FVector Location, float Radius, float Force) const
{
	ParticleSystem->ReceiveImpulse(Location, Radius, Force);
}

void AGooEnemy::SpawnParticleGroup()
{
	if (ParticleSystem->ObjectPool->ActiveInstances.Num() >= MaxParticleCount)
		return;

	for (int32 i = 0; i < ParticlesPerGroup; i++)
	{
		const FVector SpawnLocation = CalculateSpawnLocation();
		ParticleSystem->ObjectPool->GetInstance(SpawnLocation, GooParams, GetWorld());

		
		if (ParticleSystem->ObjectPool->ActiveInstances.Num() >= MaxParticleCount)
			break;
	}
}

AGooEnemy::~AGooEnemy()
{
	delete ParticleSystem;
}

