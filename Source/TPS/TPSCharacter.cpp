// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GooEnemy.h"
#include "InputActionValue.h"
#include "Components/InstancedStaticMeshComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATPSCharacter

ATPSCharacter::ATPSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void ATPSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	TArray<USkeletalMeshComponent*> Components;
	GetComponents<USkeletalMeshComponent>(Components);
	
	for (USkeletalMeshComponent* Component : Components)
	{
		if (Component && Component->GetFName() == FName("Weapon"))
		{
			WeaponSkeletalMesh = Component;
			break;  
		}
	}
	check(WeaponSkeletalMesh);
}

void ATPSCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ATPSCharacter::Tick(float deltaTime)
{
	Super::Tick(deltaTime);
	if (bIsShooting && bCanShoot)
	{
		Shoot();
	}
}

void ATPSCharacter::Jump()
{
	StopAim();
	Super::Jump();
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPSCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPSCharacter::Look);

		// Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ATPSCharacter::StartAim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATPSCharacter::StopAim);

		// Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &ATPSCharacter::StartShoot);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &ATPSCharacter::StopShoot);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATPSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATPSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);

		Pitch += LookAxisVector.Y / 2;
		Pitch = FMath::Clamp(Pitch, -18, 18);
	}
}

void ATPSCharacter::StartAim()
{
	if (GetCharacterMovement()->IsFalling())
		return;
	bIsAiming = true;
	SetMoveType(ECharacterMoveType::Walk);
	FOnStartAim.Broadcast();
}

void ATPSCharacter::StopAim()
{
	bIsAiming = false;
	SetMoveType(ECharacterMoveType::Run);
	FOnStopAim.Broadcast();
}

void ATPSCharacter::StartShoot()
{
	bIsShooting = true;
}

void ATPSCharacter::StopShoot()
{
	bIsShooting = false;
}

void ATPSCharacter::Shoot()
{
	bCanShoot = false;
	
	const FVector AimStart = GetFollowCamera()->GetComponentLocation();
	const FVector AimEnd = AimStart + (GetFollowCamera()->GetForwardVector()  * ShootDistance) + GetSpreadValue();
	
	FHitResult AimHitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);


	const bool bHit = GetWorld()->LineTraceSingleByChannel(AimHitResult, AimStart, AimEnd, ECC_GameTraceChannel2, Params, FCollisionResponseParams());
	
	const FVector Start = WeaponSkeletalMesh->GetSocketLocation("Barrel");
	FVector Dir = (bHit ? AimHitResult.Location : AimHitResult.TraceEnd)  - Start;
	Dir.Normalize();
	const FVector End = Start + Dir * ShootDistance;
	
	FHitResult HitResult;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_GameTraceChannel2, Params, FCollisionResponseParams()))
	{
		UInstancedStaticMeshComponent* HitISMComponent = Cast<UInstancedStaticMeshComponent>(HitResult.GetComponent());
		
		if (HitISMComponent)
		{
			const AGooEnemy* Enemy = Cast<AGooEnemy>(HitISMComponent->GetOwner());
			
			const int32 InstanceIndex = HitResult.Item;
            
			if (Enemy && InstanceIndex != INDEX_NONE)
			{
				Enemy->Hit(InstanceIndex);
				Enemy->ReceiveImpulse(HitResult.Location, HitImpulseRadius, HitImpulseForce);
			}
			OnShoot.Broadcast(true, HitResult.Location);
		}
		//DrawDebugLine(GetWorld(), Start, HitResult.Location, FColor::Green, false, 1.0f);
		else
		{
			OnShoot.Broadcast(false, HitResult.Location);
		}
	}
	else
	{
		//DrawDebugLine(GetWorld(), Start, HitResult.TraceEnd, FColor::Green, false, 1.0f);
		OnShoot.Broadcast(false, HitResult.TraceEnd);
	}
	
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ATPSCharacter::ResetCanShoot, ShootCoolDown, false);
}

void ATPSCharacter::ResetCanShoot()
{
	bCanShoot = true;
}

FVector ATPSCharacter::GetSpreadValue() const
{
	float SpreadAmount = bIsAiming ? ShootSpreadAim : ShootSpread;
	if (GetMovementComponent()->IsFalling())
	{
		SpreadAmount = ShootSpreadFalling;
	}

	const FVector Spread(
	FMath::RandRange(-SpreadAmount, SpreadAmount),
	FMath::RandRange(-SpreadAmount, SpreadAmount),
	FMath::RandRange(-SpreadAmount, SpreadAmount)
	);
	
	return Spread;
}

void ATPSCharacter::SetMoveType(const ECharacterMoveType moveType) const
{
	switch (moveType)
	{
	case ECharacterMoveType::Walk:
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			break;
	case ECharacterMoveType::Run:
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			break;
		default:
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			break;
	}
}
