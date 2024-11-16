// Fill out your copyright notice in the Description page of Project Settings.


#include "GooEnemy.h"

#include "GooSkeletal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AGooEnemy::AGooEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM"));
	//ISM->SetupAttachment(RootComponent);
}

void AGooEnemy::BeginPlay()
{
	Super::BeginPlay();

	SkeletalBones = MakeUnique<GooSkeletal>(SkeletalMesh);
	SkeletalBones->SetRootBone(new BonePair("neck", SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex("neck") + 1), RootBoneRadius, 0.5f, RootBoneActiveThreshold, SkeletalMesh, true));

	
	//CreateThighAndCalf("r_thigh", "r_calf", BoneRadius, 1.5f, BoneActiveThreshold, SkeletalBones->GetRootBone());
	CreateThighAndCalf("r_thigh_001", "r_calf_001", BoneRadius, 1, BoneActiveThreshold, SkeletalBones->GetRootBone());
	CreateThighAndCalf("r_thigh_002", "r_calf_002", BoneRadius, 1, BoneActiveThreshold, SkeletalBones->GetRootBone());
	CreateThighAndCalf("r_thigh_003", "r_calf_003", BoneRadius, 1, BoneActiveThreshold, SkeletalBones->GetRootBone());
	
	//CreateThighAndCalf("l_thigh", "l_calf", BoneRadius, 1.5f, BoneActiveThreshold, SkeletalBones->GetRootBone());
	CreateThighAndCalf("l_thigh_001", "l_calf_001", BoneRadius, 1, BoneActiveThreshold, SkeletalBones->GetRootBone());
	CreateThighAndCalf("l_thigh_002", "l_calf_002", BoneRadius, 1, BoneActiveThreshold, SkeletalBones->GetRootBone());
	CreateThighAndCalf("l_thigh_003", "l_calf_003", BoneRadius, 1, BoneActiveThreshold, SkeletalBones->GetRootBone());
	
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
	
	GetWorldTimerManager().SetTimer(UpdateBonesTimerHandle, this, &AGooEnemy::UpdateBones, 1.0f, true);
	
	GooParticle::MinDistanceToCamera = MinDistanceToCamera;
	GooParticle::MaxDistanceToCamera = MaxDistanceToCamera;
	GooParticle::MinThreshold = MinThreshold;
	GooParticle::MaxThreshold = MaxThreshold;
	GooParticle::OffViewThreshold = OffViewThreshold;

	BonePair::AttractionMultiplierForActiveState = AttractionMultiplierForActiveState;
	BonePair::AttractionMultiplierForNotEnoughParticles = AttractionMultiplierForNotEnoughParticles;
	BonePair::AttractionMultiplierForDeactivatedState = AttractionMultiplierForDeactivatedState;
	
	ISM->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));
}

void AGooEnemy::StartSpawning()
{
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AGooEnemy::SpawnParticleGroup, SpawnInterval, true);
}

FVector AGooEnemy::CalculateSpawnLocation()
{
	const FVector Start = SkeletalMesh->GetComponentLocation();
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

void AGooEnemy::UpdateBones() const
{
	SkeletalBones->UpdateSkeletal(GetWorld(), ParticleSystem.Get());
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

	
	BonePair* ClosestBonePair = SkeletalBones.Get()->FindClosestBonePair(gooParticle.Position);
	if (ClosestBonePair) ClosestBonePair->GetHit(GetWorld());

	const FName ParentBone = ClosestBonePair ? ClosestBonePair->Bone1 : EName::None;
	
	const FTransform BoneTransform1 = SkeletalMesh->GetBoneTransform(ParentBone);
	
	ParticleSystem->ObjectPool->ReturnInstance(InstanceIndex, GooParams.healDelay,
		ParentBone, BoneTransform1, GetWorld());

	SkeletalBones->PerformCapsuleTrace(GetWorld(), ClosestBonePair, ParticleSystem.Get());
	OnHitEvent.Broadcast(gooParticle.Position);
}

void AGooEnemy::ReceiveImpulse(FVector Location, float Radius, float Force) const
{
	ParticleSystem->ReceivePointImpulse(Location, Radius, Force);
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

void AGooEnemy::CreateThighAndCalf(const FName& ThighName, const FName& CalfName, float Radius, float AttractionMultiplier, float Threshold, BonePair* ParentBone) const
{
	BonePair* Thigh = new BonePair(ThighName, SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex(ThighName) + 1), Radius, AttractionMultiplier, Threshold, SkeletalMesh);
	SkeletalBones->AddBone(Thigh, ParentBone);

	BonePair* Calf = new BonePair(CalfName, SkeletalMesh->GetBoneName(SkeletalMesh->GetBoneIndex(CalfName) + 1), Radius, AttractionMultiplier, Threshold, SkeletalMesh, false, true);
	SkeletalBones->AddBone(Calf, Thigh);
}