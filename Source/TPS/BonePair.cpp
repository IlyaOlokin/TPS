// Fill out your copyright notice in the Description page of Project Settings.


#include "BonePair.h"

#include "GooParticleSystem.h"


BonePair::BonePair(FName InBone1, FName InBone2, float Radius, int ActiveThreshold,USkeletalMeshComponent* SkeletalMesh, bool bIsRootBone)
: Bone1(InBone1), Bone2(InBone2), Radius(Radius), ActiveThreshold(ActiveThreshold), bIsRootBone(bIsRootBone), SkeletalMesh(SkeletalMesh)
{
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
	else if (bIsActive && Count <= ActiveThreshold * 0.6f)
	{
		SetActive(false, ParticleSystem, World);
	}
}

void BonePair::SetActive(bool bActive, const GooParticleSystem* ParticleSystem, const UWorld* World)
{
	bIsActive = bActive;
	
	for (auto BonePair : ChildBones)
	{
		if (!bIsActive) BonePair->SetActive(bActive, ParticleSystem, World);
		BonePair->SetHasAttraction(bActive);
	}
	
	if (!bIsActive)
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

float BonePair::GetAttractionMultiplier() const
{
	if (bIsActive && CurrentParticleCount >= ActiveThreshold * 1.1f)
	{
		return AttractionMultiplierForActiveState;
	}
	if (bIsActive && CurrentParticleCount >= ActiveThreshold * 0.7f)
	{
		return AttractionMultiplierForNotEnoughParticles;
	}
	if (!bIsActive)
	{
		return AttractionMultiplierForDeactivatedState;
	}

	return 1;
}

void BonePair::ResetRecentlyDestroyed()
{
	bIsRecentlyDestroyed = false;
}
