// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StealthEnemy.h"

#include "AI/StealthAIController.h"
#include "Character/StealthDetectiveGameCharacter.h"
#include "Perception/AIPerceptionComponent.h"

void AStealthEnemy::AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// clear the attacking flag
	bIsAttacking = false;

	OnAttackCompleted.Broadcast();
}

AStealthEnemy::AStealthEnemy()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

	OnAttackMontageEnded.BindUObject(this, &AStealthEnemy::AttackMontageEnded);
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

void AStealthEnemy::DoAttackTrace(FName DamageSourceBone)
{
	// sweep for objects in front of the character to be hit by the attack
	TArray<FHitResult> OutHits;

	// start at the provided socket location, sweep forward
	const FVector TraceStart = GetMesh()->GetSocketLocation(DamageSourceBone);
	const FVector TraceEnd = TraceStart + (GetActorForwardVector() * MeleeTraceDistance);

	// enemies only affect Pawn collision objects; they don't knock back boxes
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	// use a sphere shape for the sweep
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(MeleeTraceRadius);

	// ignore self
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, CollisionShape, QueryParams))
	{
		// iterate over each object hit
		for (const FHitResult& CurrentHit : OutHits)
		{
			/** does the actor have the player tag? */
			if (AStealthDetectiveGameCharacter* PlayerCharacter = Cast<AStealthDetectiveGameCharacter>(CurrentHit.GetActor()))
			{
				PlayerCharacter->Stun();
				
			}
		}
	}
}

void AStealthEnemy::DoAIPunchAttack()
{
	// ignore if we're already playing an attack animation
	if (bIsAttacking)
	{
		return;
	}

	// raise the attacking flag
	bIsAttacking = true;


	// play the attack montage
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(PunchAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

		// subscribe to montage completed and interrupted events
		if (MontageLength > 0.0f)
		{
			// set the end delegate for the montage
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, PunchAttackMontage);
		}
	}
}
