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

	UPROPERTY(EditAnywhere, Category = Skeletal)
	float RootBoneRadius = 75;
	
	UPROPERTY(EditAnywhere, Category = Skeletal)
	int RootBoneActiveThreshold = 100;

	UPROPERTY(EditAnywhere, Category = Skeletal)
	float BoneRadius = 50;

	UPROPERTY(EditAnywhere, Category = Skeletal)
	int BoneActiveThreshold = 80;

	UPROPERTY(EditAnywhere, Category = Skeletal)
	float AttractionMultiplierForActiveState =  0.7f;

	UPROPERTY(EditAnywhere, Category = Skeletal)
	float AttractionMultiplierForNotEnoughParticles = 1.2f;

	UPROPERTY(EditAnywhere, Category = Skeletal)
	float AttractionMultiplierForDeactivatedState = 4.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	FGooParams GooParams;

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
