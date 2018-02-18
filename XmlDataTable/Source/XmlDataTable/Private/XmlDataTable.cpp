// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "XmlDataTable.h"
#include "Modules/ModuleManager.h"
#include "CoreMinimal.h"
#include "AssetActions/AssetTypeActions_DataTableXml.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FXmlDataTableModule"

void FXmlDataTableModule::RegisterActions()
{
	auto& AssetTools = FAssetToolsModule::GetModule().Get();
	auto Action = MakeShareable(new FAssetTypeActions_DataTableExtended());
	AssetTools.RegisterAssetTypeActions(Action);
	RegisteredAssetTypeActions.Add(Action);
}

void FXmlDataTableModule::UnRegisterActions()
{
	FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");
	if (AssetToolsModule != nullptr)
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();
		for (auto Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action);
		}
	}
}

void FXmlDataTableModule::StartupModule()
{
	RegisterActions();
}

void FXmlDataTableModule::ShutdownModule()
{
	UnRegisterActions();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FXmlDataTableModule, XmlDataTable)