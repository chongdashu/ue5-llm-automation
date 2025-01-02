// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FLLMEditorAutomationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	// void PluginButtonClicked();
	
	static FLLMEditorAutomationModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FLLMEditorAutomationModule>("LLMEditorAutomation");
	}

	TSharedPtr<class ILLMService> GetLLMService() const { return LLMService; }
	
private:

	// void RegisterMenus();

	// TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<class SDockTab> SpawnChatTab(const class FSpawnTabArgs& SpawnTabArgs);

	void RegisterTabSpawner();
	void UnregisterTabSpawner();

private:
	// TSharedPtr<class FUICommandList> PluginCommands;
	static const FName ChatTabName;
	TSharedPtr<class ILLMService> LLMService;
} ;

