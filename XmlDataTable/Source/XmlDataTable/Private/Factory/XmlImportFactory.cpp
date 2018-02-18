#include "XmlImportFactory.h"
#include "XmlImportOptions.h"
#include "DataTableImporterXml.h"
#include "Misc/MessageDialog.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "EditorFramework/AssetImportData.h"
#include "Curves/CurveLinearColor.h"
#include "Curves/CurveVector.h"
#include "Engine/CurveTable.h"
#include "Engine/DataTable.h"
#include "Editor.h"

#include "Interfaces/IMainFrameModule.h"
#include "Engine/DataTable.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogXmlImportFactory);

#define LOCTEXT_NAMESPACE "XmlImportFactory"

FXmlImportSettings::FXmlImportSettings()
{
	ImportRowStruct = nullptr;
	ImportType = EXmlImportType::EXml_DataTable;
}

UXmlImportFactory::UXmlImportFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = UDataTable::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("xml;xml data"));
}

bool UXmlImportFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	return Extension == TEXT("xml");
}

UObject* UXmlImportFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	UDataTable* ExistingTable = FindObject<UDataTable>(InParent, *InName.ToString());
	bool bHaveInfo = false;
	UScriptStruct* ImportRowStruct = NULL;
	EXmlImportType ImportType = EXmlImportType::EXml_DataTable;

	if (IsAutomatedImport())
	{
		ImportRowStruct = AutomatedImportSettings.ImportRowStruct;
		ImportType = AutomatedImportSettings.ImportType;
		bHaveInfo = ImportRowStruct != nullptr;
	}
	else if (ExistingTable != NULL)
	{
		ImportRowStruct = ExistingTable->RowStruct;
		bHaveInfo = true;
	}

	bool bDoImport = true;
	if (!bHaveInfo && !IsAutomatedImport())
	{
		TSharedPtr<SWindow> ParentWindow;
		// Check if the main frame is loaded.  When using the old main frame it may not be.
		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		TSharedPtr<SXmlImportOptions> ImportOptionsWindow;

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(LOCTEXT("DataTableOptionsWindowTitle", "DataTable Options"))
			.SizingRule(ESizingRule::Autosized);

		FString ParentFullPath;
		if (InParent)
		{
			ParentFullPath = InParent->GetPathName();
		}

		Window->SetContent
		(
			SAssignNew(ImportOptionsWindow, SXmlImportOptions)
			.WidgetWindow(Window)
			.FullPath(FText::FromString(ParentFullPath))
		);

		FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

		ImportType = ImportOptionsWindow->GetSelectedImportType();
		ImportRowStruct = ImportOptionsWindow->GetSelectedRowStruct();
		bDoImport = ImportOptionsWindow->ShouldImport();
	}
	else if (!bHaveInfo && IsAutomatedImport())
	{
		if (ImportType == EXmlImportType::EXml_DataTable && !ImportRowStruct)
		{
			UE_LOG(LogXmlImportFactory, Error, TEXT("A Data table row type must be specified in the import settings xml file for automated import"));
		}
		bDoImport = false;
	}

	UObject* NewAsset = NULL;
	if (bDoImport)
	{
		TArray<FString> Problems;
		if (ImportType == EXmlImportType::EXml_DataTable)
		{
			UClass* DataTableClass = UDataTable::StaticClass();
			if (ExistingTable != NULL)
			{
				DataTableClass = ExistingTable->GetClass();
				ExistingTable->EmptyTable();
			}

			UDataTable* NewTable = NewObject<UDataTable>(InParent, DataTableClass, InName, Flags);
			NewTable->RowStruct = ImportRowStruct;
			NewTable->AssetImportData->Update(Filename);
			Problems = DoImportDataTable(NewTable, Filename);
			UE_LOG(LogXmlImportFactory, Log, TEXT("Imported DataTable '%s' - %d Problems"), *InName.ToString(), Problems.Num());
			NewAsset = NewTable;
		}

		if (Problems.Num() > 0)
		{
			FString AllProblems;

			for (int32 ProbIdx = 0; ProbIdx<Problems.Num(); ProbIdx++)
			{
				// Output problems to log
				UE_LOG(LogXmlImportFactory, Log, TEXT("%d:%s"), ProbIdx, *Problems[ProbIdx]);
				AllProblems += Problems[ProbIdx];
				AllProblems += TEXT("\n");
			}
			if (!IsAutomatedImport())
			{
				// Pop up any problems for user
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(AllProblems));
			}
		}
	}

	FEditorDelegates::OnAssetPostImport.Broadcast(this, NewAsset);
	return NewAsset;
}

TArray<FString> UXmlImportFactory::DoImportDataTable(UDataTable* TargetDataTable, const FString& FileName)
{
	TArray<FString> Problems;
	FDataTableImporterXml().ReadTable(*TargetDataTable, FileName, Problems);
	return Problems;
}

#undef LOCTEXT_NAMESPACE