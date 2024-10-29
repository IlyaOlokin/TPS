// Fill out your copyright notice in the Description page of Project Settings.


#include "ISMObjectPool.h"

#include "FGooParams.h"
#include "Components/InstancedStaticMeshComponent.h"


ISMObjectPool::ISMObjectPool(UInstancedStaticMeshComponent* InISM)
{
	ISM = InISM;
}

ISMObjectPool::~ISMObjectPool()
{
}

int32 ISMObjectPool::GetInstance(const FVector Pos, const FGooParams& GooParams)
{
	FTransform DefaultTransform(Pos);
	DefaultTransform.SetScale3D(FVector::Zero());

	int32 InstanceIndex;
    
	if (!FreeInstances.IsEmpty())
	{
		FreeInstances.Dequeue(InstanceIndex);
		InitializeParticle(Particles[InstanceIndex], DefaultTransform);
	}
	else
	{
		InstanceIndex = ISM->AddInstance(DefaultTransform, true);
		GooParticle NewParticle = GooParticle(ISM, InstanceIndex);
		InitializeParticle(NewParticle, DefaultTransform);
		Particles.Add(NewParticle);
	}

	Particles[InstanceIndex].StartScaleUp(GooParams.size, GooParams.appearDuration); 

    
	ActiveInstances.Add(InstanceIndex);
	return InstanceIndex;
}

void ISMObjectPool::InitializeParticle(GooParticle& Particle, const FTransform& Transform)
{
	Particle.Position = Transform.GetLocation();
	Particle.Scale = FVector::Zero();
	Particle.Velocity = FVector::Zero();
	Particle.Density = 0;
	Particle.PredictedPosition = FVector::Zero();
	Particle.IsAlive = true;
}

void ISMObjectPool::ReturnInstance(int32 InstanceIndex, float HealDelay, const FName ParentBone, const FTransform& ParentTransform, const UWorld* World)
{
	Particles[InstanceIndex].Scale = FVector::Zero();
	Particles[InstanceIndex].UpdateInstanceScale();
	Particles[InstanceIndex].IsAlive = false;
	Particles[InstanceIndex].ParentBoneName = ParentBone;
	
	
	FVector ParentRightVector = ParentTransform.GetRotation().GetRightVector();
	FVector VectorToParticle = Particles[InstanceIndex].Position - ParentTransform.GetLocation();
	
	ParentRightVector.Normalize();
	VectorToParticle.Normalize();
	
	Particles[InstanceIndex].ParentBoneOffsetRot = FQuat::FindBetweenNormals(ParentRightVector, VectorToParticle);
	Particles[InstanceIndex].ParentBoneOffsetDist = (Particles[InstanceIndex].Position - ParentTransform.GetLocation()).Size();
	
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;
	TimerDel.BindRaw(this, &ISMObjectPool::ReturnInstanceAfterDelay, InstanceIndex);
	World->GetTimerManager().SetTimer(TimerHandle, TimerDel, HealDelay, false);
}

void  ISMObjectPool::ReturnInstanceAfterDelay(int32 InstanceIndex)
{
 	if (ActiveInstances.Remove(InstanceIndex))
	{
		FreeInstances.Enqueue(InstanceIndex);
		Particles[InstanceIndex].Position = FVector(FLT_MAX);
	}
}
