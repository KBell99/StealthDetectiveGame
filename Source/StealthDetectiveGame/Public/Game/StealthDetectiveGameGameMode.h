// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameModeBase.h"
#include "StealthDetectiveGameGameMode.generated.h"

class AStealthDetectiveGameCharacter;
/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AStealthDetectiveGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

	UFUNCTION()
	void SetActiveTrailVisibility(bool bVisible);

	UFUNCTION()
	void EvidenceFound(FGameplayTag EvidenceTag);

	UAudioComponent* BGMComponent;
	
public:
	
	virtual void BeginPlay() override;
	
	virtual void ShowStoryScreen(AStealthDetectiveGameCharacter* PlayerCharacter);
	
	UFUNCTION(BlueprintCallable)
	virtual void StartGame();
	

	UFUNCTION()
	void SetActiveTrailTag(FGameplayTag NewActiveTrailTag);
	
	void CheckObjectiveCompletion(FGameplayTag ObjectiveTag);

	void PlayerDied(AStealthDetectiveGameCharacter* DeadCharacter);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	USoundBase* EvidenceFoundSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	USoundBase* PlayerDeathSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	USoundBase* BackgroundMusic;
	

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> GameEndingMap;
	
	void TravelToMap(const FString& MapName);
	
	UPROPERTY(EditDefaultsOnly)
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;

	
};



