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
	DeltaTime = FMath::Min(DeltaTime, 0.016f);

	double StartTime, EndTime, ElapsedTime;

	
	StartTime = FPlatformTime::Seconds();
	CalculateParentAttraction(DeltaTime);
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	//UE_LOG(LogTemp, Log, TEXT("CalculateParentAttraction: %f"), ElapsedTime * 1000);

	StartTime = FPlatformTime::Seconds();
	ParticleGrid->ConstructGrid(ObjectPool->Particles, ObjectPool->ActiveInstances);
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	//UE_LOG(LogTemp, Log, TEXT("ConstructGrid: %f"), ElapsedTime * 1000);
	
	StartTime = FPlatformTime::Seconds();
	UpdateDensities();
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	//UE_LOG(LogTemp, Log, TEXT("UpdateDensities: %f"), ElapsedTime * 1000);

	StartTime = FPlatformTime::Seconds();
	CalculatePressure(DeltaTime);
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	//UE_LOG(LogTemp, Log, TEXT("CalculatePressure: %f"), ElapsedTime * 1000);

	StartTime = FPlatformTime::Seconds();
	UpdateParticlePositions(DeltaTime);
	EndTime = FPlatformTime::Seconds();
	ElapsedTime = EndTime - StartTime;
	//UE_LOG(LogTemp, Log, TEXT("UpdateParticlePositions: %f"), ElapsedTime * 1000);
	//UE_LOG(LogTemp, Log, TEXT("__________________________________________"));
	
	ObjectPool->ISM->MarkRenderStateDirty();
}

/*void GooParticleSystem::Update(float DeltaTime)
{
	DeltaTime = FMath::Min(DeltaTime, 0.016f);
	
	CalculateParentAttraction(DeltaTime);
	ParticleGrid->ConstructGrid(ObjectPool->Particles, ObjectPool->ActiveInstances);
	// Массив для отслеживания завершения задач
	TArray<TSharedPtr<FGraphEvent>> Tasks;

	// Создаем первую задачу: UpdateDensities
	FGraphEventRef UpdateDensitiesTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[this]()
		{
			this->UpdateDensities();
		},
		TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask
	);

	// Создаем вторую задачу: CalculatePressure, которая ждет завершения UpdateDensities
	FGraphEventRef CalculatePressureTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[this, DeltaTime]()
		{
			this->CalculatePressure(DeltaTime);
		},
		TStatId(), UpdateDensitiesTask, ENamedThreads::AnyBackgroundThreadNormalTask
	);

	// Выполняем UpdateParticlePositions на основном потоке
	UpdateParticlePositions(DeltaTime);

	// Ожидаем завершения всех задач
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(CalculatePressureTask);

	// Вызываем MarkRenderStateDirty после завершения всех задач
	
	ObjectPool->ISM->MarkRenderStateDirty();
}*/

void GooParticleSystem::CalculateParentAttraction(float DeltaTime) const
{
	const float MaxAttractionDistance = GooParams.maxAttractionDistance;
	
	ParallelFor(ObjectPool->ActiveInstances.Num(), [&, DeltaTime](int32 Index)
	{
		GooParticle* Particle = &ObjectPool->Particles[ObjectPool->ActiveInstances[Index]];
		FVector TotalForce = FVector::ZeroVector;
		float MinDistance = FLT_MAX;
		FVector ClosestPointOnBone;
		const BonePair* ClosestBonePair = nullptr;
		
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
					ClosestBonePair = BonePair;
				}

				if (Distance < MaxAttractionDistance)
				{
					const float SecondaryAttractionStrength = FMath::Lerp(GooParams.parentAttractionForce, 0.0f, Distance * Distance / (MaxAttractionDistance * MaxAttractionDistance));
					const FVector SecondaryAttractionDirection = (ClosestPoint - Particle->Position).GetSafeNormal();
					TotalForce += SecondaryAttractionDirection * (SecondaryAttractionStrength * GooParams.secondaryAttractionStrengthMultiplier * BonePair->GetAttractionMultiplier());
					TotalForce += BonePair->GetAdditionalForceToBoneEnd(Particle->Position, SecondaryAttractionStrength / 15.0f);
				}
			}
		}
		
		if (ClosestBonePair)
		{
			float AttractionStrength = FMath::Lerp(GooParams.parentAttractionForce, 0.0f,
											   MinDistance * MinDistance / (MaxAttractionDistance * MaxAttractionDistance));
			AttractionStrength = FMath::Max(GooParams.parentAttractionForce * 0.3f, AttractionStrength);
			const FVector AttractionDirection = (ClosestPointOnBone - Particle->Position).GetSafeNormal();
			TotalForce += AttractionDirection * AttractionStrength * FMath::Clamp(ClosestBonePair->GetAttractionMultiplier(), 0 , 1);
			Particle->Velocity += TotalForce * DeltaTime;
			
		}
		else
		{
			FVector NormVelocity = Particle->Velocity;
			NormVelocity.Normalize();
			Particle->Velocity += NormVelocity * 15 * DeltaTime;
		}
		Particle->PredictedPosition = Particle->Position + Particle->Velocity * (1 / 120.0f);
	});
}

void GooParticleSystem::CalculatePressure(float DeltaTime) const
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
		ObjectPool->Particles[ParticleIndex].Velocity = ObjectPool->Particles[ParticleIndex].Velocity.GetClampedToMaxSize(GooParams.maxParticleSpeed);
		ObjectPool->Particles[ParticleIndex].Position += ObjectPool->Particles[ParticleIndex].Velocity * DeltaTime;
		
		ObjectPool->Particles[ParticleIndex].Update(DeltaTime,
			(ObjectPool->Particles[ParticleIndex].Position - PlayerCamera->GetCameraLocation()).Size(),
			GooCalculator::IsPointInView(PlayerCamera, ObjectPool->Particles[ParticleIndex].Position));
	}
}

void GooParticleSystem::UpdateDensities() const
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

void GooParticleSystem::ReceivePointImpulse(const FVector& Location, float Radius, float Force) const
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
			particle->Velocity = dir * (Force * distMultiplier);
		}
	}
}

void GooParticleSystem::ReceiveCapsuleImpulse(const FVector& Location1, const FVector& Location2, float Radius,
	float Force) const
{
	ParallelFor(ObjectPool->ActiveInstances.Num(), [&](int32 Index)
	{
		const int32 ParticleIndex = ObjectPool->ActiveInstances[Index];
		if (!ObjectPool->Particles[ParticleIndex].IsAlive) return;

		const FVector ClosestPointOnLineSegment = GooCalculator::ClosestPointOnLineSegment(Location1, Location2,
		                                                                                   ObjectPool->Particles[ParticleIndex].Position);
		FVector DirToPoint = ClosestPointOnLineSegment - ObjectPool->Particles[ParticleIndex].Position;

		if (DirToPoint.Size() < Radius)
		{
			double dist = DirToPoint.Size();
			const float distMultiplier = 1 - (dist * dist / (Radius * Radius));
			DirToPoint.Normalize();
			ObjectPool->Particles[ParticleIndex].Velocity += DirToPoint * (Force * distMultiplier);
		}
	});
}
