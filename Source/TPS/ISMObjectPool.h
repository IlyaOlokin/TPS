// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GooParticle.h"

struct FGooParams;
/**
 * 
 */
class TPS_API ISMObjectPool
{
public:
	TArray<GooParticle> Particles;
	TQueue<int32> FreeInstances;  
	TArray<int32> ActiveInstances;
	
	UInstancedStaticMeshComponent* ISM;
	
	ISMObjectPool(UInstancedStaticMeshComponent* InISM);
	~ISMObjectPool();
	
	int32 GetInstance(const FVector Pos, const FGooParams& GooParams);
	void InitializeParticle(GooParticle& Particle, const FTransform& Transform);
	
	void ReturnInstance(int32 InstanceIndex, float HealDelay, const FName ParentBone,
	                    const FTransform& ParentTransform, const UWorld* World);
	void ReturnInstanceAfterDelay(int32 InstanceIndex);
};
