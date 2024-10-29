// Fill out your copyright notice in the Description page of Project Settings.


#include "GooEnemy.h"

#include "GooSkeletal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AGooEnemy::AGooEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM"));
	ISM->SetupAttachment(RootComponent);
}

void AGooEnemy::BeginPlay()
{
	Super::BeginPlay();

	TArray<FBonePair> BonePairs = { {"spine_03", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("spine_03") + 1)},
		{"spine_02", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("spine_02") + 1)},
		{"head", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("head") + 1)},
		{"upperarm_r", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("upperarm_r") + 1)},
		{"lowerarm_r", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("lowerarm_r") + 1)},
		{"hand_r", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("hand_r") + 1)},
		{"upperarm_l", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("upperarm_l") + 1)},
		{"lowerarm_l", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("lowerarm_l") + 1)},
		{"hand_l", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("hand_l") + 1)},
		{"thigh_r", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("thigh_r") + 1)},
		{"calf_r", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("calf_r") + 1)},
		{"foot_r", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("foot_r") + 1)},
		{"thigh_l", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("thigh_l") + 1)},
		{"calf_l", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("calf_l") + 1)},
		{"foot_l", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("foot_l") + 1)}
	};

	SkeletalBones = MakeUnique<GooSkeletal>(SkeletalMesh, BonePairs);
	
	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	TObjectPtr<APlayerCameraManager> PlayerCamera;
	if (PlayerController && PlayerController->PlayerCameraManager)
	{
		PlayerCamera = PlayerController->PlayerCameraManager;
	}
	
	ParticleSystem =  MakeUnique<GooParticleSystem>(ISM, SkeletalMesh, SkeletalBones.Get(), PlayerCamera);
	
	const std::function<FVector()> CalculatePosDelegate = std::bind(&AGooEnemy::CalculateSpawnLocation, this);
	ParticleSystem->SetInitialPool(FMath::Min(InitialPoolSize, MaxParticleCount), GooParams,  CalculatePosDelegate);
	StartSpawning();

	GooParticle::MinDistanceToCamera = GooParams.minDistanceToCamera;
	GooParticle::MaxDistanceToCamera = GooParams.maxDistanceToCamera;
	GooParticle::MinThreshold = GooParams.minThreshold;
	GooParticle::MaxThreshold = GooParams.maxThreshold;
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
	const GooParticle gooParticle = ParticleSystem->ObjectPool->Particles[InstanceIndex];
	if (!gooParticle.IsAlive) return;
	
	const FName ParentBone = SkeletalBones.Get()->FindClosestBonePair(gooParticle.Position).Bone1;
	const FTransform BoneTransform1 = SkeletalMesh->GetBoneTransform(ParentBone);
	
	
	ParticleSystem->ObjectPool->ReturnInstance(InstanceIndex, GooParams.healDelay,
		ParentBone, BoneTransform1, GetWorld());
	
	OnHitEvent.Broadcast(gooParticle.Position);
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
		ParticleSystem->ObjectPool->GetInstance(SpawnLocation, GooParams);

		
		if (ParticleSystem->ObjectPool->ActiveInstances.Num() >= MaxParticleCount)
			break;
	}
}

AGooEnemy::~AGooEnemy()
{
}

