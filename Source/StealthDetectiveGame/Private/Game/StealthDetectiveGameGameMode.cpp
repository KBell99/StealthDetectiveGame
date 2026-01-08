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
#include "Player/StealthDetectiveGamePlayerController.h"
#include "UI/StealthHUD.h"

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
	
	UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance());
	if (GI && GI->bShowStoryScreen)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UGameplayStatics::SetGamePaused(World, true);
		}
		ShowStoryScreen(PlayerCharacter);
	} else
	{
		StartGame();
	}
}

void AStealthDetectiveGameGameMode::ShowStoryScreen(AStealthDetectiveGameCharacter* PlayerCharacter)
{
	if (PlayerCharacter)
	{
		AStealthDetectiveGamePlayerController* PC = Cast<AStealthDetectiveGamePlayerController>(PlayerCharacter->GetController());
		if (PC)
		{
			AStealthHUD* StealthHUD = Cast<AStealthHUD>(PC->GetHUD());
			if (StealthHUD)
			{
				StealthHUD->ShowStoryScreen(PC);
			}
		}
	}	
}

void AStealthDetectiveGameGameMode::StartGame_Implementation()
{
	UWorld* World = GetWorld();
	if (World)
	{
		UGameplayStatics::SetGamePaused(World, false);
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
	

	UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, PlayerDeathSound, DeadCharacter->GetActorLocation());

	if (AudioComponent)
	{
		AudioComponent->OnAudioFinishedNative.AddLambda([this](UAudioComponent* FinishedComponent)
		{
			EndGame();
		});
	}
}

void AStealthDetectiveGameGameMode::TravelToMap(const FString& MapName)
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Maps.FindChecked(MapName));
};

void AStealthDetectiveGameGameMode::EndGame()
{
	AStealthGameStateBase* GS = GetGameState<AStealthGameStateBase>();
	checkf(GS, TEXT("GameState is not of type AStealthGameStateBase"));
	UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance());
	checkf(GI, TEXT("GameInstance is not of type UStealthGameInstance"));
	
	bool bAllObjectivesCompleted = GS->AllObjectivesCompleted();
	GI->bObjectivesCompleted = bAllObjectivesCompleted;
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameEndingMap);
}
