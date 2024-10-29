// Fill out your copyright notice in the Description page of Project Settings.


#include "GooSkeletal.h"

GooSkeletal::GooSkeletal(USkeletalMeshComponent* InSkeletalMesh, const TArray<FBonePair>& InBonePairs)
	: BonePairs(InBonePairs), SkeletalMesh(InSkeletalMesh)
{
}

GooSkeletal::~GooSkeletal()
{
}

FBonePair GooSkeletal::FindClosestBonePair(const FVector& ParticlePosition)
{
	float MinDistanceAlongSegment = FLT_MAX;
	const FBonePair* ClosestBonePair = nullptr;

	for (FBonePair& BonePair : BonePairs)
	{
		int32 Bone1Index = SkeletalMesh->GetBoneIndex(BonePair.Bone1);
		int32 Bone2Index = SkeletalMesh->GetBoneIndex(BonePair.Bone2);

		if (Bone1Index == INDEX_NONE || Bone2Index == INDEX_NONE)
		{
			continue;
		}

		FVector Bone1Pos = SkeletalMesh->GetBoneLocation(BonePair.Bone1, EBoneSpaces::WorldSpace);
		FVector Bone2Pos = SkeletalMesh->GetBoneLocation(BonePair.Bone2, EBoneSpaces::WorldSpace);

		FVector SegmentDirection = (Bone2Pos - Bone1Pos).GetSafeNormal();
		FVector ProjectionPoint = Bone1Pos + FVector::DotProduct(ParticlePosition - Bone1Pos, SegmentDirection) * SegmentDirection;

		bool IsWithinSegment = FVector::DotProduct(ProjectionPoint - Bone1Pos, Bone2Pos - Bone1Pos) >= 0 &&
							   FVector::DotProduct(ProjectionPoint - Bone2Pos, Bone1Pos - Bone2Pos) >= 0;

		if (IsWithinSegment)
		{
			const float DistanceToSegment = FVector::Dist(ParticlePosition, ProjectionPoint);

			if (DistanceToSegment < MinDistanceAlongSegment)
			{
				MinDistanceAlongSegment = DistanceToSegment;

				
				ClosestBonePair = &BonePair;
			}
		}
	}
	return ClosestBonePair == nullptr ? FBonePair() : *ClosestBonePair;
}

