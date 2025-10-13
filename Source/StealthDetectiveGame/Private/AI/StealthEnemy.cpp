// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StealthEnemy.h"

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

	StunDuration = StunDurations.GetValueAtLevel(HitDistance);
	bIsStunned = true;
}

