// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GooParticleSystem.h"
#include "GooSkeletal.h"
#include "GameFramework/Actor.h"
#include "GooEnemy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitEvent, FVector, HitLocation);

UCLASS()
class TPS_API AGooEnemy : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Components")
	USkeletalMeshComponent* SkeletalMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ParticleSystem)
	UInstancedStaticMeshComponent* ISM;
	
	TUniquePtr<GooParticleSystem> ParticleSystem;
	TUniquePtr<GooSkeletal> SkeletalBones;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParticleSystem|Spawning")
	int32 MaxParticleCount = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParticleSystem|Spawning")
	int32 InitialPoolSize = 100;
	
	UPROPERTY(EditAnywhere, Category = "ParticleSystem|Spawning")
	int32 ParticlesPerGroup = 50;
	
	UPROPERTY(EditAnywhere, Category = "ParticleSystem|Spawning")
	float SpawnInterval = 10;
	
	UPROPERTY(EditAnywhere, Category = "ParticleSystem|Spawning")
	float RaycastDistance = 1000;

	UPROPERTY(EditAnywhere, Category = "ParticleSystem|Skeletal")
	float RootBoneRadius = 75;
	
	UPROPERTY(EditAnywhere, Category = "ParticleSystem|Skeletal")
	int RootBoneActiveThreshold = 100;

	UPROPERTY(EditAnywhere, Category = "ParticleSystem|Skeletal")
	float BoneRadius = 50;

	UPROPERTY(EditAnywhere, Category = "ParticleSystem|Skeletal")
	int BoneActiveThreshold = 80;

	UPROPERTY(EditAnywhere, Category = "ParticleSystem|Skeletal")
	float AttractionMultiplierForActiveState =  0.7f;

	UPROPERTY(EditAnywhere, Category = "ParticleSystem|Skeletal")
	float AttractionMultiplierForNotEnoughParticles = 1.2f;

	UPROPERTY(EditAnywhere, Category = "ParticleSystem|Skeletal")
	float AttractionMultiplierForDeactivatedState = 4.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	FGooParams GooParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "ParticleSystem|Update Treshold")
	float MinDistanceToCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "ParticleSystem|Update Treshold")
	float MaxDistanceToCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "ParticleSystem|Update Treshold")
	float MinThreshold;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "ParticleSystem|Update Treshold")
	float MaxThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "ParticleSystem|Update Treshold")
	float OffViewThreshold;

	FTimerHandle SpawnTimerHandle;
	FTimerHandle UpdateBonesTimerHandle;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHitEvent OnHitEvent;
	
	virtual void BeginPlay() override;

	void StartSpawning();
	FVector CalculateSpawnLocation();
	void UpdateBones() const;

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
