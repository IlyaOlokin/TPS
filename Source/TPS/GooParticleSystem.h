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
	
	ISMObjectPool* ObjectPool;
	
	GooParticleSystem(UInstancedStaticMeshComponent* InObjectPool, USkeletalMeshComponent* InSkeletalMesh,
		GooSkeletal* InBones, const TObjectPtr<APlayerCameraManager>& InPlayerCamera);
	~GooParticleSystem();
	
	void SetInitialPool(int32 PoolSize, const FGooParams& GooParams, const std::function<FVector()>& CalculatePosDelegate);
	void Update(float DeltaTime);

	void ReceivePointImpulse(const FVector& Location, float Radius, float Force) const;
	void ReceiveCapsuleImpulse(const FVector& Location1, const FVector& Location2,
		float Radius, float Force) const;

private:
	FGooParams GooParams;
	GooSkeletal* Bones;
	TObjectPtr<APlayerCameraManager> PlayerCamera;
	GooParticleGrid* ParticleGrid;
	USkeletalMeshComponent* SkeletalMesh;

	void CalculateParentAttraction(float DeltaTime) const;
	void CalculatePressure(float DeltaTime) const;
	void UpdateParticlePositions(float DeltaTime);
	void UpdateDensities() const;
	void UpdateDestroyedParticleTransform(GooParticle& Particle);
	
	
};

