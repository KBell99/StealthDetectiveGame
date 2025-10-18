// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/StealthDoor.h"

#include "Components/BoxComponent.h"

// Sets default values
AStealthDoor::AStealthDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DoorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;
	InteractableCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractableCollider"));
	InteractableCollider->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AStealthDoor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AStealthDoor::OnInteract()
{
	if (bIsDoorOpen)
	{
		if (DoorCloseAnimation)
		{
			DoorMesh->PlayAnimation(DoorCloseAnimation, false);
			bIsDoorOpen = false;
		}
	}
	else
	{
		if (DoorOpenAnimation)
		{
			DoorMesh->PlayAnimation(DoorOpenAnimation, false);
			bIsDoorOpen = true;
		}
	}
}

