// EditorAutomationManager.cpp
#include "EditorAutomationManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "GameFramework/Character.h"  
#include "GameFramework/CharacterMovementComponent.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "AIModule.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet2/KismetEditorUtilities.h"

FEditorAutomationManager* FEditorAutomationManager::Instance = nullptr;

FEditorAutomationManager& FEditorAutomationManager::Get()
{
    if (!Instance)
    {
        Instance = new FEditorAutomationManager();
    }
    return *Instance;
}

void FEditorAutomationManager::ProcessLLMResponse(const FString& Response)
{
    // Check if this is an NPC creation request
    if (Response.Contains(TEXT("follow")) || Response.Contains(TEXT("NPC")) || Response.Contains(TEXT("character")))
    {
        const FString BlueprintName = TEXT("BP_FollowerNPC");
        UBlueprint* CreatedBlueprint = CreateFollowerNPCBlueprint(BlueprintName);
        
        if (CreatedBlueprint)
        {
            // Place in world at a reasonable position
            UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
            if (EditorWorld)
            {
                FVector SpawnLocation = FVector(0, 0, 100); // Slightly above ground
                FRotator SpawnRotation = FRotator::ZeroRotator;
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = 
                    ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                
                AActor* SpawnedActor = EditorWorld->SpawnActor(
                    CreatedBlueprint->GeneratedClass, 
                    &SpawnLocation, 
                    &SpawnRotation, 
                    SpawnParams
                );
                
                if (SpawnedActor)
                {
                    // Select the newly spawned actor
                    GEditor->SelectActor(SpawnedActor, true, true);
                }
            }
        }
    }
}

UBlueprint* FEditorAutomationManager::CreateFollowerNPCBlueprint(const FString& BlueprintName)
{
    UE_LOG(LogTemp, Log, TEXT("Creating Blueprint: %s"), *BlueprintName);
    // Create the Blueprint
    UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
        ACharacter::StaticClass(),
        GEditor->GetEditorWorldContext().World()->GetCurrentLevel(),
        *BlueprintName,
        BPTYPE_Normal,
        UBlueprint::StaticClass(),
        UBlueprintGeneratedClass::StaticClass()
    );

    if (!Blueprint)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unable to create create Blueprint: %s"), *BlueprintName);
        return nullptr;
    }

    // Add components and setup logic
    SetupNPCMovementComponent(Blueprint);
    CreateVariables(Blueprint);
    CreateEventGraph(Blueprint);
    
    // Compile the Blueprint
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    FKismetEditorUtilities::CompileBlueprint(Blueprint);
    
    return Blueprint;
}

void FEditorAutomationManager::SetupNPCMovementComponent(UBlueprint* Blueprint)
{
    if (!Blueprint)
    {
        return;
    }

    // Get the Character Movement Component
    UCharacterMovementComponent* MovementComp = Blueprint->GeneratedClass->GetDefaultObject<ACharacter>()->GetCharacterMovement();
    if (MovementComp)
    {
        // Configure movement settings
        MovementComp->MaxWalkSpeed = 400.f;
        MovementComp->bOrientRotationToMovement = true;
        MovementComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    }
}

void FEditorAutomationManager::CreateVariables(UBlueprint* Blueprint)
{
    if (!Blueprint) return;

    // TargetActor variable
    FEdGraphPinType PinType;
    PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
    PinType.PinSubCategoryObject = ACharacter::StaticClass();

    FBlueprintEditorUtils::AddMemberVariable(Blueprint, TEXT("TargetActor"), PinType);

    // FollowDistance variable with default value
    FEdGraphPinType FloatPinType;
    FloatPinType.PinCategory = UEdGraphSchema_K2::PC_Float;

    FBlueprintEditorUtils::AddMemberVariable(Blueprint, TEXT("FollowDistance"), FloatPinType);

    // Set default value for FollowDistance
    if (UClass* GeneratedClass = Blueprint->GeneratedClass)
    {
        if (UObject* DefaultObject = GeneratedClass->GetDefaultObject())
        {
            if (FProperty* FollowDistanceProperty = GeneratedClass->FindPropertyByName(TEXT("FollowDistance")))
            {
                float* ValuePtr = FollowDistanceProperty->ContainerPtrToValuePtr<float>(DefaultObject);
                if (ValuePtr)
                {
                    *ValuePtr = 200.0f;
                }
            }
        }
    }
}

