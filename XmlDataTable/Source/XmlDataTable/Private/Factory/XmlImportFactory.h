#pragma once
#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "XmlImportFactory.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogXmlImportFactory, Log, All);

UENUM()
enum class EXmlImportType : uint8
{
	/** Import as UDataTable */
	EXml_DataTable,
};

USTRUCT()
struct FXmlImportSettings
{
	GENERATED_BODY()

	FXmlImportSettings();

	UPROPERTY()
	UScriptStruct* ImportRowStruct;

	UPROPERTY()
	EXmlImportType ImportType;
};

UCLASS(hidecategories = Object)
class UXmlImportFactory	: public UFactory
{
	GENERATED_UCLASS_BODY()

public:
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool FactoryCanImport(const FString& Filename) override;
private:
	TArray<FString> DoImportDataTable(UDataTable* TargetDataTable, const FString& FileName);

	UPROPERTY()
	FXmlImportSettings AutomatedImportSettings;
};