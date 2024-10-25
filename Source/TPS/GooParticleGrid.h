// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GooParticle.h"

struct FGridCell
{
	TArray<GooParticle*> Particles;
	bool bIsProcessed;
};

class TPS_API GooParticleGrid
{
public:
	TMap<FVector, FGridCell> Grid;
	
	GooParticleGrid(float particleRadius);
	void ConstructGrid(TArray<GooParticle>& particles, const TArray<int32>& indexes);
	TArray<GooParticle*> GetNeighboringParticles(const GooParticle* particle) const;
	TArray<const FGridCell*> GetNeighboringSectors(const FVector& pos, const float Radius) const;
	FVector GetCellIndex(const FVector& position) const;
	
	~GooParticleGrid();

private:
	float SectorSize;
	void ClearGrid() ;
	
};
