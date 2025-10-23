// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Processing/AutoSettingsInputCapturePreProcessor.h"
#include "Widgets/AutoSettingWidget.h"
#include "MappingIssues/MappingIssueTypes.h"
#include "InputMappingSettingWidget.generated.h"

class UInputKeyFilter;
struct FMappingAttemptInfo;
class UInputMappingContext;
class UActionLabel;
enum class EBindingCaptureMode : uint8;

/**
 * Widget that can be used to display an input mapping and allow the player to rebind it
 */
UCLASS(abstract)
class AUTOSETTINGSINPUT_API UInputMappingSettingWidget : public UAutoSettingWidget
{
	GENERATED_BODY()

public:
	UInputMappingSettingWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Input Mapping Selector")
	UInputKeyFilter* KeyFilter;

	// Should keys and mouse clicks be captured when they are pressed, or when they are released?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Mapping Selector")
	EBindingCaptureMode CaptureMode;

	// Fired when an input chord is captured using this button
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChordCapturedEvent, FCapturedInput, CapturedInput);

	UPROPERTY(BlueprintAssignable, Category = "Input Mapping Selector")
	FChordCapturedEvent OnChordCaptured;

	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Input Mapping Selector")
	void SetMapping(FName MappingName, EPlayerMappableKeySlot KeySlot);

	void UpdateSettingKey();

	// Create prompt and start listening for input chord
	// Returns the newly created prompt
	UFUNCTION(BlueprintCallable, Category = "Input Mapping Selector")
	void StartCapture();

	// Stop listening
	UFUNCTION(BlueprintCallable, Category = "Input Mapping Selector")
	void StopCapture();

	UFUNCTION(BlueprintPure, Category = "Input Mapping Selector")
	bool IsCapturing() const;

protected:
	// Name of the action mapping to display and rebind
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Mapping Selector")
	FName MappingName;

	// Which slot of the action mapping to display and rebind
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Mapping Selector")
	EPlayerMappableKeySlot KeySlot;

	virtual void UpdateSelection_Implementation(const FString& Value) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Input Mapping Selector")
	void OnKeyChanged(const FKey& Key);

	// Returns true if the key is allowed to be selected with this selector
	UFUNCTION(BlueprintNativeEvent, Category = "Input Mapping Selector")
	bool IsKeyAllowed(const FKey& PrimaryKey);

	UFUNCTION(BlueprintNativeEvent, Category = "Input Mapping Selector")
	void OnEncounteredMappingIssue(const FMappingAttemptInfo& Issues);

	UFUNCTION(BlueprintCallable, Category = "Input Mapping Selector")
	void SetMappingIssueResolution(EMappingIssueResolution Resolution);

	virtual void PostLoad() override;

	// #if WITH_EDITOR
	// 	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// #endif

	virtual TFuture<EMappingIssueResolution> DetermineResolutionAsync(const FMappingAttemptInfo& Issues);

private:
	TSharedPtr<class FAutoSettingsInputCapturePreProcessor> InputProcessor;

	TSharedPtr<TPromise<EMappingIssueResolution>> InProgressIssueResolutionPromise;

	void StoreMappingDirectionalIntent(const FCapturedInput& CapturedInput);
	void BindKey(const FCapturedInput& CapturedInput);
};