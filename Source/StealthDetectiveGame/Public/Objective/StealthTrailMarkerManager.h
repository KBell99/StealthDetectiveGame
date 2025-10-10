// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "StealthTrailMarkerManager.generated.h"

UCLASS()
class STEALTHDETECTIVEGAME_API AStealthTrailMarkerManager : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere, Category="Objective")
	TArray<class AStealthTrailMarker*> TrailMarkers;

	UPROPERTY(EditAnywhere, Category="Objective")
	FGameplayTag GameplayTag;

};
