// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StealthHUD.h"

#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void AStealthHUD::ShowSettingsMenu(APlayerController* PlayerController)
{
	/*if (SettingsMenu)
	{
		SettingsMenu->SetVisibility(ESlateVisibility::Visible);
		//UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, SettingsMenu, EMouseLockMode::DoNotLock, true);
	} 
	else
	{*/
		if (SettingsMenuClass)
		{
			SettingsMenu = CreateWidget<UUserWidget>(GetWorld(), SettingsMenuClass);
			SettingsMenu->AddToViewport();
			PlayerController->SetPause(true);
			//UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, SettingsMenu, EMouseLockMode::DoNotLock, true);
		}
	/*}*/

	//PlayerController->bShowMouseCursor = true;
}

void AStealthHUD::HideSettingsMenu(APlayerController* PlayerController)
{
	if (SettingsMenu)
	{
		SettingsMenu->SetVisibility(ESlateVisibility::Hidden);
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController);
	}

	PlayerController->bShowMouseCursor = false;
}

void AStealthHUD::ShowDeathScreen_Implementation()
{
}
