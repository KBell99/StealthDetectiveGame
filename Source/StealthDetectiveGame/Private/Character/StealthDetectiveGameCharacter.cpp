// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/StealthDetectiveGameCharacter.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "StealthDetectiveGame.h"
#include "Objective/StealthEvidence.h"
#include "Player/StealthDetectiveGamePlayerController.h"
#include "DrawDebugHelpers.h"
#include "AI/StealthEnemy.h"
#include "Objective/StealthTrailStart.h"


// Construction and Input Setup
AStealthDetectiveGameCharacter::AStealthDetectiveGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//Create a photo camera
	PhotoCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PhotoCamera"));
	PhotoCamera->SetupAttachment(GetMesh(), FName("CameraSocket"));
	PhotoCamera->bUsePawnControlRotation = true;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AStealthDetectiveGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// // Jumping
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::DoCrouch);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::Look);

		//Camera
		EnhancedInputComponent->BindAction(EnableCameraAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::EnableCamera);
		EnhancedInputComponent->BindAction(ZoomInAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::ZoomInCamera);
		EnhancedInputComponent->BindAction(ZoomOutAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::ZoomOutCamera);
		EnhancedInputComponent->BindAction(TakePictureAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::TakePicture);
		EnhancedInputComponent->BindAction(EnableCameraFlashAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::EnableCameraFlash);
		//Detective Mode
		EnhancedInputComponent->BindAction(EnableDetectiveModeAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::EnableDetectiveMode);
		
		EnhancedInputComponent->BindAction(EvidenceScanAction, ETriggerEvent::Started, this,
		                                   &AStealthDetectiveGameCharacter::StartScanning);
		EnhancedInputComponent->BindAction(EvidenceScanAction, ETriggerEvent::Triggered, this,
		                                   &AStealthDetectiveGameCharacter::EvidenceScanned);
	}
	else
	{
		UE_LOG(LogStealthDetectiveGame, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

// Internal Input Handling
void AStealthDetectiveGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AStealthDetectiveGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AStealthDetectiveGameCharacter::DoCrouch(const FInputActionValue& Value)
{
	if (bIsCrouched)
	{
		UnCrouch();
		bIsCrouched = false;
	}
	else
	{
		Crouch();
		bIsCrouched = true;
	}
}

void AStealthDetectiveGameCharacter::EnableCamera()
{
	AStealthDetectiveGamePlayerController* MyController = Cast<AStealthDetectiveGamePlayerController>(GetController());
	if (MyController)
	{
		if (!bIsCameraEnabled)
		{
			MyController->EnableMappingContext(FName("Camera"));
			ToggleCamera();
			bUseControllerRotationYaw = true;
			bIsCameraEnabled = true;
		}
		else
		{
			MyController->DisableMappingContext(FName("Camera"));
			ToggleCamera();
			bUseControllerRotationYaw = false;
			bIsCameraEnabled = false;
		}
	}
}

void AStealthDetectiveGameCharacter::ZoomInCamera(const FInputActionValue& Value)
{
	PhotoCamera->SetFieldOfView(FMath::Clamp(PhotoCamera->FieldOfView - 1.0f, 15.0f, 90.0f));
}

void AStealthDetectiveGameCharacter::ZoomOutCamera(const FInputActionValue& Value)
{
	PhotoCamera->SetFieldOfView(FMath::Clamp(PhotoCamera->FieldOfView + 1.0f, 15.0f, 90.0f));
}

void AStealthDetectiveGameCharacter::TakePicture()
{
	AStealthEvidence* Evidence = EvidenceInView();
	if (Evidence && Evidence->GameplayTags.HasTag(FGameplayTag::RequestGameplayTag("Evidence.Objective")))
	{
		for (const FGameplayTag& Tag : Evidence->GameplayTags.GetGameplayTagArray())
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag("Evidence.Objective")))
			{
				UE_LOG(LogStealthDetectiveGame, Log, TEXT("Objective Evidence Found: %s"), *Tag.ToString());
				OnEvidenceFound.Broadcast(Tag);
				return;
			}
		}
	}

	if (bIsCameraFlashEnabled)
	{
		FlashPhotography();
	}
}

void AStealthDetectiveGameCharacter::EnableCameraFlash()
{
	bIsCameraFlashEnabled = !bIsCameraFlashEnabled;
}

