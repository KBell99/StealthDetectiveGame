// Fill out your copyright notice in the Description page of Project Settings.


#include "Objective/StealthEvidence.h"

#include "StealthDetectiveGame.h"
#include "Character/StealthDetectiveGameCharacter.h"

// Sets default values
AStealthEvidence::AStealthEvidence()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AStealthEvidence::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStealthEvidence::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AStealthEvidence::OnEvidencePhotographed_Implementation(AStealthDetectiveGameCharacter* PhotographingCharacter)
{
	for (const FGameplayTag& Tag : GameplayTags.GetGameplayTagArray())
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag("Evidence.Objective")))
		{
			UE_LOG(LogStealthDetectiveGame, Log, TEXT("Objective Evidence Found: %s"), *Tag.ToString());
			PhotographingCharacter->OnEvidenceFound.Broadcast(Tag);
		}
	}

}

