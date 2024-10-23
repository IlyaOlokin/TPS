// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FGooParams.generated.h"

USTRUCT(BlueprintType)
struct TPS_API FGooParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float size;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float smoothingRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float pressureMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float viscosityForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float parentAttractionForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float drag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float healDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float secondaryAttractionStrengthMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float maxAttractionDistance;
};
