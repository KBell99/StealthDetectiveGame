// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "StealthAIController.generated.h"

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
};
