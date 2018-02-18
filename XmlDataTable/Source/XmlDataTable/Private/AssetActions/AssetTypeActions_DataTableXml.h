#pragma once
#include "Engine/DataTable.h"
#include "AssetTypeActions_Base.h"
#include "Templates/SharedPointer.h"

class FMenuBuilder;

class FAssetTypeActions_DataTableExtended
	: public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_DataTableExtended();
public:
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_DataTableExtended", "Data Table"); }
	virtual UClass* GetSupportedClass() const override;
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual void GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const override;
	virtual void PerformAssetDiff(UObject* Asset1, UObject* Asset2, const struct FRevisionInfo& OldRevision, const struct FRevisionInfo& NewRevision) const override;
	virtual FColor GetTypeColor() const override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
	virtual uint32 GetCategories() override;
	virtual bool IsImportedAsset() const override;


private:
	void ExecuteExportAsXml(TArray< TWeakObjectPtr<UObject> > Objects);
	bool VerifyFileExists(const FString& InFileName) const;
	bool CanExecuteFindSourceFileInExplorer(TArray<FString> Filenames, TArray<FString> OverrideExtensions) const;
	void ExecuteFindSourceFileInExplorer(TArray<FString> Filenames, TArray<FString> OverrideExtensions);

	TSharedPtr<IAssetTypeActions> DataTableActions;
};