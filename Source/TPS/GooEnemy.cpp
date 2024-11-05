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

	TArray<FBonePair> BonePairs = { {"Pelvis", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("Pelvis") + 1)},
		{"neck", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("neck") + 1)},
		/*{"r_thigh", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("r_thigh") + 1)},
		{"r_calf", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("r_calf") + 1)},*/
		{"r_thigh_001", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("r_thigh_001") + 1)},
		{"r_calf_001", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("r_calf_001") + 1)},
		{"r_thigh_002", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("r_thigh_002") + 1)},
		{"r_calf_002", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("r_calf_002") + 1)},
		{"r_thigh_003", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("r_thigh_003") + 1)},
		{"r_calf_003", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("r_calf_003") + 1)},
		/*{"l_thigh", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("l_thigh") + 1)},
		{"l_calf", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("l_calf") + 1)},*/
		{"l_thigh_001", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("l_thigh_001") + 1)},
		{"l_calf_001", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("l_calf_001") + 1)},
		{"l_thigh_002", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("l_thigh_002") + 1)},
		{"l_calf_002", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("l_calf_002") + 1)},
		{"l_thigh_003", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("l_thigh_003") + 1)},
		{"l_calf_003", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("l_calf_003") + 1)}
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
	GooParticle::OffViewThreshold = GooParams.offViewThreshold;
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

