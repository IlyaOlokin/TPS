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
		Particle.Position = DefaultTransform.GetLocation() /*+ ISM->GetOwner()->GetActorLocation()*/;
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
}

void ISMObjectPool::ReturnInstance(int32 InstanceIndex, float HealDelay, const UWorld* World)
{
	Particles[InstanceIndex].UpdateInstanceScale(FVector::Zero());
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
