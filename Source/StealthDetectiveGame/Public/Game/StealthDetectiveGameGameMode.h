// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameModeBase.h"
#include "StealthDetectiveGameGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AStealthDetectiveGameGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	void SetActiveTrailVisibility(bool bVisible);
	void SetActiveTrailTag(FGameplayTag NewActiveTrailTag);

	
public:
	
	/** Constructor */
	AStealthDetectiveGameGameMode();

	virtual void BeginPlay() override;
};



