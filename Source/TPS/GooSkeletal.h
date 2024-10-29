// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"

struct FBonePair
{
	FName Bone1;
	FName Bone2;
    
	FBonePair(FName InBone1, FName InBone2)
		: Bone1(InBone1), Bone2(InBone2) {}
	FBonePair(){}
};

class TPS_API GooSkeletal
{
public:
	GooSkeletal(USkeletalMeshComponent* InSkeletalMesh, const TArray<FBonePair>& InBonePairs);
	~GooSkeletal();

	FBonePair FindClosestBonePair(const FVector& ParticlePosition);

	TArray<FBonePair> BonePairs;

private:
	USkeletalMeshComponent* SkeletalMesh;
	
};
