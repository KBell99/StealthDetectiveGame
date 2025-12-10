// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/StealthGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "Objective/StealthTrailMarkerManager.h"

void AStealthGameStateBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStealthTrailMarkerManager::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AStealthTrailMarkerManager* Manager = Cast<AStealthTrailMarkerManager>(Actor);
		if (Manager)
		{
			FName TagName = Manager->GameplayTag.GetTagName();
			Trails.Add(TagName, Manager);
		}
	}
}

bool AStealthGameStateBase::IsObjectiveCompleted(FGameplayTag ObjectiveName) const
{
	if (ObjectiveCompletionMap.Contains(ObjectiveName))
	{
		return *ObjectiveCompletionMap.Find(ObjectiveName);
	}
	
	return false;
}

void AStealthGameStateBase::SetObjectiveCompleted(FGameplayTag ObjectiveName, bool bCompleted)
{
	ObjectiveCompletionMap.Add(ObjectiveName, bCompleted);
}

bool AStealthGameStateBase::AllObjectivesCompleted()
{
	TArray<bool> ObjectiveCompletionMapValues;
	ObjectiveCompletionMap.GenerateValueArray(ObjectiveCompletionMapValues);
	if (ObjectiveCompletionMapValues.Num() == 0)
	{
		return false;
	}
	
	for (bool bCompleted : ObjectiveCompletionMapValues)
	{
		if (!bCompleted)
		{
			return false;
		}
	}

	return true;
}
