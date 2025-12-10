// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/StealthDetectiveGameGameMode.h"

#include "StealthDetectiveGame.h"
#include "Character/StealthDetectiveGameCharacter.h"
#include "Components/AudioComponent.h"
#include "Game/StealthGameInstance.h"
#include "Game/StealthGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Objective/StealthTrailMarker.h"
#include "Objective/StealthTrailMarkerManager.h"

void AStealthDetectiveGameGameMode::EvidenceFound(FGameplayTag EvidenceTag)
{
	if (EvidenceTag.MatchesTag(FGameplayTag::RequestGameplayTag("Evidence.Trail")))
	{
		SetActiveTrailTag(EvidenceTag);
	}
	if (EvidenceTag.MatchesTag(FGameplayTag::RequestGameplayTag("Evidence.Objective")))
	{
		AStealthGameStateBase* GS = GetGameState<AStealthGameStateBase>();
		if (GS)
		{
			if (GS->IsObjectiveCompleted(EvidenceTag))
			{
				return;
			}
			
			GS->SetObjectiveCompleted(EvidenceTag, true);
			if (EvidenceFoundSound)
			{
				AStealthDetectiveGameCharacter* PlayerCharacter = Cast<AStealthDetectiveGameCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
				if (PlayerCharacter)
				{
					UGameplayStatics::PlaySoundAtLocation(this, EvidenceFoundSound, PlayerCharacter->GetActorLocation());
				}
			}
			
		}
		
	}
}

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
				if (!TrailMarker) continue;
				TrailMarker->SetActorHiddenInGame(!bVisible);
				TrailMarker->GetRootComponent()->SetVisibility(bVisible, true);
			}
		}
	}
}

void AStealthDetectiveGameGameMode::SetActiveTrailTag(FGameplayTag NewActiveTrailTag)
{
	AStealthGameStateBase* GS = GetGameState<AStealthGameStateBase>();
	SetActiveTrailVisibility(false);
	GS->SetActiveTrailTag(NewActiveTrailTag);
	SetActiveTrailVisibility(true);
	
}

void AStealthDetectiveGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	AStealthDetectiveGameCharacter* PlayerCharacter = Cast<AStealthDetectiveGameCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (PlayerCharacter)
	{
		PlayerCharacter->OnEvidenceFound.AddDynamic(this, &AStealthDetectiveGameGameMode::EvidenceFound);
		PlayerCharacter->OnActiveTrail.AddUObject(this, &AStealthDetectiveGameGameMode::SetActiveTrailVisibility);
		PlayerCharacter->OnPlayerDead.AddUObject(this, &AStealthDetectiveGameGameMode::PlayerDied);
	}

	if (BackgroundMusic)
	{
		BGMComponent = UGameplayStatics::SpawnSound2D(GetWorld(), BackgroundMusic);
	}
}

void AStealthDetectiveGameGameMode::CheckObjectiveCompletion(FGameplayTag ObjectiveTag)
{
	AStealthGameStateBase* GS = GetGameState<AStealthGameStateBase>();
	if (GS && GS->IsObjectiveCompleted(ObjectiveTag))
	{
		
	}
}

void AStealthDetectiveGameGameMode::PlayerDied(AStealthDetectiveGameCharacter* DeadCharacter)
{
	if (BGMComponent != nullptr)
	{
		BGMComponent->Stop();
	}
	
	AStealthGameStateBase* GS = GetGameState<AStealthGameStateBase>();
	checkf(GS, TEXT("GameState is not of type AStealthGameStateBase"));
	UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance());
	checkf(GI, TEXT("GameInstance is not of type UStealthGameInstance"));
	UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, PlayerDeathSound, DeadCharacter->GetActorLocation());

	if (AudioComponent)
	{
		AudioComponent->OnAudioFinishedNative.AddLambda([this, GS, GI](UAudioComponent* FinishedComponent)
		{
			UE_LOG(LogStealthDetectiveGame, Log, TEXT("Restarting Level after Player Death Sound Finished"));

			bool bAllObjectivesCompleted = GS->AllObjectivesCompleted();
			GI->bObjectivesCompleted = bAllObjectivesCompleted;
			UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameEndingMap);
		});
	}
}

void AStealthDetectiveGameGameMode::TravelToMap(const FString& MapName)
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Maps.FindChecked(MapName));
};
