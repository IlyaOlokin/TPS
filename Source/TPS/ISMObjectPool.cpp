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

/*int32 ISMObjectPool::GetInstance(const FVector Pos, const FGooParams& GooParams, const UWorld* World)
{
	int32 InstanceIndex;
	FTransform DefaultTransform(Pos);
	DefaultTransform.SetScale3D(FVector(GooParams.size));
	
	if (!FreeInstances.IsEmpty())
	{
		FreeInstances.Dequeue(InstanceIndex);  
		ActiveInstances.Add(InstanceIndex);
		Particles[InstanceIndex].Position = DefaultTransform.GetLocation();
		Particles[InstanceIndex].Scale = FVector(GooParams.size);
		//Particles[InstanceIndex].UpdateInstanceScale(FVector(GooParams.size));
		Particles[InstanceIndex].Velocity = FVector::Zero();
		Particles[InstanceIndex].Density = 0;
		Particles[InstanceIndex].PredictedPosition = FVector::Zero();
		Particles[InstanceIndex].Active = true;
		return InstanceIndex;
	}
	else
	{
		InstanceIndex = ISM->AddInstance(DefaultTransform, true);
		
		GooParticle Particle = GooParticle(ISM, InstanceIndex);
		Particle.Position = DefaultTransform.GetLocation() /*+ ISM->GetOwner()->GetActorLocation()#1#;
		Particle.Scale = FVector(GooParams.size);
		//Particle.UpdateInstanceScale(FVector(GooParams.size));
		Particle.Velocity = FVector::Zero();
		Particle.Density = 0;
		Particle.PredictedPosition = FVector::Zero();
		Particle.Active = true;
		Particles.Add(Particle);
		
		ActiveInstances.Add(InstanceIndex);
		
		return InstanceIndex;
	}
}*/

int32 ISMObjectPool::GetInstance(const FVector Pos, const FGooParams& GooParams, const UWorld* World)
{
	FTransform DefaultTransform(Pos);
	DefaultTransform.SetScale3D(FVector::Zero());

	int32 InstanceIndex;
    
	if (!FreeInstances.IsEmpty())
	{
		FreeInstances.Dequeue(InstanceIndex);
		InitializeParticle(Particles[InstanceIndex], DefaultTransform, GooParams.size);
	}
	else
	{
		InstanceIndex = ISM->AddInstance(DefaultTransform, true);
		GooParticle NewParticle = GooParticle(ISM, InstanceIndex);
		InitializeParticle(NewParticle, DefaultTransform, GooParams.size);
		Particles.Add(NewParticle);
	}

	Particles[InstanceIndex].StartScaleUp(GooParams.size, GooParams.appearDuration); 

    
	ActiveInstances.Add(InstanceIndex);
	return InstanceIndex;
}

void ISMObjectPool::InitializeParticle(GooParticle& Particle, const FTransform& Transform, float Size)
{
	Particle.Position = Transform.GetLocation();
	Particle.Scale = FVector::Zero();
	Particle.Velocity = FVector::Zero();
	Particle.Density = 0;
	Particle.PredictedPosition = FVector::Zero();
	Particle.Active = true;
}

void ISMObjectPool::ReturnInstance(int32 InstanceIndex, float HealDelay, const UWorld* World)
{
	Particles[InstanceIndex].Scale = FVector::Zero();
	Particles[InstanceIndex].UpdateInstanceScale();
	Particles[InstanceIndex].Active = false;
	
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
