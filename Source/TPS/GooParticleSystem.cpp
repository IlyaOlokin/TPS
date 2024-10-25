// Fill out your copyright notice in the Description page of Project Settings.


#include "GooParticleSystem.h"

#include "GooCalculator.h"
#include "ISMObjectPool.h"
#include "Components/InstancedStaticMeshComponent.h"


GooParticleSystem::GooParticleSystem(UInstancedStaticMeshComponent* InObjectPool, USkeletalMeshComponent* InSkeletalMesh, TArray<FName>* InBones)
{
	ObjectPool = new ISMObjectPool(InObjectPool);
	SkeletalMesh = InSkeletalMesh;
	Bones = InBones;
}

GooParticleSystem::~GooParticleSystem()
{
	delete ObjectPool;
}

void GooParticleSystem::SetInitialPool(int32 PoolSize, const FGooParams& InGooParams, const std::function<FVector()>& CalculatePosDelegate, const UWorld* World)
{
	for (int32 i = 0; i < PoolSize; ++i)
	{
		int32 newIndex = ObjectPool->GetInstance(CalculatePosDelegate(), InGooParams, World);
		//densities.Add(0);
		//velocities.Add(FVector::Zero());
		//predictedPositions.Add(FVector::Zero());
	}
	GooParams = InGooParams;

	ParticleGrid = new GooParticleGrid(GooParams.smoothingRadius);
}

void GooParticleSystem::Update(float DeltaTime)
{
	// 1000 - 47 FPS
	// 2000 - 15 FPS

	// 1000 - 68 FPS
	// 2000 - 36 FPS

	// 1000 - 68 FPS
	// 2000 - 39-50 FPS
	
	// 1000 - 75 FPS
	// 2000 - 50 FPS

	/*
	LogTemp: CalculateParentAttraction :      0.000080 | 0.000062 | 0.000 131
	LogTemp: ConstructGrid             :      0.000475 | 0.000233 | 0.000 534
	LogTemp: UpdateDensities           :      0.000216 | 0.000496 | 0.000 914
	LogTemp: CalculatePressure         :      0.000228 | 0.000711 | 0.001 773
	LogTemp: UpdateParticlePositions   :      0.003161 | 0.001362 | 0.002 985
	LogTemp: MarkRenderStateDirty      :      0.000003 | 0.000006 | 
	*/
	
	double StartTime, EndTime, ElapsedTime;
	
	StartTime = FPlatformTime::Seconds();
	CalculateParentAttraction(DeltaTime);
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	UE_LOG(LogTemp, Log, TEXT("CalculateParentAttraction: %f"), ElapsedTime);
	
	StartTime = FPlatformTime::Seconds();
	ParticleGrid->ConstructGrid(ObjectPool->Particles, ObjectPool->ActiveInstances);
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	UE_LOG(LogTemp, Log, TEXT("ConstructGrid: %f"), ElapsedTime);
	
	StartTime = FPlatformTime::Seconds();
	UpdateDensities();
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	UE_LOG(LogTemp, Log, TEXT("UpdateDensities: %f"), ElapsedTime);
	
	StartTime = FPlatformTime::Seconds();
	CalculatePressure(DeltaTime);
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	UE_LOG(LogTemp, Log, TEXT("CalculatePressure: %f"), ElapsedTime);

	/*StartTime = FPlatformTime::Seconds();
	CalculateViscosity(DeltaTime);
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	UE_LOG(LogTemp, Log, TEXT("CalculateViscosity: %f"), ElapsedTime);*/
	
	StartTime = FPlatformTime::Seconds();
	UpdateParticlePositions(DeltaTime);
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	UE_LOG(LogTemp, Log, TEXT("UpdateParticlePositions: %f"), ElapsedTime);
	
	//ObjectPool->ISM->MarkRenderStateDirty();
}

void GooParticleSystem::CalculateParentAttraction(float DeltaTime)
{
	const float MaxAttractionDistance = GooParams.maxAttractionDistance;
	
	ParallelFor(ObjectPool->ActiveInstances.Num(), [&, DeltaTime](int32 Index)
	{
		GooParticle* Particle = &ObjectPool->Particles[ObjectPool->ActiveInstances[Index]];
		FVector TotalForce = FVector::ZeroVector;
		float MinDistance = FLT_MAX;
		FVector ClosestPointOnBone;
		
		for (const auto BoneName : *Bones)
		{
			const int32 BoneIndex = SkeletalMesh->GetBoneIndex(BoneName);
			if (BoneIndex == INDEX_NONE)
			{
				continue;
			}
			
			FVector BoneStart = SkeletalMesh->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
			if (BoneIndex + 1 >= SkeletalMesh->GetNumBones())
			{
				continue;
			}
			FVector BoneEnd = SkeletalMesh->GetSocketLocation(SkeletalMesh->GetBoneName(BoneIndex + 1));

			FVector BoneDirection = BoneEnd - BoneStart;
			if (!BoneDirection.IsNearlyZero())
			{
				BoneDirection.Normalize();
				
				FVector ClosestPoint = GooCalculator::ClosestPointOnLineSegment(BoneStart, BoneEnd, Particle->Position);
				
				const float Distance = FVector::Dist(Particle->Position, ClosestPoint);
			
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
					ClosestPointOnBone = ClosestPoint;
				}

				if (Distance < MaxAttractionDistance)
				{
					const float SecondaryAttractionStrength = FMath::Lerp(GooParams.parentAttractionForce, 0.0f, Distance * Distance / (MaxAttractionDistance * MaxAttractionDistance));
					const FVector SecondaryAttractionDirection = (ClosestPoint - Particle->Position).GetSafeNormal();
					TotalForce += SecondaryAttractionDirection * (SecondaryAttractionStrength * GooParams.secondaryAttractionStrengthMultiplier);
				}
			}
		}

		float AttractionStrength = FMath::Lerp(GooParams.parentAttractionForce, 0.0f,
		                                             MinDistance * MinDistance / (MaxAttractionDistance * MaxAttractionDistance));
		AttractionStrength = FMath::Max(GooParams.parentAttractionForce * 0.2f, AttractionStrength);
		const FVector AttractionDirection = (ClosestPointOnBone - Particle->Position).GetSafeNormal();
		TotalForce += AttractionDirection * AttractionStrength;
		
		Particle->Velocity += TotalForce * DeltaTime;
		Particle->PredictedPosition = Particle->Position + Particle->Velocity * (1 / 120.0f);

	});
}

