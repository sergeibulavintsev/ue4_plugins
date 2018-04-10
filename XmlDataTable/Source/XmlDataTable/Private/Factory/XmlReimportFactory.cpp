#include "XmlReimportFactory.h"
#include "DataTableEditorUtils.h"
#include "Engine/DataTable.h"

#define LOCTEXT_NAMESPACE "XmlReimportFactory"

UXmlReimportFactory::UXmlReimportFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Formats.Add(TEXT("xml;xml data"));
}

bool UXmlReimportFactory::FactoryCanImport(const FString& Filename)
{
	return true;
}

bool UXmlReimportFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UDataTable* DataTable = Cast<UDataTable>(Obj);
	if (DataTable)
	{
		DataTable->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UXmlReimportFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UDataTable* DataTable = Cast<UDataTable>(Obj);
	if (DataTable && ensure(NewReimportPaths.Num() == 1))
	{
		DataTable->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UXmlReimportFactory::Reimport(UObject* Obj)
{
	auto Result = EReimportResult::Failed;
	if (auto DataTable = Cast<UDataTable>(Obj))
	{
		FDataTableEditorUtils::BroadcastPreChange(DataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowList);
		Result = UXmlImportFactory::ReimportDataTableFromXml(DataTable) ? EReimportResult::Succeeded : EReimportResult::Failed;
		FDataTableEditorUtils::BroadcastPostChange(DataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowList);
	}
	return Result;
}

int32 UXmlReimportFactory::GetPriority() const
{
	return ImportPriority;
}

#undef LOCTEXT_NAMESPACE