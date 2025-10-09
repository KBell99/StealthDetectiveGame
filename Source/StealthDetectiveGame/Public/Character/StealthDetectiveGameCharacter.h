// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StealthCharacterBase.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "StealthDetectiveGameCharacter.generated.h"

class AStealthEvidence;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FEvidenceFound, AStealthEvidence* /*FoundEvidence*/);

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
	TScriptInterface<IBlendableInterface> DetectiveModePostProcessMaterial;
	
	bool bIsCameraEnabled = false;
	bool bDetectiveMode = false;
	bool bIsThirdPerson = true;
	bool bIsCameraFlashEnabled = false;

	
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
	void EnableCamera();
	void ZoomInCamera(const FInputActionValue& Value);
	void ZoomOutCamera(const FInputActionValue& Value);
	void TakePicture();
	void EnableCameraFlash();
	void EnableDetectiveMode();
	void StartScanning();
	void EvidenceScanned();
	
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

	FEvidenceFound OnEvidenceFound;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintImplementableEvent)
	void ToggleCamera(bool bThirdPerson);
};
