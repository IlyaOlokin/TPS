// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "FGooParams.h"
#include "GooParticleGrid.h"
#include "GooSkeletal.h"
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
	TObjectPtr<APlayerCameraManager> PlayerCamera;
	
	
	GooParticleSystem(UInstancedStaticMeshComponent* InObjectPool, USkeletalMeshComponent* InSkeletalMesh,
		GooSkeletal* InBones, const TObjectPtr<APlayerCameraManager>& InPlayerCamera);
	~GooParticleSystem();
	
	void SetInitialPool(int32 PoolSize, const FGooParams& GooParams, const std::function<FVector()>& CalculatePosDelegate);
	void Update(float DeltaTime);

	void CalculateParentAttraction(float DeltaTime);
	void CalculatePressure(float DeltaTime);
	void CalculateViscosity(float DeltaTime);
	void UpdateParticlePositions(float DeltaTime);
	void UpdateDensities();
	void UpdateDestroyedParticleTransform(GooParticle& Particle);
	
	void ReceiveImpulse(FVector Location, float Radius, float Force) const;

private:
	FGooParams GooParams;
	//TArray<float> densities;
	//TArray<FVector> velocities;
	//TArray<FVector> predictedPositions;
	
	GooSkeletal* Bones;
};

