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
				.Text(LOCTEXT("DecookButtonLabel", "Uncook Selected Assets"))
				.OnClicked_Static(&Locals::OnUncookButtonClick);
		}
	};

	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(25)
		.IsEnabled_Static(&Locals::IsWidgetEnabled)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(50)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
		.Text(LOCTEXT("HelperLabel", "Select assets to uncook."))
		]
	+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.AutoHeight()
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
