// EditorAutomationManager.h
#pragma once

#include "CoreMinimal.h"

class UBlueprint;
class UEdGraph;
class UK2Node_Event;
class UClass;
class FProperty;

class LLMEDITORAUTOMATION_API FEditorAutomationManager : public TSharedFromThis<FEditorAutomationManager>
{
public:
	static FEditorAutomationManager& Get();
    
	/** Process an LLM response and execute editor automation tasks */
	void ProcessLLMResponse(const FString& Response);
    
	/** Create an NPC Blueprint with basic following behavior */
	UBlueprint* CreateFollowerNPCBlueprint(const FString& BlueprintName = TEXT("BP_FollowerNPC"));

private:
	/** Singleton instance */
	static FEditorAutomationManager* Instance;
    
	/** Blueprint creation helpers */
	bool AddComponentToBlueprint(UBlueprint* Blueprint, UClass* ComponentClass);
	void SetupNPCMovementComponent(UBlueprint* Blueprint);
	void ImplementFollowLogic(UBlueprint* Blueprint);
    
	/** Graph manipulation helpers */
	void CreateVariables(UBlueprint* Blueprint);
	void CreateEventGraph(UBlueprint* Blueprint);
    
	FEditorAutomationManager() = default;
	~FEditorAutomationManager() = default;
};