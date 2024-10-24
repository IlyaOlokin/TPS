// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "FGooParams.h"
#include "GooParticleGrid.h"
#include "ISMObjectPool.h"

/**
 * 
 */


class TPS_API GooParticleSystem
{
public:
	
	USkeletalMeshComponent* SkeletalMesh;
	ISMObjectPool* ObjectPool;
	GooParticleGrid* ParticleGrid;
	
	
	
	GooParticleSystem(UInstancedStaticMeshComponent* InObjectPool, USkeletalMeshComponent* InSkeletalMesh, TArray<FName>* InBones);
	~GooParticleSystem();
	
	void SetInitialPool(int32 PoolSize, const FGooParams& GooParams, const std::function<FVector()>& CalculatePosDelegate);
	void Update(float DeltaTime);

	void CalculateParentAttraction(float DeltaTime);
	void CalculatePressure(float DeltaTime);
	void CalculateViscosity(float DeltaTime);
	void UpdateParticlePositions(float DeltaTime);
	void UpdateDensities();
	
	void ReceiveImpulse(FVector Location, float Radius, float Force) const;

private:
	FGooParams GooParams;
	//TArray<float> densities;
	//TArray<FVector> velocities;
	//TArray<FVector> predictedPositions;
	
	TArray<FName>* Bones;
};

