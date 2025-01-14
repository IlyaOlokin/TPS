// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TPSCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShoot, bool, bHit, FVector, EndLocation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartAim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopAim);

UCLASS(config=Game)
class ATPSCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	/** Shoot Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;

	
	UPROPERTY(VisibleAnywhere, Category = Shoot, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponSkeletalMesh;

	UPROPERTY(BlueprintAssignable)
	FOnShoot OnShoot;

	UPROPERTY(BlueprintAssignable)
	FOnStartAim FOnStartAim;

	UPROPERTY(BlueprintAssignable)
	FOnStopAim FOnStopAim;

public:
	ATPSCharacter();
	virtual void PostInitializeComponents() override;

	virtual void Jump() override;

protected:

	enum class ECharacterMoveType : uint8
	{
		Run,
		Walk
	};

	UPROPERTY(BlueprintReadOnly)
	float Pitch;
	
	// Shooting params
	UPROPERTY(BlueprintReadOnly)
	bool bIsAiming = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsShooting = false;

	UPROPERTY(BlueprintReadOnly)
	bool bCanShoot = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	float ShootCoolDown = 0.1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	float ShootDistance = 5000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	int ShootSpread = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	int ShootSpreadAim = 20;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	int ShootSpreadFalling = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	float HitImpulseRadius = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	float HitImpulseForce = 10;

	// Move params
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WalkSpeed = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RunSpeed = 500;
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for aiming input */
	void StartAim();
	void StopAim();

	/** Called for shooting input */
	void StartShoot();
	void StopShoot();

	void Shoot();
	void ResetCanShoot();
	FVector GetSpreadValue() const;

	// Movement
	void SetMoveType(const ECharacterMoveType moveType) const;
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void BeginPlay() override;
	virtual void Tick(float deltaTime) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE  USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE  UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};





