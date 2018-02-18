#pragma once

#include "CoreMinimal.h"
#include "XmlImportFactory.h"
#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"

class SXmlImportOptions : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SXmlImportOptions)
		: _WidgetWindow()
		, _FullPath()
	{}

	SLATE_ARGUMENT(TSharedPtr<SWindow>, WidgetWindow)
		SLATE_ARGUMENT(FText, FullPath)
		SLATE_END_ARGS()

		SXmlImportOptions()
		: bImport(false)
		, SelectedImportType(EXmlImportType::EXml_DataTable)
		, SelectedStruct(NULL)
	{}

	void Construct(const FArguments& InArgs);

	/** If we should import */
	bool ShouldImport();

	/** Get the row struct we selected */
	UScriptStruct* GetSelectedRowStruct();

	/** Get the import type we selected */
	EXmlImportType GetSelectedImportType();

	/** Whether to show table row options */
	EVisibility GetTableRowOptionVis() const;

	FString GetImportTypeText(TSharedPtr<EXmlImportType> Type) const;

	/** Called to create a widget for each struct */
	TSharedRef<SWidget> MakeImportTypeItemWidget(TSharedPtr<EXmlImportType> Type);

	/** Called to create a widget for each struct */
	TSharedRef<SWidget> MakeRowStructItemWidget(UScriptStruct* Struct);

	/** Called when 'OK' button is pressed */
	FReply OnImport();

	/** Called when 'Cancel' button is pressed */
	FReply OnCancel();

	FText GetSelectedItemText() const;

	FText GetSelectedRowOptionText() const;

private:
	/** Whether we should go ahead with import */
	bool										bImport;

	/** Window that owns us */
	TWeakPtr< SWindow >							WidgetWindow;

	// Import type

	/** List of import types to pick from, drives combo box */
	TArray< TSharedPtr<EXmlImportType> >						ImportTypes;

	/** The combo box */
	TSharedPtr< SComboBox< TSharedPtr<EXmlImportType> > >		ImportTypeCombo;

	/** Indicates what kind of asset we want to make from the CSV file */
	EXmlImportType												SelectedImportType;


	// Row type

	/** Array of row struct options */
	TArray< UScriptStruct* >						RowStructs;

	/** The row struct combo box */
	TSharedPtr< SComboBox<UScriptStruct*> >			RowStructCombo;

	/** The selected row struct */
	UScriptStruct*									SelectedStruct;
};
