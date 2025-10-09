// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/StealthCharacterBase.h"

// Sets default values
AStealthCharacterBase::AStealthCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AStealthCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStealthCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AStealthCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