void FEditorAutomationManager::CreateEventGraph(UBlueprint* Blueprint)
{
    if (!Blueprint)
    {
        return;
    }

    UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
    if (!EventGraph)
    {
        return;
    }

    // Create BeginPlay event - Fix first occurrence
    UK2Node_Event* BeginPlayNode = FBlueprintEditorUtils::FindOverrideForFunction(
        Blueprint,
        AActor::StaticClass(),  // Changed from UGameplayStatics to AActor
        FName(TEXT("ReceiveBeginPlay"))  // Changed to FName
    );
    
    if (!BeginPlayNode)
    {
        BeginPlayNode = NewObject<UK2Node_Event>(EventGraph);
        BeginPlayNode->EventReference.SetExternalMember(TEXT("ReceiveBeginPlay"), ACharacter::StaticClass());
        BeginPlayNode->NodePosX = 0;
        BeginPlayNode->NodePosY = 0;
        EventGraph->AddNode(BeginPlayNode);
    }

    // Create Tick event - Fix second occurrence
    UK2Node_Event* TickNode = FBlueprintEditorUtils::FindOverrideForFunction(
        Blueprint,
        AActor::StaticClass(),  // Changed from UGameplayStatics to AActor
        FName(TEXT("ReceiveTick"))  // Changed to FName
    );
    
    if (!TickNode)
    {
        TickNode = NewObject<UK2Node_Event>(EventGraph);
        TickNode->EventReference.SetExternalMember(TEXT("ReceiveTick"), ACharacter::StaticClass());
        TickNode->NodePosX = 0;
        TickNode->NodePosY = 200;
        EventGraph->AddNode(TickNode);
    }

    ImplementFollowLogic(Blueprint);
}

void FEditorAutomationManager::ImplementFollowLogic(UBlueprint* Blueprint)
{
    if (!Blueprint) return;

    UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
    if (!EventGraph) return;

    // Create Tick event
    UK2Node_Event* TickNode = FBlueprintEditorUtils::FindOverrideForFunction(Blueprint, AActor::StaticClass(), FName(TEXT("ReceiveTick")));
    if (!TickNode)
    {
        TickNode = NewObject<UK2Node_Event>(EventGraph);
        TickNode->EventReference.SetExternalMember(TEXT("ReceiveTick"), ACharacter::StaticClass());
        TickNode->NodePosX = 0;
        TickNode->NodePosY = 200;
        EventGraph->AddNode(TickNode);
    }

    // Get TargetActor variable
    UK2Node_VariableGet* GetTargetNode = NewObject<UK2Node_VariableGet>(EventGraph);
    GetTargetNode->VariableReference.SetSelfMember(TEXT("TargetActor"));
    GetTargetNode->NodePosX = TickNode->NodePosX + 250;
    GetTargetNode->NodePosY = TickNode->NodePosY;
    EventGraph->AddNode(GetTargetNode);

    // Create AI Move To node
    UK2Node_CallFunction* MoveToNode = NewObject<UK2Node_CallFunction>(EventGraph);
    MoveToNode->FunctionReference.SetExternalMember(TEXT("SimpleMoveToActor"), UAIBlueprintHelperLibrary::StaticClass());
    MoveToNode->NodePosX = GetTargetNode->NodePosX + 250;
    MoveToNode->NodePosY = GetTargetNode->NodePosY;
    EventGraph->AddNode(MoveToNode);

    // Connect execution flow
    if (UEdGraphPin* TickThenPin = TickNode->FindPin(UEdGraphSchema_K2::PN_Then))
    {
        if (UEdGraphPin* MoveToExecPin = MoveToNode->FindPin(UEdGraphSchema_K2::PN_Execute))
        {
            TickThenPin->MakeLinkTo(MoveToExecPin);
        }
    }

    // Connect target actor
    if (UEdGraphPin* TargetPin = GetTargetNode->FindPin(TEXT("TargetActor")))
    {
        if (UEdGraphPin* GoalPin = MoveToNode->FindPin(TEXT("Goal")))
        {
            TargetPin->MakeLinkTo(GoalPin);
        }
    }

    // Get controller and connect
    UK2Node_CallFunction* GetControllerNode = NewObject<UK2Node_CallFunction>(EventGraph);
    GetControllerNode->FunctionReference.SetExternalMember(TEXT("GetController"), ACharacter::StaticClass());
    GetControllerNode->NodePosX = GetTargetNode->NodePosX;
    GetControllerNode->NodePosY = GetTargetNode->NodePosY - 100;
    EventGraph->AddNode(GetControllerNode);

    if (UEdGraphPin* ControllerPin = GetControllerNode->FindPin(TEXT("ReturnValue")))
    {
        if (UEdGraphPin* MoveToControllerPin = MoveToNode->FindPin(TEXT("Controller")))
        {
            ControllerPin->MakeLinkTo(MoveToControllerPin);
        }
    }

    // Compile blueprint
    FKismetEditorUtilities::CompileBlueprint(Blueprint);
}