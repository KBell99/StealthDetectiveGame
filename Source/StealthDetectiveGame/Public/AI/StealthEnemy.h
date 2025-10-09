// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/StealthCharacterBase.h"
#include "StealthEnemy.generated.h"
/** Patrol Target */
DECLARE_MULTICAST_DELEGATE(FOnTargetLocationGet);

/**
 * 
 */
UCLASS()
class STEALTHDETECTIVEGAME_API AStealthEnemy : public AStealthCharacterBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<AActor*> PatrolPoints;
	int32 CurrentPatrolPointIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "AI")
	FVector GetCurrentPatrolPointLocation();
	UFUNCTION(BlueprintCallable, Category = "AI")
	FVector GetNextPatrolPointLocation();

	FOnTargetLocationGet OnTargetLocationGet;
};
