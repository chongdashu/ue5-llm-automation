// Copyright Epic Games, Inc. All Rights Reserved.

#include "LLMEditorAutomation.h"
// #include "LLMEditorAutomationStyle.h"
// #include "LLMEditorAutomationCommands.h"
// #include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
// #include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "UI/SLLMChatWidget.h"

// static const FName LLMEditorAutomationTabName("LLMEditorAutomation");
const FName FLLMEditorAutomationModule::ChatTabName(TEXT("LLMChat"));

#define LOCTEXT_NAMESPACE "FLLMEditorAutomationModule"

void FLLMEditorAutomationModule::StartupModule()
{
	// // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//
	// FLLMEditorAutomationStyle::Initialize();
	// FLLMEditorAutomationStyle::ReloadTextures();
	//
	// FLLMEditorAutomationCommands::Register();
	//
	// PluginCommands = MakeShareable(new FUICommandList);
	//
	// PluginCommands->MapAction(
	// 	FLLMEditorAutomationCommands::Get().OpenPluginWindow,
	// 	FExecuteAction::CreateRaw(this, &FLLMEditorAutomationModule::PluginButtonClicked),
	// 	FCanExecuteAction());
	//
	// UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLLMEditorAutomationModule::RegisterMenus));
	//
	// FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LLMEditorAutomationTabName, FOnSpawnTab::CreateRaw(this, &FLLMEditorAutomationModule::OnSpawnPluginTab))
	// 	.SetDisplayName(LOCTEXT("FLLMEditorAutomationTabTitle", "LLMEditorAutomation"))
	// 	.SetMenuType(ETabSpawnerMenuType::Hidden);

	RegisterTabSpawner();
}

void FLLMEditorAutomationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// UToolMenus::UnRegisterStartupCallback(this);
	//
	// UToolMenus::UnregisterOwner(this);
	//
	// FLLMEditorAutomationStyle::Shutdown();
	//
	// FLLMEditorAutomationCommands::Unregister();
	//
	// FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LLMEditorAutomationTabName);
	UnregisterTabSpawner();
}

TSharedRef<SDockTab> FLLMEditorAutomationModule::SpawnChatTab(const FSpawnTabArgs& SpawnTabArgs)
{
	// FText WidgetText = FText::Format(
	// 	LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
	// 	FText::FromString(TEXT("FLLMEditorAutomationModule::OnSpawnPluginTab")),
	// 	FText::FromString(TEXT("LLMEditorAutomation.cpp"))
	// 	);

	return SNew(SDockTab)
	   .TabRole(ETabRole::NomadTab)
	   [
		   SNew(SLLMChatWidget)
	   ];
}

// void FLLMEditorAutomationModule::PluginButtonClicked()
// {
// 	FGlobalTabmanager::Get()->TryInvokeTab(LLMEditorAutomationTabName);
// }
//
// void FLLMEditorAutomationModule::RegisterMenus()
// {
// 	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
// 	FToolMenuOwnerScoped OwnerScoped(this);
//
// 	{
// 		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
// 		{
// 			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
// 			Section.AddMenuEntryWithCommandList(FLLMEditorAutomationCommands::Get().OpenPluginWindow, PluginCommands);
// 		}
// 	}
//
// 	{
// 		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
// 		{
// 			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
// 			{
// 				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLLMEditorAutomationCommands::Get().OpenPluginWindow));
// 				Entry.SetCommandList(PluginCommands);
// 			}
// 		}
// 	}
// }

void FLLMEditorAutomationModule::RegisterTabSpawner()
{
	// Register the tab spawner
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		ChatTabName,
		FOnSpawnTab::CreateRaw(this, &FLLMEditorAutomationModule::SpawnChatTab))
		.SetDisplayName(LOCTEXT("ChatTabTitle", "LLM Chat"))
		.SetTooltipText(LOCTEXT("ChatTooltip", "Open the LLM Chat Window"))
		.SetMenuType(ETabSpawnerMenuType::Enabled);

	// Register AI menu
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu.Window");
	FToolMenuSection& Section = Menu->FindOrAddSection("AI");
	Section.Label = LOCTEXT("AISection", "AI");
    
	// Add our tab to the AI section
	Section.AddMenuEntry(
		ChatTabName,
		LOCTEXT("ChatTabTitle", "LLM Chat"),
		LOCTEXT("ChatTooltip", "Open the LLM Chat Window"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("LLMChat")));
			})
		)
	);
}

// ADD: New function for tab unregistration
void FLLMEditorAutomationModule::UnregisterTabSpawner()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ChatTabName);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLLMEditorAutomationModule, LLMEditorAutomation)