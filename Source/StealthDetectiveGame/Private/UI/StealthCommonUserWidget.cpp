// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StealthCommonUserWidget.h"

#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Engine/InputDelegateBinding.h"
#include "Extensions/UserWidgetExtension.h"
#include "Extensions/WidgetBlueprintGeneratedClassExtension.h"

void UStealthCommonUserWidget::NativeOnInitialized()
{
	bAutomaticallyRegisterInputOnConstruction = true;

	if (APlayerController* Controller = GetOwningPlayer())
	{
		UInputDelegateBinding::BindInputDelegates(GetClass(), Controller->InputComponent, this);
	}

	if (UWidgetBlueprintGeneratedClass* BPClass = Cast<UWidgetBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->ForEachExtension(
			[this] (UWidgetBlueprintGeneratedClassExtension* Extension)
			{
				Extension->Initialize(this);
			}
		);
	}

	for (UUserWidgetExtension* Extension : GetExtensions(UUserWidgetExtension::StaticClass()))
	{
		Extension->Initialize();
	}

	OnInitialized();
}

void UStealthCommonUserWidget::PostInitProperties()
{
	Super::PostInitProperties();
	bAutomaticallyRegisterInputOnConstruction = true;
}

UStealthCommonUserWidget::UStealthCommonUserWidget(){
	bAutomaticallyRegisterInputOnConstruction = true;
}


