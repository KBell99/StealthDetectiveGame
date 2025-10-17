// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StealthEnemy.h"

#include "AI/StealthAIController.h"
#include "Perception/AIPerceptionComponent.h"

AStealthEnemy::AStealthEnemy()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
}

FVector AStealthEnemy::GetCurrentPatrolPointLocation()
{
	return PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation();
}

FVector AStealthEnemy::GetNextPatrolPointLocation()
{
	CurrentPatrolPointIndex = (CurrentPatrolPointIndex + 1) % PatrolPoints.Num();

	return PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation();
}

void AStealthEnemy::Stun(float HitDistance)
{
	Super::Stun(HitDistance);
	
	bIsStunned = true;

	if (AStealthAIController* AIController = Cast<AStealthAIController>(Controller))
	{
		AIController->PawnIsStunned(HitDistance);
	}
}

