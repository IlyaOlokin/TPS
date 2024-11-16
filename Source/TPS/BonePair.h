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

	float AttractionMultiplier = 1;
	float Radius;
	int CurrentParticleCount;
	int ActiveThreshold;
	
	BonePair(FName InBone1, FName InBone2, float Radius, float AttractionMultiplier, int ActiveThreshold,
		USkeletalMeshComponent* SkeletalMesh, bool bIsRootBone = false, bool bNeedAdditionalForce = false);

	FName GetBoneName() const;
	void AddChildBone(BonePair* LinkedBone);
	
	const TArray<BonePair*>& GetChildBones() const;

	void UpdateParticleCount(int Count, const GooParticleSystem* ParticleSystem, const UWorld* World);
	void SetActive(bool bActive, const GooParticleSystem* ParticleSystem, const UWorld* World, bool bIsNeedToBeHit = true);
	void SetHasAttraction(bool bAttraction);
	bool HasAttraction() const;
	bool IsActive() const;
	float GetAttractionMultiplier() const;
	FVector GetAdditionalForceToBoneEnd(const FVector& Point, const float Force) const;
	void GetHit(const UWorld* World);
	void Destroy(const GooParticleSystem* ParticleSystem, const UWorld* World);

private:
	bool bIsActive = false;
	bool bHasAttraction = false;
	bool bIsRootBone = false;
	bool bNeedAdditionalForce = false;
	bool bIsRecentlyDestroyed = false;
	bool bIsRecentlyHit = false;
	
	FName BoneName;
	TArray<BonePair*> ChildBones;
	
	USkeletalMeshComponent* SkeletalMesh;

	FTimerHandle HitTimerHandle;

};
