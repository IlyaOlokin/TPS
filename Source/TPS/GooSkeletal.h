// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BonePair.h"
#include "Components/SkeletalMeshComponent.h"
class GooParticleSystem;

class TPS_API GooSkeletal
{
public:
	GooSkeletal(USkeletalMeshComponent* InSkeletalMesh);
	~GooSkeletal();
	
	void SetRootBone(BonePair* InRootBone);
	void AddBone(BonePair* Bone, BonePair* ParentBone = nullptr);
	BonePair* GetRootBone() const;
	const TArray<BonePair*>& GetAllBones() const;
	
	void UpdateSkeletal(UWorld* World, const GooParticleSystem* ParticleSystem);
	BonePair* FindClosestBonePair(const FVector& Point);
	
	void PerformCapsuleTrace(UWorld* World, BonePair* BonePair, const GooParticleSystem* ParticleSystem) const;

private:
	USkeletalMeshComponent* SkeletalMesh;
	BonePair* RootBone;
	TArray<BonePair*> BonePairs;
	
};
