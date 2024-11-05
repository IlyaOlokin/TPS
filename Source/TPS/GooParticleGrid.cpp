// Fill out your copyright notice in the Description page of Project Settings.


#include "GooParticleGrid.h"

GooParticleGrid::GooParticleGrid(float particleRadius) : SectorSize(particleRadius)
{
}

void GooParticleGrid::ConstructGrid(TArray<GooParticle>& particles, const TArray<int32>& indexes)
{
	ClearGrid();
	for (const int32 index : indexes)
	{
		auto cellIndex = GetCellIndex(particles[index].Position);
		Grid.FindOrAdd(cellIndex).Particles.Add(&(particles[index]));
	}
}

TArray<GooParticle*> GooParticleGrid::GetNeighboringParticles(const GooParticle* particle) const
{
	const auto cellIndex = GetCellIndex(particle->Position);
	TArray<GooParticle*> neighboringParticles;
	
	for (int offsetX = -1; offsetX <= 1; ++offsetX)
	{
		for (int offsetY = -1; offsetY <= 1; ++offsetY)
		{
			for (int offsetZ = -1; offsetZ <= 1; ++offsetZ)
			{
				FVector neighborIndex = cellIndex + FVector(offsetX, offsetY, offsetZ);
				
				if (const FGridCell* neighborCell = Grid.Find(neighborIndex))
				{
					for (GooParticle* neighborParticle : neighborCell->Particles)
					{
						if (neighborParticle != particle)
						{
							neighboringParticles.Add(neighborParticle);
						}
					}
				}
			}
		}
	}

	return neighboringParticles;
}

TArray<const FGridCell*> GooParticleGrid::GetNeighboringSectors(const FVector& Pos, const float Radius) const
{
	const auto cellIndex = GetCellIndex(Pos);
	TArray<const FGridCell*> neighboringSectors;

	const int offset = Radius / SectorSize + 1;
	
	for (int offsetX = -offset; offsetX <= offset; ++offsetX)
	{
		for (int offsetY = -offset; offsetY <= offset; ++offsetY)
		{
			for (int offsetZ = -offset; offsetZ <= offset; ++offsetZ)
			{
				FVector neighborIndex = cellIndex + FVector(offsetX, offsetY, offsetZ);
				
				if (const FGridCell* neighborCell = Grid.Find(neighborIndex))
				{
					neighboringSectors.Add(neighborCell);
				}
			}
		}
	}

	return neighboringSectors;
}

void GooParticleGrid::ClearGrid()
{
	Grid.Empty();
}

FVector GooParticleGrid::GetCellIndex(const FVector& position) const
{
	int x = FMath::FloorToInt(position.X / SectorSize);
	int y = FMath::FloorToInt(position.Y / SectorSize);
	int z = FMath::FloorToInt(position.Z / SectorSize);
	return FVector(x, y, z);
}