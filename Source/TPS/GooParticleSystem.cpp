// Fill out your copyright notice in the Description page of Project Settings.


#include "GooParticleSystem.h"

#include "GooCalculator.h"
#include "GooSkeletal.h"
#include "ISMObjectPool.h"
#include "Components/InstancedStaticMeshComponent.h"


GooParticleSystem::GooParticleSystem(UInstancedStaticMeshComponent* InObjectPool, USkeletalMeshComponent* InSkeletalMesh,
	GooSkeletal* InBones, const TObjectPtr<APlayerCameraManager>& InPlayerCamera)
{
	ObjectPool = new ISMObjectPool(InObjectPool);
	SkeletalMesh = InSkeletalMesh;
	Bones = InBones;
	PlayerCamera = InPlayerCamera;
}

GooParticleSystem::~GooParticleSystem()
{
	delete ObjectPool;
}

void GooParticleSystem::SetInitialPool(int32 PoolSize, const FGooParams& InGooParams, const std::function<FVector()>& CalculatePosDelegate)
{
	for (int32 i = 0; i < PoolSize; ++i)
	{
		ObjectPool->GetInstance(CalculatePosDelegate(), InGooParams);
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

	// 1000 - 65 FPS
	// 2000 - 60 FPS

	/*
	LogTemp: CalculateParentAttraction :      0.000080 | 0.000062 | 0.000 131
	LogTemp: ConstructGrid             :      0.000475 | 0.000233 | 0.000 534
	LogTemp: UpdateDensities           :      0.000216 | 0.000496 | 0.000 914
	LogTemp: CalculatePressure         :      0.000228 | 0.000711 | 0.001 773
	LogTemp: UpdateParticlePositions   :      0.003161 | 0.001362 | 0.002 985
	LogTemp: MarkRenderStateDirty      :      0.000003 | 0.000006 | 
	*/
	

	DeltaTime = FMath::Min(DeltaTime, 0.016f);
	
	CalculateParentAttraction(DeltaTime);
	ParticleGrid->ConstructGrid(ObjectPool->Particles, ObjectPool->ActiveInstances);
	UpdateDensities();
	CalculatePressure(DeltaTime);
	UpdateParticlePositions(DeltaTime);
	
	ObjectPool->ISM->MarkRenderStateDirty();
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
		
		for (BonePair* BonePair : Bones->GetAllBones())
		{
			if (!BonePair->HasAttraction()) continue;
			
			const int32 BoneIndex = SkeletalMesh->GetBoneIndex(BonePair->Bone1);
			if (BoneIndex == INDEX_NONE)continue;
			
			FVector BoneStart = SkeletalMesh->GetBoneLocation(BonePair->Bone1, EBoneSpaces::WorldSpace);
			if (BoneIndex + 1 >= SkeletalMesh->GetNumBones())continue;
			
			FVector BoneEnd = SkeletalMesh->GetSocketLocation(BonePair->Bone2);
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

void GooParticleSystem::UpdateParticlePositions(float DeltaTime)
{
	for (const auto ParticleIndex : ObjectPool->ActiveInstances)
	{
		if (!ObjectPool->Particles[ParticleIndex].IsAlive)
		{
			UpdateDestroyedParticleTransform(ObjectPool->Particles[ParticleIndex]);
			continue;
		}
		ObjectPool->Particles[ParticleIndex].Position += ObjectPool->Particles[ParticleIndex].Velocity * DeltaTime;
		ObjectPool->Particles[ParticleIndex].Update(DeltaTime,
			(ObjectPool->Particles[ParticleIndex].Position - PlayerCamera->GetCameraLocation()).Size(),
			GooCalculator::IsPointInView(PlayerCamera, ObjectPool->Particles[ParticleIndex].Position));
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

void GooParticleSystem::UpdateDestroyedParticleTransform(GooParticle& Particle)
{
	if (Particle.ParentBoneName.IsNone()) return;
	
	const FTransform BoneTransform = SkeletalMesh->GetBoneTransform(Particle.ParentBoneName);

	const FVector Right = BoneTransform.GetRotation().GetRightVector();
	FVector RotatedVector = Particle.ParentBoneOffsetRot.RotateVector(Right);
	RotatedVector.Normalize();

	const FTransform NewReferenceTransform(BoneTransform.GetRotation(), BoneTransform.GetLocation());
	const FTransform RelativeTransform(Particle.ParentBoneOffsetRot, Particle.ParentBoneOffset);
	const FTransform UpdatedTransform = RelativeTransform * NewReferenceTransform;
	
	Particle.Position = FMath::Lerp(Particle.Position, UpdatedTransform.GetLocation(), 0.8f);
	Particle.UpdateInstancePos();
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
