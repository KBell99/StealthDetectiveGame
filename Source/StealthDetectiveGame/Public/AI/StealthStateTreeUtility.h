// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"

#include "StealthStateTreeUtility.generated.h"

class AStealthEnemy;

USTRUCT()
struct FStateTreeGetTargetActorInfoInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ACharacter> OwnerActor;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(VisibleAnywhere)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere)
	float Distance = 0.0f;
};

// USTRUCT(meta=(Display="GetTargetActorInfo"), Category="Stealth")
// struct FStateTreeGetTargetActorInfoTask : public FStateTreeTaskCommonBase
// {
// 	GENERATED_BODY()
//
// 	using FInstanceDataType = FStateTreeGetTargetActorInfoInstanceData;
// 	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
//
// 	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
// 	
// };