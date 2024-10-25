#include "GooParticle.h"
#include "Components/InstancedStaticMeshComponent.h"

GooParticle::GooParticle(UInstancedStaticMeshComponent* InISM, int32 InISMIndex)
	: Index(InISMIndex), ISM(InISM), bIsScaling(false), CurrentScaleTime(0.0f), TargetScaleTime(1.0f)
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

void GooParticle::Update(float DeltaTime)
{
	if (bIsScaling)
	{
		TickScaleUp(DeltaTime);
	}
	UpdateInstanceTransform();
}

void GooParticle::TickScaleUp(float DeltaTime)
{
	CurrentScaleTime += DeltaTime;
	float Alpha = FMath::Clamp(CurrentScaleTime / TargetScaleTime, 0.0f, 1.0f);
	Scale = FMath::Lerp(StartScale, TargetScale, Alpha);

	if (Alpha >= 1.0f)
	{
		bIsScaling = false;
		Scale = TargetScale;
	}
}

void GooParticle::UpdateInstanceTransform()
{
	if (!ISM) return;

	FTransform InstanceTransform;
	if (ISM->GetInstanceTransform(Index, InstanceTransform, true))
	{
		InstanceTransform.SetLocation(Position);
		InstanceTransform.SetScale3D(Scale);
		ISM->UpdateInstanceTransform(Index, InstanceTransform, true, false, false);
	}
}

void GooParticle::UpdateInstancePos()
{
	if (!ISM) return;

	FTransform InstanceTransform;
	if (ISM->GetInstanceTransform(Index, InstanceTransform, true))
	{
		InstanceTransform.SetLocation(Position);
		ISM->UpdateInstanceTransform(Index, InstanceTransform, true, false, false);
	}
}

void GooParticle::UpdateInstanceScale()
{
	if (!ISM) return;

	FTransform InstanceTransform;
	if (ISM->GetInstanceTransform(Index, InstanceTransform, true))
	{
		InstanceTransform.SetScale3D(Scale);
		ISM->UpdateInstanceTransform(Index, InstanceTransform, true, false, false);
	}
}