FVector ClosestPointOnLineSegment(const FVector& LineStart, const FVector& LineEnd, const FVector& Point)
{
	const FVector LineVector = LineEnd - LineStart;
	const float LineLengthSquared = LineVector.SizeSquared();
    
	// Проецируем точку на линию (но результат будет выходить за пределы отрезка)
	const float T = FVector::DotProduct(Point - LineStart, LineVector) / LineLengthSquared;

	// Ограничиваем проекцию значениями от 0 до 1, чтобы она осталась на отрезке
	const float ClampedT = FMath::Clamp(T, 0.0f, 1.0f);

	// Возвращаем ближайшую точку на отрезке
	return LineStart + ClampedT * LineVector;
}

void GooParticleSystem::CalculatePressure(float DeltaTime)
{
	ParallelFor(ObjectPool->ActiveInstances.Num(), [this, DeltaTime](int32 Index)
	{
		const int32 ParticleIndex = ObjectPool->ActiveInstances[Index];
		const FVector pressureForce = GooCalculator::CalculatePressureForce(ParticleIndex, *ParticleGrid, *ObjectPool, GooParams);
		
		if (pressureForce.ContainsNaN()) return;

		const FVector pressureAcceleration = pressureForce / ObjectPool->Particles[ParticleIndex].Density;
		ObjectPool->Particles[ParticleIndex].Velocity += pressureAcceleration * DeltaTime;
		ObjectPool->Particles[ParticleIndex].Velocity *=  FMath::Pow(1.0f - GooParams.drag, DeltaTime);
	});
}

void GooParticleSystem::CalculateViscosity(float DeltaTime)
{
	ParallelFor(ObjectPool->ActiveInstances.Num(), [this, DeltaTime](int32 Index)
	{
		const int32 ParticleIndex = ObjectPool->ActiveInstances[Index];

		const FVector viscosityForce = GooCalculator::CalculateViscosityForce(ParticleIndex, *ParticleGrid, *ObjectPool, GooParams);
		
		if (viscosityForce.ContainsNaN()) return;
		
		ObjectPool->Particles[ParticleIndex].Velocity += viscosityForce * DeltaTime;
		ObjectPool->Particles[ParticleIndex].Velocity *=  FMath::Pow(1.0f - GooParams.drag, DeltaTime);
	});
}

void GooParticleSystem::UpdateParticlePositions(float DeltaTime)
{
	for (const auto ParticleIndex : ObjectPool->ActiveInstances)
	{
		if (!ObjectPool->Particles[ParticleIndex].Active) continue;
		FVector newPos = ObjectPool->Particles[ParticleIndex].Position +  ObjectPool->Particles[ParticleIndex].Velocity * DeltaTime;
		//FVector newPos = ObjectPool->Particles[ParticleIndex].Position +  velocities[ParticleIndex] * DeltaTime;
		ObjectPool->Particles[ParticleIndex].Update(&newPos, DeltaTime);
		ObjectPool->Particles[ParticleIndex].Update(&newPos, DeltaTime);
	}
}

void GooParticleSystem::UpdateDensities()
{
	ParallelFor(ObjectPool->ActiveInstances.Num(), [this](int32 Index)
	{
		const int32 ParticleIndex = ObjectPool->ActiveInstances[Index];

		const float density = GooCalculator::CalculateDensity(ObjectPool->Particles[ParticleIndex].PredictedPosition, ParticleIndex, *ParticleGrid, *ObjectPool, GooParams);
		if (density != 0)
			ObjectPool->Particles[ParticleIndex].Density = density;
	});
}

void GooParticleSystem::ReceiveImpulse(FVector Location, float Radius, float Force) const
{
	for (const auto sector : ParticleGrid->GetNeighboringSectors(Location, Radius))
	{
		for (const auto particle : sector->Particles)
		{
			FVector dir = particle->Position - Location;
			double dist = dir.Size();
			if (dist > Radius) continue;
			const float distMultiplier = 1 - (dist * dist / (Radius * Radius));
			dir.Normalize();
			particle->Velocity += dir * (Force * distMultiplier);
		}
	}
}