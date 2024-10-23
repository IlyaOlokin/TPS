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
	ParticleSystem->SetInitialPool(PoolSize, GooParams);
}

void AGooEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ParticleSystem->Update(DeltaTime);
}

void AGooEnemy::Hit(int32 InstanceIndex, UWorld* World) const
{
	ParticleSystem->ObjectPool->ReturnInstance(InstanceIndex, GooParams.healDelay, World);
}

void AGooEnemy::ReceiveImpulse(FVector Location, float Radius, float Force) const
{
	ParticleSystem->ReceiveImpulse(Location, Radius, Force);
}

AGooEnemy::~AGooEnemy()
{
	delete ParticleSystem;
}

