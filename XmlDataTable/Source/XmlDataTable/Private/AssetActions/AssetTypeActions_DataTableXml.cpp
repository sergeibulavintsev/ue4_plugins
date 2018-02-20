#include "AssetTypeActions_DataTableXml.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/FileHelper.h"
#include "EditorFramework/AssetImportData.h"
#include "Dialogs/Dialogs.h"
#include "Framework/Application/SlateApplication.h"
#include "AssetToolsModule.h"
#include "Developer/DesktopPlatform//Public/DesktopPlatformModule.h"
#include "DataTableExporterXml.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FAssetTypeActions_DataTableExtended::FAssetTypeActions_DataTableExtended()
{
	auto& AssetTools = FAssetToolsModule::GetModule().Get();
	DataTableActions = AssetTools.GetAssetTypeActionsForClass(UDataTable::StaticClass()).Pin();
}

UClass* FAssetTypeActions_DataTableExtended::GetSupportedClass() const
{
	return DataTableActions->GetSupportedClass();
}

FColor FAssetTypeActions_DataTableExtended::GetTypeColor() const
{
	return DataTableActions->GetTypeColor();
}

bool FAssetTypeActions_DataTableExtended::HasActions(const TArray<UObject*>& InObjects) const
{
	return DataTableActions->HasActions(InObjects);
}

uint32 FAssetTypeActions_DataTableExtended::GetCategories()
{
	return DataTableActions->GetCategories();
}

bool FAssetTypeActions_DataTableExtended::IsImportedAsset() const
{
	return DataTableActions->IsImportedAsset();
}

void FAssetTypeActions_DataTableExtended::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	DataTableActions->GetActions(InObjects, MenuBuilder);
	auto Tables = GetTypedWeakObjectPtrs<UObject>(InObjects);

	TArray<FString> ImportPaths;
	for (auto TableIter = Tables.CreateConstIterator(); TableIter; ++TableIter)
	{
		const UDataTable* CurTable = Cast<UDataTable>((*TableIter).Get());
		if (CurTable)
		{
			CurTable->AssetImportData->ExtractFilenames(ImportPaths);
		}
	}

	MenuBuilder.AddMenuEntry(
		LOCTEXT("DataTable_ExportAsXml", "Export as XML"),
		LOCTEXT("DataTable_ExportAsXmlTooltip", "Export the data table as a file containing XML data."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_DataTableExtended::ExecuteExportAsXml, Tables),
			FCanExecuteAction()
		)
	);

	TArray<FString> PotentialFileExtensions;
	PotentialFileExtensions.Add(TEXT(".xml"));
	MenuBuilder.AddMenuEntry(
		LOCTEXT("DataTable_OpenXmlSourceData", "Open Source Data (XML)"),
		LOCTEXT("DataTable_OpenXmlSourceDataTooltip", "Opens the data table's source data file in an external editor. It will search using the following extensions: .xml"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_DataTableExtended::ExecuteFindSourceFileInExplorer, ImportPaths, PotentialFileExtensions),
			FCanExecuteAction::CreateSP(this, &FAssetTypeActions_DataTableExtended::CanExecuteFindSourceFileInExplorer, ImportPaths, PotentialFileExtensions)
		)
	);
}

void FAssetTypeActions_DataTableExtended::ExecuteExportAsXml(TArray< TWeakObjectPtr<UObject> > Objects)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto DataTable = Cast<UDataTable>((*ObjIt).Get());
		if (DataTable)
		{
			const FText Title = FText::Format(LOCTEXT("DataTable_ExportXmlDialogTitle", "Export '{0}' as Xml..."), FText::FromString(*DataTable->GetName()));
			const FString CurrentFilename = DataTable->AssetImportData->GetFirstFilename();
			const FString FileTypes = TEXT("Data Table Xml (*.xml)|*.xml");

			TArray<FString> OutFilenames;
			DesktopPlatform->SaveFileDialog(
				ParentWindowWindowHandle,
				Title.ToString(),
				(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetPath(CurrentFilename),
				(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetBaseFilename(CurrentFilename) + TEXT(".xml"),
				FileTypes,
				EFileDialogFlags::None,
				OutFilenames
			);

			if (OutFilenames.Num() > 0)
			{
				FDataTableExporterXml(EDataTableExportFlags::UsePrettyPropertyNames).WriteTable(*DataTable, *OutFilenames[0]);
			}
		}
	}
}

void FAssetTypeActions_DataTableExtended::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	DataTableActions->OpenAssetEditor(InObjects, EditWithinLevelEditor);
}

void FAssetTypeActions_DataTableExtended::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	DataTableActions->GetResolvedSourceFilePaths(TypeAssets, OutSourceFilePaths);
}

void FAssetTypeActions_DataTableExtended::PerformAssetDiff(UObject* OldAsset, UObject* NewAsset, const FRevisionInfo& OldRevision, const FRevisionInfo& NewRevision) const
{
	DataTableActions->PerformAssetDiff(OldAsset, NewAsset, OldRevision, NewRevision);
}

void FAssetTypeActions_DataTableExtended::ExecuteFindSourceFileInExplorer(TArray<FString> Filenames, TArray<FString> OverrideExtensions)
{
	for (TArray<FString>::TConstIterator FilenameIter(Filenames); FilenameIter; ++FilenameIter)
	{
		const FString XmlFilename = FPaths::ConvertRelativePathToFull(*FilenameIter);
		const FString RootPath = FPaths::GetPath(XmlFilename);
		const FString BaseFilename = FPaths::GetBaseFilename(XmlFilename, true);

		for (TArray<FString>::TConstIterator ExtensionItr(OverrideExtensions); ExtensionItr; ++ExtensionItr)
		{
			const FString FilenameWithExtension(FString::Printf(TEXT("%s/%s%s"), *RootPath, *BaseFilename, **ExtensionItr));
			if (VerifyFileExists(FilenameWithExtension))
			{
				FPlatformProcess::LaunchFileInDefaultExternalApplication(*FilenameWithExtension, NULL, ELaunchVerb::Edit);
				break;
			}
		}
	}
}

bool FAssetTypeActions_DataTableExtended::CanExecuteFindSourceFileInExplorer(TArray<FString> Filenames, TArray<FString> OverrideExtensions) const
{
	// Verify that extensions were provided
	if (OverrideExtensions.Num() == 0)
	{
		return false;
	}

	// Verify that the file exists with any of the given extensions
	for (TArray<FString>::TConstIterator FilenameIter(Filenames); FilenameIter; ++FilenameIter)
	{
		const FString XmlFilename = FPaths::ConvertRelativePathToFull(*FilenameIter);
		const FString RootPath = FPaths::GetPath(XmlFilename);
		const FString BaseFilename = FPaths::GetBaseFilename(XmlFilename, true);

		for (TArray<FString>::TConstIterator ExtensionItr(OverrideExtensions); ExtensionItr; ++ExtensionItr)
		{
			const FString FilenameWithExtension(FString::Printf(TEXT("%s/%s%s"), *RootPath, *BaseFilename, **ExtensionItr));
			if (VerifyFileExists(FilenameWithExtension))
			{
				return true;
			}
		}
	}

	return false;
}

bool FAssetTypeActions_DataTableExtended::VerifyFileExists(const FString& InFileName) const
{
	return (!InFileName.IsEmpty() && IFileManager::Get().FileSize(*InFileName) != INDEX_NONE);
}

#undef LOCTEXT_NAMESPACE