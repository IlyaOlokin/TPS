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
	int32 MaxParticleCount = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	int32 InitialPoolSize = 100;
	
	UPROPERTY(EditAnywhere, Category = ParticleSystem)
	int32 ParticlesPerGroup = 50;
	
	UPROPERTY(EditAnywhere, Category = ParticleSystem)
	float SpawnInterval = 10;
	
	UPROPERTY(EditAnywhere, Category = ParticleSystem)
	float RaycastDistance = 1000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	FGooParams GooParams;

	TArray<FName> Bones;
	FTimerHandle SpawnTimerHandle;
	
	virtual void BeginPlay() override;

	void StartSpawning();
	FVector CalculateSpawnLocation();

	

public:	
	// Sets default values for this actor's properties
	AGooEnemy();
	virtual ~AGooEnemy() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Hit(int32 InstanceIndex) const;
	void ReceiveImpulse(FVector Location, float Radius, float Force) const;
	
	UFUNCTION(BlueprintCallable)
	void SpawnParticleGroup();

};
