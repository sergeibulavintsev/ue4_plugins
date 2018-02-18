// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "DataTableExporterXml.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"
#include "DataTableUtils.h"
#include "Engine/DataTable.h"

#include "Engine/UserDefinedStruct.h"

#if WITH_EDITOR


FDataTableExporterXml::FDataTableExporterXml(const EDataTableExportFlags InDTExportFlags)
	: DTExportFlags(InDTExportFlags)
{
}

FDataTableExporterXml::~FDataTableExporterXml()
{
}

bool FDataTableExporterXml::WriteTable(const UDataTable& InDataTable, const FString& FileName)
{
	if (!InDataTable.RowStruct)
	{
		return false;
	}

	XmlFile = TSharedPtr<FXmlFile>(new FXmlFile(TEXT("<?xml version=\"1.0\" encoding=\"UTF - 8\"?>\n<root>\n</root>"), EConstructMethod::ConstructFromBuffer));

	FXmlNode* RootNode = XmlFile->GetRootNode();
	if (RootNode == nullptr)
	{
		return false;
	}
	// Iterate over rows
	for (auto RowIt = InDataTable.RowMap.CreateConstIterator(); RowIt; ++RowIt)
	{
		{
			// RowName
			const FName RowName = RowIt.Key();
			RootNode->AppendChildNode(RowName.ToString(), TEXT(""));
			FXmlNode* RowNode = RootNode->FindChildNode(RowName.ToString());

			// Now the values
			uint8* RowData = RowIt.Value();
			WriteRow(RowNode, InDataTable.RowStruct, RowData);
		}
	}

	XmlFile->Save(FileName);
	return true;
}

bool FDataTableExporterXml::WriteRow(FXmlNode* RowNode, const UScriptStruct* InRowStruct, const void* InRowData)
{
	if (!InRowStruct)
	{
		return false;
	}

	for (TFieldIterator<UProperty> It(InRowStruct); It; ++It)
	{
		UProperty* BaseProp = *It;
		check(BaseProp);

		const void* Data = BaseProp->ContainerPtrToValuePtr<void>(InRowData, 0);
		WriteStructEntry(RowNode, InRowData, BaseProp, Data);
	}

	return true;
}

bool FDataTableExporterXml::WriteStructEntry(FXmlNode* RowNode, const void* InRowData, const UProperty* InProperty, const void* InPropertyData)
{
	const FString Identifier = DataTableUtils::GetPropertyExportName(InProperty, DTExportFlags);
	const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(InProperty, (uint8*)InRowData, DTExportFlags);
	RowNode->AppendChildNode(Identifier, PropertyValue);
	return true;
}
#endif // WITH_EDITOR
