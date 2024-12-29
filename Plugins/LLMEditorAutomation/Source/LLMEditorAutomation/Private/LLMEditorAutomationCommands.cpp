// Copyright Epic Games, Inc. All Rights Reserved.

#include "LLMEditorAutomationCommands.h"

#define LOCTEXT_NAMESPACE "FLLMEditorAutomationModule"

void FLLMEditorAutomationCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "LLMEditorAutomation", "Bring up LLMEditorAutomation window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
