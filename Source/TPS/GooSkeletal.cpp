// Fill out your copyright notice in the Description page of Project Settings.


#include "GooSkeletal.h"

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

void GooSkeletal::UpdateSkeletal(UWorld* World, float Radius)
{
	for (const auto BonePair : BonePairs)
	{
		PerformCapsuleTrace(World, BonePair, Radius);
	}
}

void GooSkeletal::PerformCapsuleTrace(UWorld* World, BonePair* BonePair, float Radius) const
{
	if (!World) return;
	const FVector& Start = SkeletalMesh->GetBoneLocation(BonePair->Bone1);
	const FVector& End = SkeletalMesh->GetBoneLocation(BonePair->Bone2);
	
	float HalfHeight = (End - Start).Size() / 2.0f;
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	
	TArray<FHitResult> HitResults;
	bool bHit = World->SweepMultiByChannel(
		HitResults,
		Start,
		End, 
		FQuat::Identity,
		ECC_GameTraceChannel2,
		CapsuleShape
	);
	
	if (bHit)
	{
		int count = 0;
		for (const FHitResult& Hit : HitResults)
		{
			const int32 InstanceIndex = Hit.Item;
			count++;
			
		}
		//UE_LOG(LogTemp, Log, TEXT("Объектов в капсуле: %d"), count);
		
	}

	// Опционально: отрисовка капсулы для визуализации
	DrawDebugCapsule(
		World,
		(Start + End) / 2,
		HalfHeight,
		Radius,
		FRotationMatrix::MakeFromZ(End - Start).ToQuat(),
		FColor::Green,
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
		int32 Bone1Index = SkeletalMesh->GetBoneIndex(BonePair->Bone1);
		int32 Bone2Index = SkeletalMesh->GetBoneIndex(BonePair->Bone2);

		if (Bone1Index == INDEX_NONE || Bone2Index == INDEX_NONE)
		{
			continue;
		}

		FVector Bone1Pos = SkeletalMesh->GetBoneLocation(BonePair->Bone1, EBoneSpaces::WorldSpace);
		FVector Bone2Pos = SkeletalMesh->GetBoneLocation(BonePair->Bone2, EBoneSpaces::WorldSpace);

		FVector SegmentDirection = (Bone2Pos - Bone1Pos).GetSafeNormal();
		FVector ProjectionPoint = Bone1Pos + FVector::DotProduct(Point - Bone1Pos, SegmentDirection) * SegmentDirection;

		bool IsWithinSegment = FVector::DotProduct(ProjectionPoint - Bone1Pos, Bone2Pos - Bone1Pos) >= 0 &&
							   FVector::DotProduct(ProjectionPoint - Bone2Pos, Bone1Pos - Bone2Pos) >= 0;

		if (IsWithinSegment)
		{
			const float DistanceToSegment = FVector::Dist(Point, ProjectionPoint);

			if (DistanceToSegment < MinDistanceAlongSegment)
			{
				MinDistanceAlongSegment = DistanceToSegment;

				
				ClosestBonePair = BonePair;
			}
		}
	}
	
	return ClosestBonePair;
}

