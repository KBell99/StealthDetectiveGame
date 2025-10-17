// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ScalableFloat.h"
#include "StealthAIController.generated.h"

USTRUCT(BlueprintType)
struct FStunDuration
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	float Duration;
};

USTRUCT(BlueprintType)
struct FPlayerLocationInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	ACharacter* PlayerCharacter;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	float TimeSinceLastSeen;
};

class UStateTreeAIComponent;
/**
 * 
 */
UCLASS()
class STEALTHDETECTIVEGAME_API AStealthAIController : public AAIController
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStateTreeAIComponent* StateTreeAI;

public:
	AStealthAIController();
	void PawnIsStunned(float HitDistance);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FScalableFloat StunDurations;
};
