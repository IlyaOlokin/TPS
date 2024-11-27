// Fill out your copyright notice in the Description page of Project Settings.


#include "BonePair.h"

#include "GooParticleSystem.h"


BonePair::BonePair(FName InBone1, FName InBone2, float Radius, float AttractionMultiplier, float InActiveThresholdByLenght, USkeletalMeshComponent* SkeletalMesh,
	bool bIsRootBone, bool bNeedAdditionalForce)
: Bone1(InBone1), Bone2(InBone2), AttractionMultiplier(AttractionMultiplier), Radius(Radius),
bIsRootBone(bIsRootBone), bNeedAdditionalForce(bNeedAdditionalForce), SkeletalMesh(SkeletalMesh)
{
	ActiveThreshold = (SkeletalMesh->GetBoneLocation(InBone1) - SkeletalMesh->GetBoneLocation(InBone2)).Size() * InActiveThresholdByLenght;
}

FName BonePair::GetBoneName() const
{
	return BoneName;
}

void BonePair::AddChildBone(BonePair* LinkedBone)
{
	ChildBones.Add(LinkedBone);
}

const TArray<BonePair*>& BonePair::GetChildBones() const
{
	return ChildBones;
}

void BonePair::UpdateParticleCount(int Count, const GooParticleSystem* ParticleSystem, const UWorld* World)
{
	CurrentParticleCount = Count;
	if (!bIsActive && Count >= ActiveThreshold)
	{
		SetActive(true, ParticleSystem, World);
	}
	else if (bIsActive && Count <= ActiveThreshold * 0.7f)
	{
		SetActive(false, ParticleSystem, World);
	}
}

void BonePair::SetActive(bool bActive, const GooParticleSystem* ParticleSystem, const UWorld* World, bool bIsNeedToBeHit)
{
	bIsActive = bActive;
	
	for (auto BonePair : ChildBones)
	{
		if (!bIsActive) BonePair->SetActive(bActive, ParticleSystem, World, !bIsRecentlyHit);
		BonePair->SetHasAttraction(bActive);
	}
	
	if (!bIsActive && (bIsRecentlyHit || !bIsNeedToBeHit))
	{
		Destroy(ParticleSystem, World);
	}
}

void BonePair::SetHasAttraction(bool bAttraction)
{
	bHasAttraction = bAttraction;

	if (ChildBones.Num() == 1)
	{
		ChildBones[0]->SetHasAttraction(bAttraction);
	}
}

bool BonePair::HasAttraction() const
{
	 return (bHasAttraction || bIsRootBone) && !bIsRecentlyDestroyed;
}

bool BonePair::IsActive() const
{
	return bIsActive;
}

void BonePair::Destroy(const GooParticleSystem* ParticleSystem, const UWorld* World)
{
	bIsRecentlyDestroyed = true;
		
	ParticleSystem->ReceiveCapsuleImpulse(
		SkeletalMesh->GetBoneLocation(Bone1, EBoneSpaces::WorldSpace),
		SkeletalMesh->GetBoneLocation(Bone2, EBoneSpaces::WorldSpace),
		Radius, 300.0f);

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, [this]
	{
		bIsRecentlyDestroyed = false;
		if (HasAttraction()) SetHasAttraction(true);
	},
	10.0f,false);
}

float BonePair::GetAttractionMultiplier() const
{
	
	if (bIsActive && CurrentParticleCount >= ActiveThreshold * 1.1f)
	{
		return AttractionMultiplierForActiveState;
	}
	if (bIsActive && CurrentParticleCount >= ActiveThreshold * 0.8f)
	{
		return AttractionMultiplierForNotEnoughParticles * AttractionMultiplier;
	}
	if (!bIsActive)
	{
		return AttractionMultiplierForDeactivatedState * AttractionMultiplier;
	}

	return 1;
}

FVector BonePair::GetAdditionalForceToBoneEnd(const FVector& Point, const float Force) const
{
	if (!bNeedAdditionalForce || bIsActive && CurrentParticleCount >= ActiveThreshold * 1.1f)
	{
		return FVector::Zero();
	}
	
	FVector Dir = SkeletalMesh->GetBoneLocation(Bone2) - Point;
	Dir.Normalize();

	return Dir * Force;
}

void BonePair::GetHit(const UWorld* World)
{
	bIsRecentlyHit = true;
	World->GetTimerManager().ClearTimer(HitTimerHandle);
	World->GetTimerManager().SetTimer(HitTimerHandle, [this]
	{
		bIsRecentlyHit = false;
	},
	1.5f, false);
}


