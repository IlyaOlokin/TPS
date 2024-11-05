// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GooSkeletal.h"

/**
 * 
 */
class TPS_API GooParticle
{
public:
	inline static float MinDistanceToCamera = 50.0f;
	inline static float MaxDistanceToCamera = 2000.0f;
	inline static float MinThreshold = 0.0f;
	inline static float MaxThreshold = 2.0f;
	inline static float OffViewThreshold = 5.0f;

	FVector Position;
	FVector Velocity;
	FVector Scale;
	int32 Index;
	
	bool IsAlive;
	float Density;
	FVector PredictedPosition;
	FVector Pressure;
	
	FName ParentBoneName;
	
	FVector ParentBoneOffset;
	FRotator ParentBoneOffsetRot;
	
	GooParticle(UInstancedStaticMeshComponent* InISM, int32 ISMIndex);
	~GooParticle();

	void StartScaleUp(float TargetSize, float Duration);
	void Update(float DeltaTime, float DistToPlayerCamera, bool bIsInView);
	void UpdateInstanceTransform(float DistToPlayerCamera, bool bIsInView);
	void UpdateInstancePos();
	void UpdateInstanceScale();
	float CalculateUpdateTransformThreshold(float DistToPlayerCamera);
	
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



