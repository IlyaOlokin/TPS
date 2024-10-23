// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GooParticleSystem.h"
#include "GameFramework/Actor.h"
#include "GooEnemy.generated.h"

UCLASS()
class TPS_API AGooEnemy : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Components")
	USkeletalMeshComponent* SkeletalMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ParticleSystem)
	UInstancedStaticMeshComponent* ISM;
	
	GooParticleSystem* ParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	int32 PoolSize = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	FGooParams GooParams;

	TArray<FName> Bones;

public:	
	// Sets default values for this actor's properties
	AGooEnemy();
	virtual ~AGooEnemy() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Hit(int32 InstanceIndex, UWorld* World) const;
	void ReceiveImpulse(FVector Location, float Radius, float Force) const;

};
