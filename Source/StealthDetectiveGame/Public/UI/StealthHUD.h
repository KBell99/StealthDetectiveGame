// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/HUD.h"
#include "StealthHUD.generated.h"

USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DocumentWidth = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DocumentHeight = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UUserWidget> DocumentWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;
};

/**
 * 
 */
UCLASS()
class STEALTHDETECTIVEGAME_API AStealthHUD : public AHUD
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UDataTable> DocumentPopUpDataTable;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Defaults|Widgets")
	UUserWidget* CameraOverlay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Defaults|Widgets")
	TSubclassOf<UUserWidget> SettingsMenuClass;

	UPROPERTY(BlueprintReadWrite, Category="Defaults|Widgets")
	UUserWidget* SettingsMenu;
	
	UPROPERTY(EditDefaultsOnly, Category="Defaults|Widgets")
	TSubclassOf<UUserWidget> StoryMenuClass;
	
	UPROPERTY(BlueprintReadOnly, Category="Defaults|Widgets")
	UUserWidget* StoryMenu;
	
	UFUNCTION(BlueprintCallable, Category="Defaults|Widgets")
	void ShowSettingsMenu(APlayerController* PlayerController);
	UFUNCTION(BlueprintCallable, Category="Defaults|Widgets")
	void HideSettingsMenu(APlayerController* PlayerController);

	UFUNCTION(BlueprintImplementableEvent, Category="Defaults|Widgets")
	void ShowDocument(FGameplayTag GameplayTag);

	UFUNCTION(BlueprintNativeEvent, Category="Defaults|Widgets")
	void ShowDeathScreen();
	
	UFUNCTION(BlueprintCallable, Category="Defaults|Widgets")
	void ShowStoryScreen(APlayerController* PlayerController);
};
