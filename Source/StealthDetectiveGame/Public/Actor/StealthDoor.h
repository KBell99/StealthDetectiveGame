// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Interactable.h"
#include "StealthDoor.generated.h"

class UBoxComponent;

UCLASS()
class STEALTHDETECTIVEGAME_API AStealthDoor : public AActor, public IInteractable
{
	GENERATED_BODY()
	


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* DoorMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UBoxComponent* InteractableCollider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Animations")
	UAnimationAsset* DoorOpenAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Animations")
	UAnimationAsset* DoorCloseAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|State")
	bool bIsDoorOpen;

	
public:	
	// Sets default values for this actor's properties
	AStealthDoor();

	UFUNCTION(BlueprintCallable)
	virtual void OnInteract() override;
};