AStealthEvidence* AStealthDetectiveGameCharacter::EvidenceInView() const
{
	if (!PhotoCamera) return nullptr;

	FVector CameraLocation = PhotoCamera->GetComponentLocation();
	FVector ForwardVector = PhotoCamera->GetForwardVector();
	FVector TraceStart = CameraLocation;
	FVector TraceEnd = CameraLocation + ForwardVector * PhotoTraceEnd;
	FQuat BoxRotation = PhotoCamera->GetComponentQuat();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FHitResult> HitResults;
	GetWorld()->SweepMultiByChannel(
		HitResults,
		TraceStart,
		TraceEnd,
		BoxRotation,
		ECC_GameTraceChannel2,
		FCollisionShape::MakeBox(BoxExtent),
		QueryParams
	);

	DrawDebugBox(GetWorld(), TraceEnd, BoxExtent, BoxRotation, FColor::Green, false, 2.0f);
	
	
	for (const FHitResult& Hit : HitResults)
	{
		if (AStealthEvidence* Evidence = Cast<AStealthEvidence>(Hit.GetActor()))
		{
			// Visibility check: Line trace from camera to evidence actor
			FHitResult VisibilityHit;
			FVector EvidenceLocation = Evidence->GetActorLocation();
			GetWorld()->LineTraceSingleByChannel(
				VisibilityHit,
				CameraLocation,
				EvidenceLocation,
				ECC_Visibility,
				QueryParams
			);

			DrawDebugLine(GetWorld(), CameraLocation, EvidenceLocation, FColor::Blue, false, 2.0f);
			UE_LOG(LogStealthDetectiveGame, Log, TEXT("Visibility Hit Actor: %s"), *GetNameSafe(VisibilityHit.GetActor()));
			
			if (VisibilityHit.GetActor()->GetClass() == Evidence->GetClass())
			{
				return Evidence; // Stop after finding the first visible evidence
			}
		}
	}

	return nullptr;
}

void AStealthDetectiveGameCharacter::FlashPhotography()
{
	if (!PhotoCamera) return;

	FVector CameraLocation = PhotoCamera->GetComponentLocation();
	FVector ForwardVector = PhotoCamera->GetForwardVector();
	FVector TraceStart = CameraLocation;
	FVector TraceEnd = CameraLocation + ForwardVector * FlashTraceEnd;
	FQuat BoxRotation = PhotoCamera->GetComponentQuat();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FHitResult> HitResults;
	GetWorld()->SweepMultiByChannel(
		HitResults,
		TraceStart,
		TraceEnd,
		BoxRotation,
		ECC_Visibility,
		FCollisionShape::MakeBox(FlashBoxExtent),
		QueryParams
	);

	DrawDebugBox(GetWorld(), TraceEnd, FlashBoxExtent,BoxRotation, FColor::Yellow, false, 2.0f);

	for (const FHitResult& Hit : HitResults)
	{
		if (AStealthEnemy* Enemy = Cast<AStealthEnemy>(Hit.GetActor()))
		{
			Enemy->Stun();
			UE_LOG(LogStealthDetectiveGame, Log, TEXT("Enemy Stunned: %s"), *Enemy->GetName());
			return;
		}
	}
}

void AStealthDetectiveGameCharacter::EnableDetectiveMode()
{
	AStealthDetectiveGamePlayerController* MyController = Cast<AStealthDetectiveGamePlayerController>(GetController());
	if (MyController)
	{
		if (!bDetectiveMode)
		{
			MyController->EnableMappingContext(FName("Detective Mode"));
			bDetectiveMode = true;
			
			// Add post process material to PhotoCamera
			if (DetectiveModePostProcessMaterial)
			{
				if (PhotoCamera)
				{
					PhotoCamera->PostProcessSettings.AddBlendable(DetectiveModePostProcessMaterial, 1.0f);
				}
				if (FollowCamera)
				{
					FollowCamera->PostProcessSettings.AddBlendable(DetectiveModePostProcessMaterial, 1.0f);
				}
			}
			if (bHasActiveTrail)
			{
				OnActiveTrail.Broadcast(true);
			}
			
		}
		else
		{
			MyController->DisableMappingContext(FName("Detective Mode"));
			bDetectiveMode = false;

			// Remove post process material
			if (DetectiveModePostProcessMaterial)
			{
				if (PhotoCamera)
				{
					PhotoCamera->PostProcessSettings.RemoveBlendable(DetectiveModePostProcessMaterial);
				}
				if (FollowCamera)
				{
					FollowCamera->PostProcessSettings.RemoveBlendable(DetectiveModePostProcessMaterial);
				}
			}
			if (bHasActiveTrail)
			{
				OnActiveTrail.Broadcast(false);
			}
		}
	}
}

void AStealthDetectiveGameCharacter::StartScanning()
{
	if (!bIsThirdPerson){}
}

void AStealthDetectiveGameCharacter::EvidenceScanned()
{
	if (!bIsThirdPerson)
	{
		AStealthTrailStart* TrailStart = Cast<AStealthTrailStart>(EvidenceInView());
		if (TrailStart)
		{
			UE_LOG(LogStealthDetectiveGame, Log, TEXT("Trail Start Found: %s"), *TrailStart->GetName());
			if (TrailStart->GameplayTags.HasTag(FGameplayTag::RequestGameplayTag("Evidence.Trail")))
			{
				UE_LOG(LogStealthDetectiveGame, Log, TEXT("Trail Evidence Found"));
				for (const FGameplayTag& Tag : TrailStart->GameplayTags.GetGameplayTagArray())
				{
					if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag("Evidence.Trail")))
					{
						UE_LOG(LogStealthDetectiveGame, Log, TEXT("Trail Found: %s"), *Tag.ToString());
						bHasActiveTrail = true;
						OnEvidenceFound.Broadcast(Tag);
						return;
					}
				}
			}
			
		}
	}
	
}


// Public Interface: Movement, Camera, and Events
void AStealthDetectiveGameCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AStealthDetectiveGameCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AStealthDetectiveGameCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AStealthDetectiveGameCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
