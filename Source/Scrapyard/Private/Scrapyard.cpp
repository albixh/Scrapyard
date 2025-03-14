// Copyright Epic Games, Inc. All Rights Reserved.

#include "Scrapyard.h"
#include "ScrapyardEdMode.h"

#define LOCTEXT_NAMESPACE "FScrapyardModule"

void FScrapyardModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FScrapyardEdMode>(FScrapyardEdMode::EM_ScrapyardEdModeId, LOCTEXT("ScrapyardEdModeName", "ScrapyardEdMode"), FSlateIcon(), true);
}

void FScrapyardModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FScrapyardEdMode::EM_ScrapyardEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FScrapyardModule, Scrapyard)