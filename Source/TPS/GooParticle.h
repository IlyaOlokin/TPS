// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class TPS_API GooParticle
{
public:
	FVector Position;
	FVector Velocity;
	FVector Scale;
	int32 Index;
	
	bool Active;
	float Density;
	FVector PredictedPosition;
	FVector Pressure;

	
	GooParticle(UInstancedStaticMeshComponent* InISM, int32 ISMIndex);
	~GooParticle();

	void Update(FVector* newPos);
	void UpdateInstancePos(const FVector* newPos);
	void UpdateInstanceScale(const FVector& newScale);

private:
	UInstancedStaticMeshComponent* ISM;
	
};



