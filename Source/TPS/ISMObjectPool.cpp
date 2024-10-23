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

int32 ISMObjectPool::GetInstance(const FGooParams& GooParams)
{
	int32 InstanceIndex;
	if (!FreeInstances.IsEmpty())
	{
		FreeInstances.Dequeue(InstanceIndex);  
		ActiveInstances.Add(InstanceIndex);        
		return InstanceIndex;
	}
	else
	{
		FTransform DefaultTransform(FVector(FMath::RandRange(-500, 500), FMath::RandRange(-500, 500), FMath::RandRange(-500, 500)));
		//FTransform DefaultTransform(FVector(FLT_MAX));
		DefaultTransform.SetScale3D(FVector(GooParams.size));
		InstanceIndex = ISM->AddInstance(DefaultTransform, true);
		
		GooParticle Particle = GooParticle(ISM, InstanceIndex);
		Particle.Position = DefaultTransform.GetLocation() + ISM->GetOwner()->GetActorLocation();
		Particle.Active = true;
		Particles.Add(Particle);
		
		ActiveInstances.Add(InstanceIndex);
		
		return InstanceIndex;
	}
}

void ISMObjectPool::ReturnInstance(int32 InstanceIndex, float HealDelay, const UWorld* World)
{
	Particles[InstanceIndex].UpdateInstanceScale( FVector::Zero());
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
