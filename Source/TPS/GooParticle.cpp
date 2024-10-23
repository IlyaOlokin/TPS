// Fill out your copyright notice in the Description page of Project Settings.


#include "GooParticle.h"

#include "Components/InstancedStaticMeshComponent.h"

GooParticle::GooParticle(UInstancedStaticMeshComponent* InISM, int32 InISMIndex)
{
	ISM = InISM;
	Index = InISMIndex;
}

GooParticle::~GooParticle()
{
}

void GooParticle::Update(FVector* newPos)
{
	Position = *newPos;
	UpdateInstancePos(&Position);
}

void GooParticle::UpdateInstancePos(FVector* newPos)
{
	FTransform InstanceTransform;
	if (ISM->GetInstanceTransform(Index, InstanceTransform, true))
	{
		
		//if (FVector::Dist(*newPos, InstanceTransform.GetLocation()) < 1.0f) return;
		InstanceTransform.SetLocation(*newPos);
		
		ISM->UpdateInstanceTransform(Index, InstanceTransform,
			true, false, false);
	}
}

void GooParticle::UpdateInstanceScale(const FVector& newScale)
{
	FTransform InstanceTransform;
	if (ISM->GetInstanceTransform(Index, InstanceTransform, true))
	{
		InstanceTransform.SetScale3D(newScale);
		if (InstanceTransform.ContainsNaN()) return;	
		ISM->UpdateInstanceTransform(Index, InstanceTransform,
			true, false, false);
	}
}
