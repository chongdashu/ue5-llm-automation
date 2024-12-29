// Copyright Epic Games, Inc. All Rights Reserved.

#include "LLMEditorAutomationStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FLLMEditorAutomationStyle::StyleInstance = nullptr;

void FLLMEditorAutomationStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FLLMEditorAutomationStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FLLMEditorAutomationStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("LLMEditorAutomationStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FLLMEditorAutomationStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("LLMEditorAutomationStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("LLMEditorAutomation")->GetBaseDir() / TEXT("Resources"));

	Style->Set("LLMEditorAutomation.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

void FLLMEditorAutomationStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FLLMEditorAutomationStyle::Get()
{
	return *StyleInstance;
}
