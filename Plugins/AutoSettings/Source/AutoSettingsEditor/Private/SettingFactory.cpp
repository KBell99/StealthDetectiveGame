// Copyright Sam Bonifacio. All Rights Reserved.

#include "SettingFactory.h"
#include "AssetToolsModule.h"
#include "Framework/SettingType.h"

USettingFactory::USettingFactory()
{
	SupportedClass = USettingType::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = false;
}

UObject* USettingFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<USettingType>(InParent, Class, Name, Flags);
}

FText USettingFactory::GetDisplayName() const
{
	return FText::FromString(FName::NameToDisplayString(*USettingType::StaticClass()->GetName(), false));
}