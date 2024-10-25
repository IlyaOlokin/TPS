#include "GooParticle.h"
#include "Components/InstancedStaticMeshComponent.h"

GooParticle::GooParticle(UInstancedStaticMeshComponent* InISM, int32 InISMIndex)
	: ISM(InISM), Index(InISMIndex), bIsScaling(false), CurrentScaleTime(0.0f), TargetScaleTime(1.0f)
{
}

GooParticle::~GooParticle() {}

void GooParticle::StartScaleUp(float TargetSize, float Duration)
{
	StartScale = FVector::ZeroVector;
	TargetScale = FVector(TargetSize);
	TargetScaleTime = Duration;
	CurrentScaleTime = 0.0f;
	bIsScaling = true;
}

void GooParticle::Update(FVector* NewPos, float DeltaTime)
{
	Position = *NewPos;
	UpdateInstancePos(&Position);

	if (bIsScaling)
	{
		TickScaleUp(DeltaTime);
	}
	UpdateInstanceScale(Scale);
}

void GooParticle::TickScaleUp(float DeltaTime)
{
	CurrentScaleTime += DeltaTime;
	float Alpha = FMath::Clamp(CurrentScaleTime / TargetScaleTime, 0.0f, 1.0f);
	Scale = FMath::Lerp(StartScale, TargetScale, Alpha);

	if (Alpha >= 1.0f)
	{
		bIsScaling = false;
	}
}

void GooParticle::UpdateInstancePos(const FVector* NewPos)
{
	if (!ISM) return;

	FTransform InstanceTransform;
	if (ISM->GetInstanceTransform(Index, InstanceTransform, true))
	{
		InstanceTransform.SetLocation(*NewPos);
		ISM->UpdateInstanceTransform(Index, InstanceTransform, true, false, false);
	}
}

void GooParticle::UpdateInstanceScale(const FVector& NewScale)
{
	if (!ISM) return;

	FTransform InstanceTransform;
	if (ISM->GetInstanceTransform(Index, InstanceTransform, true))
	{
		InstanceTransform.SetScale3D(NewScale);
		if (InstanceTransform.ContainsNaN()) return;
		ISM->UpdateInstanceTransform(Index, InstanceTransform, true, false, false);
	}
}
