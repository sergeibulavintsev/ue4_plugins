#pragma once
#include "CoreMinimal.h"
#include "XmlFile.h"

class UDataTable;

class FDataTableImporterXml
{
public:
	bool ReadTable(UDataTable& InDataTable, FString InFileName, TArray<FString>& OutProblems);
};
