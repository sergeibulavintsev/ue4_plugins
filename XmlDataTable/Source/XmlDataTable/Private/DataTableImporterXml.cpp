#include "DataTableImporterXml.h"
#include "Engine/DataTable.h"
#include "Engine/UserDefinedStruct.h"
#include "Common.h"
#include "pugixml.hpp"
#include <algorithm>


bool FDataTableImporterXml::ReadTable(UDataTable& InDataTable, FString InFileName, TArray<FString>& OutProblems)
{
	if (InFileName.IsEmpty())
	{
		OutProblems.Add(TEXT("Filename is empty."));
		return false;
	}

	if (!InDataTable.RowStruct)
	{
		OutProblems.Add(TEXT("No RowStruct specified."));
		return false;
	}

	pugi::xml_document XmlTable;
	auto result = XmlTable.load_file(TCHAR_TO_UTF8(*InFileName), pugi::parse_default, pugi::xml_encoding::encoding_utf8);
	if (result.status != pugi::xml_parse_status::status_ok)
	{
		OutProblems.Add(TEXT("Corrupted xml file."));
		return false;
	}
	
	pugi::xml_node Root = XmlTable.child(XmlTags::Root);
	if (!Root)
	{
		OutProblems.Add(TEXT("XML file is empty."));
		return false;
	}

	auto RowNodes = Root.children();
	int RowNum = std::distance(RowNodes.begin(), RowNodes.end());
	if (RowNum == 0)
	{
		OutProblems.Add(TEXT("Too few rows."));
		return false;
	}
	int ColumnsNum = InDataTable.GetColumnTitles().Num();
	for (auto RowNode = RowNodes.begin(); RowNode != RowNodes.end(); RowNode++)
	{
		FName RowName = DataTableUtils::MakeValidName(FString(UTF8_TO_TCHAR(RowNode->child(XmlTags::RowName).text().as_string())));
		// Check its not 'none'
		if (RowName == NAME_None)
		{
			OutProblems.Add(FString::Printf(TEXT("Row missing a name.")));
			continue;
		}

		// Check its not a duplicate
		if (InDataTable.RowMap.Find(RowName) != NULL)
		{
			OutProblems.Add(FString::Printf(TEXT("Duplicate row name '%s'."), *RowName.ToString()));
			continue;
		}

		auto ColumnNodes = RowNode->children();
		int PropNums = std::distance(RowNode->begin(), RowNode->end());
		if (PropNums < ColumnsNum)
		{
			OutProblems.Add(FString::Printf(TEXT("Row '%s' has more cells than properties, is there a malformed string?"), *RowName.ToString()));
			continue;
		}

		// Allocate data to store information, using UScriptStruct to know its size
		uint8* RowData = (uint8*)FMemory::Malloc(InDataTable.RowStruct->GetStructureSize());
		InDataTable.RowStruct->InitializeStruct(RowData);
		// And be sure to call DestroyScriptStruct later

		if (auto UDStruct = Cast<const UUserDefinedStruct>(InDataTable.RowStruct))
		{
			UDStruct->InitializeDefaultValue(RowData);
		}

		// Add to row map
		InDataTable.RowMap.Add(RowName, RowData);

		for (TFieldIterator<UProperty> It(InDataTable.RowStruct); It; ++It)
		{
			UProperty* Property = *It;
			check(Property);

			const FString ColumnName = DataTableUtils::GetPropertyDisplayName(Property, Property->GetName());
			pugi::xml_node ColumnNode = RowNode->child(TCHAR_TO_UTF8(*ColumnName));
			if (!ColumnNode)
			{
				OutProblems.Add(FString::Printf(TEXT("Column %s is missing a in row %s."), *ColumnName, *RowName.ToString()));
				continue;
			}

			const FString PropValue(UTF8_TO_TCHAR(ColumnNode.text().as_string()));
			FString Error = DataTableUtils::AssignStringToProperty(PropValue, Property, RowData);

			// If we failed, output a problem string
			if (Error.Len() > 0)
			{
				FString PropertyName = (Property != NULL)
					? DataTableUtils::GetPropertyDisplayName(Property, Property->GetName())
					: FString(TEXT("NONE"));
				OutProblems.Add(FString::Printf(TEXT("Problem assigning string '%s' to property '%s' on row '%s' : %s"), *PropValue, *PropertyName, *RowName.ToString(), *Error));
			}
		}

		// Problem if we didn't have enough cells on this row
		if (ColumnsNum < PropNums)
		{
			OutProblems.Add(FString::Printf(TEXT("Too few cells on row '%s'."), *RowName.ToString()));
		}
	}

	InDataTable.Modify(true);
	return true;
}