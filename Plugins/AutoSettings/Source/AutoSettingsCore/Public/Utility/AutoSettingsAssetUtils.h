// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FPrimaryAssetType;

/**
 * Utility functions for loading and managing AutoSettings assets.
 * 
 * Provides helper functions for bulk asset loading operations using the Asset Manager system.
 */
class AUTOSETTINGSCORE_API FAutoSettingsAssetUtils
{
public:

	/** Load all assets of a specific type using the Asset Manager */
	static TArray<UObject*> LoadAndGetAllAssets(const FPrimaryAssetType& AssetType, bool bGetClassDefaults = true);

	/** Template version that automatically casts loaded assets to the specified type */
	template<typename T>
    static TArray<T*> LoadAndGetAllAssets(const FPrimaryAssetType& AssetType, bool bGetClassDefaults = true)
	{
		TArray<UObject*> Assets = LoadAndGetAllAssets(AssetType, bGetClassDefaults);

		TArray<T*> AssetObjects;
		for(UObject* Asset : Assets)
		{
			T* TypedAssetObject = Cast<T>(Asset);
			if(!ensure(TypedAssetObject))
			{
				continue;
			}
			AssetObjects.Add(TypedAssetObject);
		}

		return AssetObjects;
	}

};
