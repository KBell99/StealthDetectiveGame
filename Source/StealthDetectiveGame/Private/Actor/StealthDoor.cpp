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

void AStealthDoor::DoorAnimationFinished()
{
	bIsAnimating = false;
	// clear timer in case it's reused
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AnimationTimerHandle);
	}
}

void AStealthDoor::OnInteract()
{
	if (bIsDoorOpen)
	{
		if (DoorCloseAnimation && !bIsAnimating)
		{
			bIsAnimating = true;
			DoorMesh->PlayAnimation(DoorCloseAnimation, false);

			if (GetWorld())
			{
				const float Length = DoorCloseAnimation->GetPlayLength();
				if (Length > KINDA_SMALL_NUMBER)
				{
					GetWorld()->GetTimerManager().SetTimer(AnimationTimerHandle, this, &AStealthDoor::DoorAnimationFinished, Length, false);
				}
				else
				{
					DoorAnimationFinished();
				}
			}
			bIsDoorOpen = false;
		}
	}
	else
	{
		if (DoorOpenAnimation && !bIsAnimating)
		{
			bIsAnimating = true;
			DoorMesh->PlayAnimation(DoorOpenAnimation, false);

			if (GetWorld())
			{
				const float Length = DoorOpenAnimation->GetPlayLength();
				if (Length > KINDA_SMALL_NUMBER)
				{
					GetWorld()->GetTimerManager().SetTimer(AnimationTimerHandle, this, &AStealthDoor::DoorAnimationFinished, Length, false);
				}
				else
				{
					DoorAnimationFinished();
				}
			}
			bIsDoorOpen = true;
		}
	}
}
