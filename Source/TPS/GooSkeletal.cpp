// Fill out your copyright notice in the Description page of Project Settings.


#include "GooSkeletal.h"

#include "GooCalculator.h"
#include "GooParticleSystem.h"

GooSkeletal::GooSkeletal(USkeletalMeshComponent* InSkeletalMesh)
	: SkeletalMesh(InSkeletalMesh), RootBone(nullptr)
{
}

GooSkeletal::~GooSkeletal()
{
	for (const BonePair* BonePair : BonePairs)
	{
		delete BonePair;
	}
	BonePairs.Empty();
}

void GooSkeletal::SetRootBone(BonePair* InRootBone)
{
	RootBone = InRootBone;
	AddBone(InRootBone);
}

void GooSkeletal::AddBone(BonePair* Bone, BonePair* ParentBone)
{
	if (ParentBone)
	{
		ParentBone->AddChildBone(Bone);
	}

	BonePairs.Add(Bone);
}

BonePair* GooSkeletal::GetRootBone() const
{
	return RootBone;
}

const TArray<BonePair*>& GooSkeletal::GetAllBones() const
{
	return BonePairs;
}

void GooSkeletal::UpdateSkeletal(UWorld* World, const GooParticleSystem* ParticleSystem)
{
	for (const auto BonePair : BonePairs)
	{
		PerformCapsuleTrace(World, BonePair, ParticleSystem);
	}
}

void GooSkeletal::PerformCapsuleTrace(UWorld* World, BonePair* BonePair, const GooParticleSystem* ParticleSystem) const
{
	if (!World || !BonePair) return;
	if (!BonePair->HasAttraction()) return;
	
	const FVector& Start = SkeletalMesh->GetBoneLocation(BonePair->Bone1);
	const FVector& End = SkeletalMesh->GetBoneLocation(BonePair->Bone2);

	FVector Dir = End - Start;
	Dir.Normalize();
	
	int count = 0;
	ParallelFor(ParticleSystem->ObjectPool->ActiveInstances.Num(), [&](int32 Index)
	{
		const int32 ParticleIndex = ParticleSystem->ObjectPool->ActiveInstances[Index];
		GooParticle& Particle = ParticleSystem->ObjectPool->Particles[ParticleIndex];
		if (Particle.IsAlive)
		{
			const float Dist = GooCalculator::GetDistanceFromPointToSegment(Particle.Position, Start + Dir * BonePair->Radius, End + (-Dir * BonePair->Radius));
			if (Dist < BonePair->Radius) count++;
		}
	});
	BonePair->UpdateParticleCount(count, ParticleSystem, World);

	// Опционально: отрисовка капсулы для визуализации

	FColor Color = BonePair->IsActive()? FColor::Green : FColor::Red;
	if (BonePair->IsActive() && BonePair->ActiveThreshold * 0.6f < count && count < BonePair->ActiveThreshold * 1.1f)
	{
		Color = FColor::Blue;
	}
	DrawDebugCapsule(
		World,
		(Start + End) / 2.0f,
		(End - Start).Size() / 2.0f,
		BonePair->Radius,
		FRotationMatrix::MakeFromZ(End - Start).ToQuat(),
		Color,
		false,
		0.2f
	);

}

BonePair* GooSkeletal::FindClosestBonePair(const FVector& Point)
{
	float MinDistanceAlongSegment = FLT_MAX;
	BonePair* ClosestBonePair = nullptr;

	for (BonePair* BonePair : BonePairs)
	{
		if (!BonePair->HasAttraction()) continue;

		int32 Bone1Index = SkeletalMesh->GetBoneIndex(BonePair->Bone1);
		int32 Bone2Index = SkeletalMesh->GetBoneIndex(BonePair->Bone2);

		if (Bone1Index == INDEX_NONE || Bone2Index == INDEX_NONE)
		{
			continue;
		}

		FVector Bone1Pos = SkeletalMesh->GetBoneLocation(BonePair->Bone1, EBoneSpaces::WorldSpace);
		FVector Bone2Pos = SkeletalMesh->GetBoneLocation(BonePair->Bone2, EBoneSpaces::WorldSpace);

		float DistanceToSegment = GooCalculator::GetDistanceFromPointToSegment(Point, Bone1Pos, Bone2Pos);
		if (DistanceToSegment > BonePair->Radius) continue;
		
		if (DistanceToSegment < MinDistanceAlongSegment)
		{
			MinDistanceAlongSegment = DistanceToSegment;
			ClosestBonePair = BonePair;
		}
	}

	return ClosestBonePair;
}

