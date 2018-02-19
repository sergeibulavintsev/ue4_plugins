#pragma once

#include "CoreMinimal.h"

class UDataTable;
enum class EDataTableExportFlags : uint8;

namespace pugi
{
	class xml_node;
}

#if WITH_EDITOR

class FDataTableExporterXml
{
public:

	FDataTableExporterXml(const EDataTableExportFlags InDTExportFlags);

	~FDataTableExporterXml();

	bool WriteTable(const UDataTable& InDataTable, const FString& FileName);

private:
	bool WriteRow(pugi::xml_node& RowNode, const UScriptStruct* InRowStruct, const void* InRowData);
	bool WriteStructEntry(pugi::xml_node& RowNode, const void* InRowData, const UProperty* InProperty, const void* InPropertyData);

	EDataTableExportFlags DTExportFlags;
};

#endif // WITH_EDITOR
