// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "StealthGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class STEALTHDETECTIVEGAME_API UStealthGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bObjectivesCompleted = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowStoryScreen = true;
};
