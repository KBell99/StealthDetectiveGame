// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StealthHUD.generated.h"

/**
 * 
 */
UCLASS()
class STEALTHDETECTIVEGAME_API AStealthHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Defaults|Widgets")
	UUserWidget* CameraOverlay;
};
