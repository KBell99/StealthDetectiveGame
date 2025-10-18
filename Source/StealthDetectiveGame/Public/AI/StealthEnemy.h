// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatAttacker.h"
#include "CombatEnemy.h"
#include "Character/StealthCharacterBase.h"
#include "ScalableFloat.h"
#include "StealthEnemy.generated.h"


class UAIPerceptionComponent;
/** Patrol Target */
DECLARE_MULTICAST_DELEGATE(FOnTargetLocationGet);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackCompleted);


/**
 * 
 */
UCLASS()
class STEALTHDETECTIVEGAME_API AStealthEnemy : public AStealthCharacterBase, public ICombatAttacker
{
	GENERATED_BODY()
	
	int32 CurrentPatrolPointIndex = 0;

	UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess))
	UAIPerceptionComponent* AIPerceptionComponent;
	

protected:
	/** Distance ahead of the character that melee attack sphere collision traces will extend */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float MeleeTraceDistance = 75.0f;

	/** Radius of the sphere trace for melee attacks */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float MeleeTraceRadius = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsAttacking = false;


	/** AnimMontage that will play for combo attacks */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Combo")
	UAnimMontage* PunchAttackMontage;

	FOnMontageEnded OnAttackMontageEnded;

	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bPlayerInSight = false;

	

	/** Performs an attack's collision check. Usually called from a montage's AnimNotify */
	UFUNCTION(BlueprintCallable, Category="Attacker")
	virtual void DoAttackTrace(FName DamageSourceBone) override;

	virtual void CheckCombo() override
	{
	}

	virtual void CheckChargedAttack() override
	{
	}

	UFUNCTION(BlueprintCallable, Category="Attacker")
	void DoAIPunchAttack();

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnAttackCompleted OnAttackCompleted;

};
