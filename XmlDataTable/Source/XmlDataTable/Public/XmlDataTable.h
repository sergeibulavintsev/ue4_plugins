// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"

class IAssetTypeActions;

class FXmlDataTableModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterActions();
	void UnRegisterActions();

	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
};