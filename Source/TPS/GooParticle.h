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

	void StartScaleUp(float TargetSize, float Duration);
	void Update(float DeltaTime);
	void UpdateInstanceTransform();
	void UpdateInstancePos();
	void UpdateInstanceScale();
	
private:
	UInstancedStaticMeshComponent* ISM;
	FTimerHandle ScaleUpTimerHandle;

	FVector StartScale;
	FVector TargetScale;
	bool bIsScaling;
	float CurrentScaleTime;
	float TargetScaleTime;
	
	void TickScaleUp(float DeltaTime);
};



