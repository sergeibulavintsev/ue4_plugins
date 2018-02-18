#include "DataTableImporterXml.h"
#include "Engine/DataTable.h"
#include "Engine/UserDefinedStruct.h"

namespace
{
	int GetDataTableColumnsNum(UDataTable& InDataTable)
	{
		int Num = 0;
		for (TFieldIterator<UProperty> It(InDataTable.RowStruct); It; ++It)
		{
			check(*It != NULL);
			Num++;
		}
		return Num;
	}
}

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

	FXmlFile XmlFile(InFileName);
	FXmlNode* RootNode = XmlFile.GetRootNode();
	if (RootNode == nullptr && !XmlFile.GetLastError().IsEmpty())
	{
		OutProblems.Add(XmlFile.GetLastError());
		return false;
	}

	if (RootNode->GetChildrenNodes().Num() == 0)
	{
		OutProblems.Add(TEXT("Too few rows."));
		return false;
	}
	int ColumnsNum = GetDataTableColumnsNum(InDataTable);
	for (const FXmlNode* RowNode = RootNode->GetFirstChildNode(); RowNode != nullptr; RowNode = RowNode->GetNextNode())
	{
		// Get row name
		FName RowName = DataTableUtils::MakeValidName(RowNode->GetTag());

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

		int PropNums = RowNode->GetChildrenNodes().Num();
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

		// Now iterate over cells (skipping first cell, that was row name)
		for (const FXmlNode* PropertyNode = RowNode->GetFirstChildNode(); PropertyNode != nullptr; PropertyNode = PropertyNode->GetNextNode())
		{
			FName PropertyName = DataTableUtils::MakeValidName(PropertyNode->GetTag());
			if (PropertyName == NAME_None)
			{
				OutProblems.Add(FString::Printf(TEXT("PropertyName missing a name for row name %s."), *RowName.ToString()));
				continue;
			}
			// Try and assign string to data using the column property
			UProperty* Property = InDataTable.FindTableProperty(PropertyName);
			if (Property == nullptr)
			{
				OutProblems.Add(FString::Printf(TEXT("PropertyName missing a name for row name %s."), *RowName.ToString()));
				continue;
			}
			const FString PropValue = PropertyNode->GetContent();
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