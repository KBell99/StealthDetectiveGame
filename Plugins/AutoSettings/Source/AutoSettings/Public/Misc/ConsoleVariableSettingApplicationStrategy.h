// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingApplicationStrategy.h"

#include "ConsoleVariableSettingApplicationStrategy.generated.h"

/**
 * Application strategy that applies setting values to Unreal Engine console variables.
 */
UCLASS()
class AUTOSETTINGS_API UConsoleVariableSettingApplicationStrategy : public USettingApplicationStrategy
{
	GENERATED_BODY()

public:
	// Console variable to use for this settings
	// If none, the Setting Key is used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	FName ConsoleVariableName = NAME_None;

	// Transformation to apply to the value to get the subsection that this setting cares about
	// Leave this empty for no transformation
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Setting")
	class USettingValueMask* ValueMask;

	UConsoleVariableSettingApplicationStrategy();

	virtual void OnApply_Implementation(const FSetSettingEvent& Event) override;
	virtual FString GetAppliedValue_Implementation(const FSettingReference& Setting) override;
	virtual bool ValidateConfiguration_Implementation(const FSettingReference& Setting) override;
	virtual void RegisterAppliedValueModifiedCallback_Implementation(const FSettingReference& Setting,
		const FSettingAppliedValueModifiedSignature& Callback) override;
	virtual void RemoveAppliedValueModifiedCallback_Implementation(const UObject* UserObject,
		const FSettingReference& Setting) override;

	FName GetConsoleVariableName(const FFullSettingKey& Key) const;
};