// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Interface/Scannable.h"
#include "StealthEvidence.generated.h"

class AStealthDetectiveGameCharacter;

UCLASS()
class STEALTHDETECTIVEGAME_API AStealthEvidence : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStealthEvidence();

	
	UPROPERTY(EditAnywhere, Category="Gameplay")
	FGameplayTagContainer GameplayTags;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gameplay")
	void OnEvidencePhotographed(AStealthDetectiveGameCharacter* PhotographingCharacter);

};
