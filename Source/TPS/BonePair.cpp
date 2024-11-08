// Fill out your copyright notice in the Description page of Project Settings.


#include "BonePair.h"

BonePair::BonePair(FName InBone1, FName InBone2, float Radius, int ActiveThreshold, bool bIsRootBone)
	: Bone1(InBone1), Bone2(InBone2), Radius(Radius), ActiveThreshold(ActiveThreshold), bIsRootBone(bIsRootBone)
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
	if (!bIsActive && Count >= ActiveThreshold)
	{
		SetActive(true);
	}
	else if (bIsActive && Count <= ActiveThreshold / 4.0f)
	{
		SetActive(false);
	}
}

void BonePair::SetActive(bool bActive)
{
	bIsActive = bActive;
	
	for (auto BonePair : ChildBones)
	{
		if (!bIsActive) BonePair->SetActive(bActive);
		BonePair->SetHasAttraction(bActive);
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
	 return bHasAttraction || bIsRootBone;
}

bool BonePair::IsActive() const
{
	return bIsActive;
}
