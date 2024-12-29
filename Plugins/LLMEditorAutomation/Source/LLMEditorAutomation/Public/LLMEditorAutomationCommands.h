// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LLMEditorAutomationStyle.h"

class FLLMEditorAutomationCommands : public TCommands<FLLMEditorAutomationCommands>
{
public:

	FLLMEditorAutomationCommands()
		: TCommands<FLLMEditorAutomationCommands>(TEXT("LLMEditorAutomation"), NSLOCTEXT("Contexts", "LLMEditorAutomation", "LLMEditorAutomation Plugin"), NAME_None, FLLMEditorAutomationStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};