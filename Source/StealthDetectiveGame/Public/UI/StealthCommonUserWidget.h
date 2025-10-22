// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "StealthCommonUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class STEALTHDETECTIVEGAME_API UStealthCommonUserWidget : public UCommonUserWidget
{
	GENERATED_BODY()

	virtual void  NativeOnInitialized() override;
	virtual void PostInitProperties() override;

public:
	UStealthCommonUserWidget();
};
