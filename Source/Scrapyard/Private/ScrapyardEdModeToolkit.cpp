// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScrapyardEdModeToolkit.h"
#include "ScrapyardEdMode.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"
#include "Uncooker.h"

#define LOCTEXT_NAMESPACE "FScrapyardEdModeToolkit"

FScrapyardEdModeToolkit::FScrapyardEdModeToolkit()
{
}
void FScrapyardEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	struct Locals
	{
		static bool IsWidgetEnabled()
		{
			// Enable the button only if there are selected assets
			TArray<UObject*> SelectedObjects;
			GEditor->GetSelectedObjects()->GetSelectedObjects(UObject::StaticClass(), SelectedObjects);

			return SelectedObjects.Num() > 0;
		}

		static FReply OnUncookButtonClick()
		{
			// Get selected objects in the editor
			TArray<UObject*> SelectedObjects;
			for (FSelectionIterator Iter(*GEditor->GetSelectedObjects()); Iter; ++Iter)
			{
				UObject* Object = Cast<UObject>(*Iter);
				if (Object)
				{
					SelectedObjects.Add(Object);
				}
			}

			FUncooker::UncookAssets(SelectedObjects);

			return FReply::Handled();
		}

		static TSharedRef<SWidget> MakeUncookButton()
		{
			return SNew(SButton)
				.IsEnabled_Static(&Locals::IsWidgetEnabled)
				.Text(LOCTEXT("DecookButtonLabel", "Uncook"))
				.OnClicked_Static(&Locals::OnUncookButtonClick)
				.ContentPadding(FMargin(10.0f)) 
				.HAlign(HAlign_Center) 
				.VAlign(VAlign_Center)
				.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DecookButtonLabel", "Uncook"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
				];
		}
	};

	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(25)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(20)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("Header", "Scrapyard Mode"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24)) 
					.Justification(ETextJustify::Center) 
					.ColorAndOpacity(FLinearColor::White) 
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("HelperLabel", "1. Select assets to uncook."))
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
				.Text(LOCTEXT("HelperLabel1", "2. Right click any of them."))
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
				.Text(LOCTEXT("HelperLabel2", "3. Reselect the assets."))
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
				.Text(LOCTEXT("HelperLabe3", "4. Click uncook button."))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(30)
				[
					Locals::MakeUncookButton()  
				]
		];

	FModeToolkit::Init(InitToolkitHost);
}

FName FScrapyardEdModeToolkit::GetToolkitFName() const
{
	return FName("ScrapyardEdMode");
}

FText FScrapyardEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("ScrapyardEdModeToolkit", "DisplayName", "ScrapyardEdMode Tool");
}

class FEdMode* FScrapyardEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FScrapyardEdMode::EM_ScrapyardEdModeId);
}

#undef LOCTEXT_NAMESPACE
