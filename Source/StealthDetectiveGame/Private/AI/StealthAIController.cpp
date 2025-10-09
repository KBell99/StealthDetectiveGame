// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StealthAIController.h"

#include "Components/StateTreeAIComponent.h"

AStealthAIController::AStealthAIController()
{
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	check(StateTreeAI);

	bStartAILogicOnPossess = true;

	bAttachToPawn = true;
}
