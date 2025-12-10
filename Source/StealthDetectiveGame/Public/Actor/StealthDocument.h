// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Interface/Interactable.h"
#include "StealthDocument.generated.h"

class UBoxComponent;

UCLASS()
class STEALTHDETECTIVEGAME_API AStealthDocument : public AActor, public IInteractable
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DocumentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UBoxComponent* InteractableCollider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	FGameplayTag GameplayTag;

	
	
public:	
	// Sets default values for this actor's properties
	AStealthDocument();

	UFUNCTION(BlueprintCallable)
	virtual void OnInteract_Implementation(APawn* InteractingPawn) override;

};
