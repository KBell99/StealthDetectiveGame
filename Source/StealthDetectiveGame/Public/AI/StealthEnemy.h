// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/StealthCharacterBase.h"
#include "ScalableFloat.h"
#include "StealthEnemy.generated.h"


class UAIPerceptionComponent;
/** Patrol Target */
DECLARE_MULTICAST_DELEGATE(FOnTargetLocationGet);


/**
 * 
 */
UCLASS()
class STEALTHDETECTIVEGAME_API AStealthEnemy : public AStealthCharacterBase
{
	GENERATED_BODY()
	
	int32 CurrentPatrolPointIndex = 0;

	UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess))
	UAIPerceptionComponent* AIPerceptionComponent;
	
	
	
public:
	AStealthEnemy();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<AActor*> PatrolPoints;
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	FVector GetCurrentPatrolPointLocation();
	UFUNCTION(BlueprintCallable, Category = "AI")
	FVector GetNextPatrolPointLocation();

	FOnTargetLocationGet OnTargetLocationGet;
	
	virtual void Stun(float HitDistance = 0.0f) override;
};
