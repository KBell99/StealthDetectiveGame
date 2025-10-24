// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/StealthDocument.h"

#include "Character/StealthDetectiveGameCharacter.h"
#include "Components/BoxComponent.h"
#include "Player/StealthDetectiveGamePlayerController.h"
#include "UI/StealthHUD.h"

// Sets default values
AStealthDocument::AStealthDocument()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DocumentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DocumentMesh"));
	RootComponent = DocumentMesh;
	InteractableCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractableCollider"));
	InteractableCollider->SetupAttachment(RootComponent);
}

void AStealthDocument::OnInteract(APawn* InteractingPawn)
{
	if (AStealthDetectiveGameCharacter* PlayerCharacter = Cast<AStealthDetectiveGameCharacter>(InteractingPawn))
	{
		if (AStealthDetectiveGamePlayerController* PlayerController = Cast<AStealthDetectiveGamePlayerController>(PlayerCharacter->GetController()))
		{
			if (AStealthHUD* HUD = Cast<AStealthHUD>(PlayerController->GetHUD()))
			{
				HUD->ShowDocument(GameplayTag);
			}
		}
	}
	
}

