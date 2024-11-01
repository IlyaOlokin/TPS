// Fill out your copyright notice in the Description page of Project Settings.


#include "GooCalculator.h"

#include "FGooParams.h"
#include "GooParticleGrid.h"

float GooCalculator::SmoothingKernel(const float radius, const float dist)
{
	if (dist >= radius) return 0;
	
	float scale = 15 / (2 * PI * pow(radius, 5));
	float v = radius - dist;
	return v * v * scale;
}

float GooCalculator::SmoothingKernelDerivative(const float radius, const float dist)
{
	if (dist >= radius) return 0;
	
	float scale = 15 / (pow(radius, 5) * PI);
	float v = radius - dist;
	return -v * scale;
}

float GooCalculator::ViscositySmoothingKernel(const float radius, const float dist)
{
	if (dist >= radius) return 0;

	const float scale = 315 / (64 * PI * pow(abs(radius), 9));
	float v = radius * radius - dist * dist;
	return v * v * v * scale;
}

float GooCalculator::CalculateDensity(FVector point, int32 ParticleIndex, const GooParticleGrid& ParticleGrid, const ISMObjectPool& ObjectPool, const FGooParams& GooParams)
{
	float density = 0;
	const float mass = 1;
	
	for (const auto OtherParticle : ParticleGrid.GetNeighboringParticles(&ObjectPool.Particles[ParticleIndex]))
	{
		const float dist = (OtherParticle->Position - point).Size();
		const float influence = SmoothingKernel(GooParams.smoothingRadius, dist);
		density += mass * influence;
	}
	// add self influence to the density
	const float influence = SmoothingKernel(GooParams.smoothingRadius, 0);
	density += mass * influence;

	return density;
}

FVector GooCalculator::CalculatePressureForce(int index, const GooParticleGrid& ParticleGrid, const ISMObjectPool& ObjectPool, const FGooParams& GooParams)
{
	FVector pressureForce = FVector::Zero();
	
	for (const auto OtherParticle : ParticleGrid.GetNeighboringParticles(&ObjectPool.Particles[index]))
	{

		FVector offset = ObjectPool.Particles[index].Position - OtherParticle->Position;
		const float dist = offset.Size();
		FVector dir = dist == 0 ? FMath::VRand() : offset / dist;
		
		const float slope = -SmoothingKernel(GooParams.smoothingRadius, dist);
		float density = ObjectPool.Particles[index].Density;
		float sharedPressure = CalculateSharedPressure(density, OtherParticle->Density, GooParams);
		pressureForce += sharedPressure * dir * slope * 1 / density;
	}

	return pressureForce;
}

FVector GooCalculator::CalculateViscosityForce(int index, const GooParticleGrid& ParticleGrid, const ISMObjectPool& ObjectPool, const FGooParams& GooParams)
{
	FVector viscosityForce = FVector::Zero();
	
	for (const auto OtherParticle : ParticleGrid.GetNeighboringParticles(&ObjectPool.Particles[index]))
	{

		FVector offset = ObjectPool.Particles[index].Position - OtherParticle->Position;
		const float dist = offset.Size();
		
		const float influence = ViscositySmoothingKernel(GooParams.smoothingRadius, dist);
		
		viscosityForce += (OtherParticle->Velocity - ObjectPool.Particles[index].Velocity) * influence;
	}

	return viscosityForce * GooParams.viscosityForce;
}

float GooCalculator::CalculateSharedPressure(float densityA, float densityB, const FGooParams& GooParams)
{
	const float pressureA = ConvertDensityToPressure(densityA, GooParams);
	const float pressureB = ConvertDensityToPressure(densityB, GooParams);
	return (pressureA + pressureB) / 2;
}

float GooCalculator::ConvertDensityToPressure(const float density, const FGooParams& GooParams)
{
	const float densityError = density - 1;
	const float pressure = densityError * GooParams.pressureMultiplier;
	return pressure;
}

FVector GooCalculator::ClosestPointOnLineSegment(const FVector& LineStart, const FVector& LineEnd, const FVector& Point)
{
	const FVector LineVector = LineEnd - LineStart;
	const float LineLengthSquared = LineVector.SizeSquared();
	
	const float T = FVector::DotProduct(Point - LineStart, LineVector) / LineLengthSquared;
	const float ClampedT = FMath::Clamp(T, 0.0f, 1.0f);
	
	return LineStart + ClampedT * LineVector;
}

bool GooCalculator::IsPointInView(const TObjectPtr<APlayerCameraManager>& CameraManager, const FVector& WorldPoint)
{
	if (!CameraManager) return false;

	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FRotator CameraRotation = CameraManager->GetCameraRotation();
	const FVector CameraForward = CameraRotation.Vector();

	const FVector ToPoint = (WorldPoint - CameraLocation).GetSafeNormal();
	const float AngleToTarget = FMath::RadiansToDegrees(acosf(FVector::DotProduct(CameraForward, ToPoint)));
	const float HalfFOV = CameraManager->GetFOVAngle() / 2.0f;
	
	return AngleToTarget <= HalfFOV * 1.3f;
}
