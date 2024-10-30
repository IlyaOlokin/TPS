// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FGooParams.h"
#include "GooParticleGrid.h"
#include "ISMObjectPool.h"

/**
 * 
 */
class TPS_API GooCalculator
{
public:
	static float SmoothingKernel(const float radius, const float dist);
	static float SmoothingKernelDerivative(const float radius, const float dist);
	static float ViscositySmoothingKernel(const float radius, const float dist);
	static float CalculateDensity(FVector point, int32 ParticleIndex, const GooParticleGrid& ParticleGrid,
		const ISMObjectPool& ObjectPool, const FGooParams& GooParams);
	static FVector CalculatePressureForce(int index, const GooParticleGrid& ParticleGrid,
		const ISMObjectPool& ObjectPool, const FGooParams& GooParams);
	static FVector CalculateViscosityForce(int index, const GooParticleGrid& ParticleGrid,
		 const ISMObjectPool& ObjectPool, const FGooParams& GooParams);
	
	//static void CalculatePressureForce(int index);
	static float CalculateSharedPressure(float densityA, float densityB, const FGooParams& GooParams);
	static float ConvertDensityToPressure(float density, const FGooParams& GooParams);
	static FVector ClosestPointOnLineSegment(const FVector& LineStart, const FVector& LineEnd, const FVector& Point);
	static bool IsPointInView(const TObjectPtr<APlayerCameraManager>& CameraManager, const FVector& WorldPoint);
};
