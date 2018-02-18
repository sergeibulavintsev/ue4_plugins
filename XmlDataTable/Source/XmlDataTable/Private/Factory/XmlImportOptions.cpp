#include "XmlImportOptions.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"
#include "UObject/Package.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "Engine/UserDefinedStruct.h"

#define LOCTEXT_NAMESPACE "XmlImportFactory"

void SXmlImportOptions::Construct(const FArguments& InArgs)
{
	WidgetWindow = InArgs._WidgetWindow;

	// Make array of enum pointers
	TSharedPtr<EXmlImportType> DataTableTypePtr = MakeShareable(new EXmlImportType(EXmlImportType::EXml_DataTable));
	ImportTypes.Add(DataTableTypePtr);

	// Find table row struct info
	UScriptStruct* TableRowStruct = FindObjectChecked<UScriptStruct>(ANY_PACKAGE, TEXT("TableRowBase"));
	if (TableRowStruct != NULL)
	{
		// Make combo of table rowstruct options
		for (TObjectIterator<UScriptStruct> It; It; ++It)
		{
			UScriptStruct* Struct = *It;
			// If a child of the table row struct base, but not itself
			const bool bBasedOnTableRowBase = Struct->IsChildOf(TableRowStruct) && (Struct != TableRowStruct);
			const bool bUDStruct = Struct->IsA<UUserDefinedStruct>();
			const bool bValidStruct = (Struct->GetOutermost() != GetTransientPackage());
			if ((bBasedOnTableRowBase || bUDStruct) && bValidStruct)
			{
				RowStructs.Add(Struct);
			}
		}

		// Alphabetically sort the row structs by name
		RowStructs.Sort([](const UScriptStruct& ElementA, const UScriptStruct& ElementB) { return (ElementA.GetName() < ElementB.GetName()); });
	}

	// Create widget
	this->ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush(TEXT("Menu.Background")))
		.Padding(10)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.Padding(FMargin(3))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		.Visibility(InArgs._FullPath.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Font(FEditorStyle::GetFontStyle("CurveEd.LabelFont"))
		.Text(LOCTEXT("Import_CurrentFileTitle", "Current File: "))
		]
	+ SHorizontalBox::Slot()
		.Padding(5, 0, 0, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(FEditorStyle::GetFontStyle("CurveEd.InfoFont"))
		.Text(InArgs._FullPath)
		]
		]
		]

	// Import type
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ChooseAssetType", "Import As:"))
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ImportTypeCombo, SComboBox< TSharedPtr<EXmlImportType> >)
			.OptionsSource(&ImportTypes)
		.OnGenerateWidget(this, &SXmlImportOptions::MakeImportTypeItemWidget)
		[
			SNew(STextBlock)
			.Text(this, &SXmlImportOptions::GetSelectedItemText)
		]
		]
	// Data row struct
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ChooseRowType", "Choose DataTable Row Type:"))
		.Visibility(this, &SXmlImportOptions::GetTableRowOptionVis)
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(RowStructCombo, SComboBox<UScriptStruct*>)
			.OptionsSource(&RowStructs)
		.OnGenerateWidget(this, &SXmlImportOptions::MakeRowStructItemWidget)
		.Visibility(this, &SXmlImportOptions::GetTableRowOptionVis)
		[
			SNew(STextBlock)
			.Text(this, &SXmlImportOptions::GetSelectedRowOptionText)
		]
		]
	// Ok/Cancel
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("OK", "OK"))
		.OnClicked(this, &SXmlImportOptions::OnImport)
		]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("Cancel", "Cancel"))
		.OnClicked(this, &SXmlImportOptions::OnCancel)
		]
		]
		]
		];

	// set-up selection
	ImportTypeCombo->SetSelectedItem(DataTableTypePtr);
}

/** If we should import */
bool SXmlImportOptions::ShouldImport()
{
	return ((SelectedStruct != NULL) || GetSelectedImportType() != EXmlImportType::EXml_DataTable) && bImport;
}

/** Get the row struct we selected */
UScriptStruct* SXmlImportOptions::GetSelectedRowStruct()
{
	return SelectedStruct;
}

/** Get the import type we selected */
EXmlImportType SXmlImportOptions::GetSelectedImportType()
{
	return SelectedImportType;
}


/** Whether to show table row options */
EVisibility SXmlImportOptions::GetTableRowOptionVis() const
{
	return (ImportTypeCombo.IsValid() && *ImportTypeCombo->GetSelectedItem() == EXmlImportType::EXml_DataTable) ? EVisibility::Visible : EVisibility::Collapsed;
}

FString SXmlImportOptions::GetImportTypeText(TSharedPtr<EXmlImportType> Type) const
{
	FString EnumString;
	if (*Type == EXmlImportType::EXml_DataTable)
	{
		EnumString = TEXT("DataTable");
	}
	return EnumString;
}

/** Called to create a widget for each struct */
TSharedRef<SWidget> SXmlImportOptions::MakeImportTypeItemWidget(TSharedPtr<EXmlImportType> Type)
{
	return	SNew(STextBlock)
		.Text(FText::FromString(GetImportTypeText(Type)));
}

/** Called to create a widget for each struct */
TSharedRef<SWidget> SXmlImportOptions::MakeRowStructItemWidget(UScriptStruct* Struct)
{
	check(Struct != NULL);
	return	SNew(STextBlock)
		.Text(FText::FromString(Struct->GetName()));
}

/** Called when 'OK' button is pressed */
FReply SXmlImportOptions::OnImport()
{
	SelectedStruct = RowStructCombo->GetSelectedItem();
	SelectedImportType = *ImportTypeCombo->GetSelectedItem();
	bImport = true;
	if (WidgetWindow.IsValid())
	{
		WidgetWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

/** Called when 'Cancel' button is pressed */
FReply SXmlImportOptions::OnCancel()
{
	bImport = false;
	if (WidgetWindow.IsValid())
	{
		WidgetWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FText SXmlImportOptions::GetSelectedItemText() const
{
	TSharedPtr<EXmlImportType> SelectedType = ImportTypeCombo->GetSelectedItem();

	return (SelectedType.IsValid())
		? FText::FromString(GetImportTypeText(SelectedType))
		: FText::GetEmpty();
}

FText SXmlImportOptions::GetSelectedRowOptionText() const
{
	UScriptStruct* SelectedScript = RowStructCombo->GetSelectedItem();
	return (SelectedScript)
		? FText::FromString(SelectedScript->GetName())
		: FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE
