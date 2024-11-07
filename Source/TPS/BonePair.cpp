// Fill out your copyright notice in the Description page of Project Settings.


#include "BonePair.h"

BonePair::BonePair(FName InBone1, FName InBone2, float Radius, bool bIsRootBone)
	: Bone1(InBone1), Bone2(InBone2), Radius(Radius), bIsRootBone(bIsRootBone)
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

void BonePair::UpdateParticleCount(int Count)
{
	if (!bIsActive && Count >= 50)
	{
		SetActive(true);
	}
	else if (bIsActive && Count <= 25)
	{
		SetActive(false);
	}
}

void BonePair::SetActive(bool bActive)
{
	bIsActive = bActive;
	
	for (auto BonePair : ChildBones)
	{
		BonePair->SetHasAttraction(bActive);
	}
	
}

void BonePair::SetHasAttraction(bool bAttraction)
{
	bHasAttraction = bAttraction;
}

bool BonePair::HasAttraction() const
{
	 return bHasAttraction || bIsRootBone;
}

bool BonePair::IsActive() const
{
	return bIsActive;
}
