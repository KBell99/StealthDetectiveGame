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

void AStealthAIController::PawnIsStunned(float HitDistance)
{
	FStunDuration StunDuration;
	StunDuration.Duration = StunDurations.GetValueAtLevel(HitDistance);

	if (StateTreeAI)
	{
		FStateTreeEvent StunEvent;

		StunEvent.Tag = FGameplayTag::RequestGameplayTag(FName("Status.Stunned"));
		StunEvent.Payload = FConstStructView::Make(StunDuration);
		StunEvent.Origin = FName("Pawn");
		
		StateTreeAI->SendStateTreeEvent(StunEvent);
	}
	
}
