#pragma once

#include "CoreMinimal.h"
#include "XmlFile.h"

class UDataTable;
enum class EDataTableExportFlags : uint8;

#if WITH_EDITOR

class FDataTableExporterXml
{
public:

	FDataTableExporterXml(const EDataTableExportFlags InDTExportFlags);

	~FDataTableExporterXml();

	bool WriteTable(const UDataTable& InDataTable, const FString& FileName);

private:
	bool WriteRow(FXmlNode* RowNode, const UScriptStruct* InRowStruct, const void* InRowData);
	bool WriteStructEntry(FXmlNode* RowNode, const void* InRowData, const UProperty* InProperty, const void* InPropertyData);

	EDataTableExportFlags DTExportFlags;
	TSharedPtr<FXmlFile> XmlFile;
};

#endif // WITH_EDITOR
