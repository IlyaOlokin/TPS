// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class TPS_API BonePair
{
public:
	FName Bone1;
	FName Bone2;

	float Radius;
	int ActiveThreshold;
	
	BonePair(FName InBone1, FName InBone2, float Radius, int ActiveThreshold, bool bIsRootBone = false);

	FName GetBoneName() const;
	void AddChildBone(BonePair* LinkedBone);
	
	const TArray<BonePair*>& GetChildBones() const;

	void UpdateParticleCount(int Count);
	void SetActive(bool bActive);
	void SetHasAttraction(bool bAttraction);
	bool HasAttraction() const;
	bool IsActive() const;

private:
	bool bIsActive = false;
	bool bHasAttraction = false;
	bool bIsRootBone = false;
	
	FName BoneName;
	TArray<BonePair*> ChildBones;
};
