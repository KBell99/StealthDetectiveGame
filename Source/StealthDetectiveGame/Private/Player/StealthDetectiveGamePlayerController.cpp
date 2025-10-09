// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/StealthDetectiveGamePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "StealthDetectiveGame.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AStealthDetectiveGamePlayerController::AddInputMappingContext(UInputMappingContext* InputMappingContext,
	int32 Priority)
{
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, Priority);
		}
	}
}

void AStealthDetectiveGamePlayerController::RemoveInputMappingContext(UInputMappingContext* InputMappingContext)
{
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(InputMappingContext);
		}
	}
}

void AStealthDetectiveGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogStealthDetectiveGame, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AStealthDetectiveGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
	{
		AddInputMappingContext(CurrentContext, 0);
	}

	// only add these IMCs if we're not using mobile touch input
	if (!SVirtualJoystick::ShouldDisplayTouchInterface())
	{
		for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
		{
			AddInputMappingContext(CurrentContext, 0);
		}
	}
}

bool AStealthDetectiveGamePlayerController::EnableMappingContext(FName ContextName)
{
	if (ToggleableMappingContexts.Contains(ContextName))
	{
		AddInputMappingContext(ToggleableMappingContexts[ContextName], 0);
		return true;
	}
	
	return false;
}

bool AStealthDetectiveGamePlayerController::DisableMappingContext(FName ContextName)
{
	if (ToggleableMappingContexts.Contains(ContextName))
	{
		RemoveInputMappingContext(ToggleableMappingContexts[ContextName]);
		return true;
	}
	
	return false;
}

