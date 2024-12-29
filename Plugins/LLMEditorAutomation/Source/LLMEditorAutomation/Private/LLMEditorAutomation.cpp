// Copyright Epic Games, Inc. All Rights Reserved.

#include "LLMEditorAutomation.h"
#include "LLMEditorAutomationStyle.h"
#include "LLMEditorAutomationCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName LLMEditorAutomationTabName("LLMEditorAutomation");

#define LOCTEXT_NAMESPACE "FLLMEditorAutomationModule"

void FLLMEditorAutomationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLLMEditorAutomationStyle::Initialize();
	FLLMEditorAutomationStyle::ReloadTextures();

	FLLMEditorAutomationCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLLMEditorAutomationCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FLLMEditorAutomationModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLLMEditorAutomationModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LLMEditorAutomationTabName, FOnSpawnTab::CreateRaw(this, &FLLMEditorAutomationModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FLLMEditorAutomationTabTitle", "LLMEditorAutomation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FLLMEditorAutomationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FLLMEditorAutomationStyle::Shutdown();

	FLLMEditorAutomationCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LLMEditorAutomationTabName);
}

TSharedRef<SDockTab> FLLMEditorAutomationModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FLLMEditorAutomationModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("LLMEditorAutomation.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FLLMEditorAutomationModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(LLMEditorAutomationTabName);
}

void FLLMEditorAutomationModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FLLMEditorAutomationCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLLMEditorAutomationCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLLMEditorAutomationModule, LLMEditorAutomation)