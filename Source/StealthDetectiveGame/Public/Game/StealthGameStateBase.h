// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameStateBase.h"
#include "StealthGameStateBase.generated.h"

/**
 * 
 */

class AStealthTrailMarkerManager;

UCLASS()
class STEALTHDETECTIVEGAME_API AStealthGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category="Objective", meta=(AllowPrivateAccess="true"))
	TMap<FName, AStealthTrailMarkerManager*> Trails;

	UPROPERTY(EditAnywhere, Category="Objective", meta=(AllowPrivateAccess="true"))
	FGameplayTag ActiveTrailTag;

public:
	virtual void PostInitializeComponents() override;

	TMap<FName, AStealthTrailMarkerManager*> GetTrails() { return Trails; }
	FGameplayTag GetActiveTrailTag() const { return ActiveTrailTag; }
	void SetActiveTrailTag(FGameplayTag NewActiveTrailTag) { ActiveTrailTag = NewActiveTrailTag; }
};
