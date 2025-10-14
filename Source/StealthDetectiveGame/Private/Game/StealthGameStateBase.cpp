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
