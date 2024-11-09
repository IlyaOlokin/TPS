// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class GooParticleSystem;

class TPS_API BonePair
{
public:
	inline static float AttractionMultiplierForActiveState = 0.7f;
	inline static float AttractionMultiplierForNotEnoughParticles = 1.2f;
	inline static float AttractionMultiplierForDeactivatedState = 4.0f;
	
	FName Bone1;
	FName Bone2;

	float Radius;
	int CurrentParticleCount;
	int ActiveThreshold;
	
	BonePair(FName InBone1, FName InBone2, float Radius, int ActiveThreshold,
		USkeletalMeshComponent* SkeletalMesh, bool bIsRootBone = false);

	FName GetBoneName() const;
	void AddChildBone(BonePair* LinkedBone);
	
	const TArray<BonePair*>& GetChildBones() const;

	void UpdateParticleCount(int Count, const GooParticleSystem* ParticleSystem, const UWorld* World);
	void SetActive(bool bActive, const GooParticleSystem* ParticleSystem, const UWorld* World);
	void SetHasAttraction(bool bAttraction);
	bool HasAttraction() const;
	bool IsActive() const;
	float GetAttractionMultiplier() const;

private:
	bool bIsActive = false;
	bool bHasAttraction = false;
	bool bIsRootBone = false;
	bool bIsRecentlyDestroyed = false;
	
	FName BoneName;
	TArray<BonePair*> ChildBones;
	
	USkeletalMeshComponent* SkeletalMesh;
	
	void ResetRecentlyDestroyed();

};
