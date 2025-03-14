// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScrapyardEdMode.h"
#include "ScrapyardEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FScrapyardEdMode::EM_ScrapyardEdModeId = TEXT("EM_ScrapyardEdMode");

FScrapyardEdMode::FScrapyardEdMode()
{

}

FScrapyardEdMode::~FScrapyardEdMode()
{

}

void FScrapyardEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FScrapyardEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FScrapyardEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FScrapyardEdMode::UsesToolkits() const
{
	return true;
}




