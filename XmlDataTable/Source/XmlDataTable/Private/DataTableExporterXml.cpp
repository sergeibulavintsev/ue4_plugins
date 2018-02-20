// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "DataTableExporterXml.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"
#include "DataTableUtils.h"
#include "Engine/DataTable.h"
#include "Common.h"
#include "Engine/UserDefinedStruct.h"
#include "pugixml.hpp"

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

	pugi::xml_document XmlTable;
	pugi::xml_node XmlHeader = XmlTable.root().append_child(pugi::node_declaration);
	XmlHeader.append_attribute("version").set_value("1.0");
	XmlHeader.append_attribute("encoding").set_value("UTF-8");
	XmlHeader.append_attribute("standalone").set_value("yes");
	pugi::xml_node RootNode = XmlTable.append_child(XmlTags::Root);
	if (!RootNode)
	{
		return false;
	}
	RootNode.append_attribute("xmlns:xsi").set_value("http://www.w3.org/2001/XMLSchema-instance");
	// Iterate over rows
	for (auto RowIt = InDataTable.RowMap.CreateConstIterator(); RowIt; ++RowIt)
	{
		pugi::xml_node RowNode = RootNode.append_child(XmlTags::RowTag);
		pugi::xml_node NameNode = RowNode.append_child(XmlTags::RowName);
		NameNode.append_child(pugi::node_pcdata).set_value(TCHAR_TO_UTF8(*RowIt.Key().ToString()));

		// Now the values
		uint8* RowData = RowIt.Value();
		WriteRow(RowNode, InDataTable.RowStruct, RowData);
	}

	XmlTable.save_file(TCHAR_TO_UTF8(*FileName), "\t", 1U, pugi::xml_encoding::encoding_utf8);
	return true;
}

bool FDataTableExporterXml::WriteRow(pugi::xml_node& RowNode, const UScriptStruct* InRowStruct, const void* InRowData)
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

bool FDataTableExporterXml::WriteStructEntry(pugi::xml_node& RowNode, const void* InRowData, const UProperty* InProperty, const void* InPropertyData)
{
	const FString Identifier = DataTableUtils::GetPropertyExportName(InProperty, DTExportFlags);
	FString PropertyValue;
	const UNumericProperty *NumProp = Cast<UNumericProperty>(InProperty);
	if (NumProp && !NumProp->IsEnum() && !NumProp->IsInteger())
	{
		const double DoubleValue = NumProp->GetFloatingPointPropertyValue(InPropertyData);
		PropertyValue = FString::SanitizeFloat(DoubleValue);
		PropertyValue.RemoveFromEnd(".0");
	}
	else
	{
		PropertyValue = DataTableUtils::GetPropertyValueAsString(InProperty, (uint8*)InRowData, DTExportFlags);
	}
	RowNode.append_child(TCHAR_TO_UTF8(*Identifier)).append_child(pugi::node_pcdata).set_value(TCHAR_TO_UTF8(*PropertyValue));
	return true;
}
#endif // WITH_EDITOR
