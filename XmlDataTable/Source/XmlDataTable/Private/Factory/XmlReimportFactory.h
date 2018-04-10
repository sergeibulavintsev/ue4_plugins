#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "XmlImportFactory.h"
#include "EditorReimportHandler.h"
#include "XmlReimportFactory.generated.h"

UCLASS()
class UXmlReimportFactory : public UXmlImportFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

		//~ Begin FReimportHandler Interface
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	//~ End FReimportHandler Interface
};