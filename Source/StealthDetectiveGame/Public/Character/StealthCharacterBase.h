
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StealthCharacterBase.generated.h"

UCLASS()
class STEALTHDETECTIVEGAME_API AStealthCharacterBase : public ACharacter
{
	GENERATED_BODY()


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	bool bIsStunned = false;

public:
	// Sets default values for this character's properties
	AStealthCharacterBase();
	virtual void Stun(float HitDistance = 0.0f);

	UFUNCTION(BlueprintCallable, Category="State")
	bool IsStunned() const { return bIsStunned; }
	UFUNCTION(BlueprintCallable, Category="State")
	void SetIsStunned(bool bStunned) { bIsStunned = bStunned; }
	
};
