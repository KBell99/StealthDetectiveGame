// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StealthCharacterBase.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "StealthDetectiveGameCharacter.generated.h"

class UTimelineComponent;
class UAIPerceptionStimuliSourceComponent;
class AStealthEvidence;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FEvidenceFound, FGameplayTag /*EvidenceTag*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FActiveTrail, bool /*bHasActiveTrail*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCameraToggle, bool, bIsCameraEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCameraFlashToggle, bool, bIsCameraFlashEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlashPictureTaken, float, FlashCooldownTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDetectiveScanTimer, float, ScanDuration);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AStealthDetectiveGameCharacter : public AStealthCharacterBase
{
	GENERATED_BODY()
	
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* PhotoCamera;

	// Collider and State Variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Photo", meta = (AllowPrivateAccess = "true"))
	FVector BoxExtent = FVector(100.0f, 200.0f, 200.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Photo", meta = (AllowPrivateAccess = "true"))
	float PhotoTraceEnd = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Photo", meta = (AllowPrivateAccess = "true"))
	FVector FlashBoxExtent = FVector(200.0f, 200.0f, 200.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Photo", meta = (AllowPrivateAccess = "true"))
	float FlashTraceEnd = 200.0f;

	// initialize flash cooldown members (default already set on declaration, reaffirm here)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Photo", meta = (AllowPrivateAccess = "true"))
	float FlashCooldown = 0.f;
	float LastFlashTime = -10000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Photo", meta = (AllowPrivateAccess = "true"))
	TScriptInterface<IBlendableInterface> DetectiveModePostProcessMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;
	
	bool bIsCameraEnabled = false;
	bool bDetectiveMode = false;
	bool bMenuIsOpen = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bIsThirdPerson = true;
	
	bool bIsCameraFlashEnabled = false;
	bool bHasActiveTrail = false;

	AStealthEvidence* EvidenceInView() const;

	
protected:
	// Input Actions
	UPROPERTY(EditAnywhere, Category="Input|Default")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input|Default")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category="Input|Default")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input|Default")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input|Default")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category="Input|Default")
	UInputAction* EnableCameraAction;

	UPROPERTY(EditAnywhere, Category="Input|Default")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category="Input|Default")
	UInputAction* OpenSettingsAction;
	
	UPROPERTY(EditAnywhere, Category="Input|Camera")
	UInputAction* ZoomInAction;

	UPROPERTY(EditAnywhere, Category="Input|Camera")
	UInputAction* ZoomOutAction;

	UPROPERTY(EditAnywhere, Category="Input|Camera")
	UInputAction* TakePictureAction;

	UPROPERTY(EditAnywhere, Category="Input|Camera")
	UInputAction* EnableCameraFlashAction;

	UPROPERTY(EditAnywhere, Category="Input|Default")
	UInputAction* EnableDetectiveModeAction;
	
	UPROPERTY(EditAnywhere, Category="Input|Detective")
	UInputAction* EvidenceScanAction;
	
	

	// Construction and Input Setup
	AStealthDetectiveGameCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Internal Input Handling
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void DoCrouch(const FInputActionValue& Value);
	void Interact();
	void OpenSettings();
	
	UFUNCTION(BlueprintCallable)
	void EnableCamera();
	void ZoomInCamera(const FInputActionValue& Value);
	void ZoomOutCamera(const FInputActionValue& Value);
	void TakePicture();

	UFUNCTION(BlueprintCallable)
	void EnableCameraFlash();
	void EnableDetectiveMode();
	void StartScanning(const FInputActionValue& Value);
	void EvidenceScanned();


	void FlashPhotography();
	
public:
	// Public Interface: Movement, Camera, and Events
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	virtual void Stun(float HitDistance = 0.f) override;
	virtual void HandleDeath();
	
	FEvidenceFound OnEvidenceFound;
	FActiveTrail OnActiveTrail;
	UPROPERTY(BlueprintAssignable, Category="Event|Input")
	FCameraToggle OnCameraToggle;
	UPROPERTY(BlueprintAssignable, Category="Event|Input")
	FCameraFlashToggle OnCameraFlashToggle;
	UPROPERTY(BlueprintAssignable, Category="Event|Photo")
	FFlashPictureTaken OnFlashPictureTaken;
	UPROPERTY(BlueprintAssignable, Category="Event|Detective")
	FDetectiveScanTimer OnDetectiveScan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default|Timeline")
	UTimelineComponent* FlashCooldownTimeline;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default|Timeline")
	UTimelineComponent* ScanTimeline;
	
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintImplementableEvent)
	void ToggleCamera();
};
