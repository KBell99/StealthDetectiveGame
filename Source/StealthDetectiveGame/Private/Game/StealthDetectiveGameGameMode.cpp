// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/StealthDetectiveGameGameMode.h"

#include "Character/StealthDetectiveGameCharacter.h"
#include "Game/StealthGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Objective/StealthTrailMarker.h"
#include "Objective/StealthTrailMarkerManager.h"

void AStealthDetectiveGameGameMode::SetActiveTrailVisibility(bool bVisible)
{
	AStealthGameStateBase* GS = GetGameState<AStealthGameStateBase>();
	FGameplayTag ActiveTrailTag = GS->GetActiveTrailTag();
	FName TrailName = ActiveTrailTag.GetTagName();

	if (GS)
	{
		if (GS->GetTrails().Contains(TrailName))
		{
			AStealthTrailMarkerManager* TrailMarkerManager = *GS->GetTrails().Find(TrailName);
			
			for (AStealthTrailMarker* TrailMarker : TrailMarkerManager->TrailMarkers)
			{
				TrailMarker->SetActorHiddenInGame(!bVisible);
				TrailMarker->GetRootComponent()->SetVisibility(bVisible, true);
			}
		}
	}
}

void AStealthDetectiveGameGameMode::SetActiveTrailTag(FGameplayTag NewActiveTrailTag)
{
	AStealthGameStateBase* GS = GetGameState<AStealthGameStateBase>();
	GS->SetActiveTrailTag(NewActiveTrailTag);
	SetActiveTrailVisibility(true);
}

void AStealthDetectiveGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	AStealthDetectiveGameCharacter* PlayerCharacter = Cast<AStealthDetectiveGameCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	PlayerCharacter->OnEvidenceFound.AddUObject(this, &AStealthDetectiveGameGameMode::SetActiveTrailTag);
	PlayerCharacter->OnActiveTrail.AddUObject(this, &AStealthDetectiveGameGameMode::SetActiveTrailVisibility);
}

AStealthDetectiveGameGameMode::AStealthDetectiveGameGameMode()
{
	// stub
}



