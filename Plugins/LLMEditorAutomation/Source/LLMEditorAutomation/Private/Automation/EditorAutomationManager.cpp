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
    // Simple response parsing - in a real implementation, this would be more sophisticated
    if (Response.Contains(TEXT("follow")) || Response.Contains(TEXT("NPC")) || Response.Contains(TEXT("character")))
    {
        UBlueprint* CreatedBlueprint = CreateFollowerNPCBlueprint();
        if (CreatedBlueprint)
        {
            // Place the actor in the world
            UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
            if (EditorWorld)
            {
                FVector SpawnLocation = FVector(0, 0, 100);  // Slightly above ground
                FRotator SpawnRotation = FRotator::ZeroRotator;
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                
                AActor* SpawnedActor = EditorWorld->SpawnActor(CreatedBlueprint->GeneratedClass, &SpawnLocation, &SpawnRotation, SpawnParams);
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
    if (!Blueprint)
    {
        return;
    }

    // Create TargetActor variable
    FEdGraphPinType PinType;
    PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
    PinType.PinSubCategoryObject = ACharacter::StaticClass();

    FBlueprintEditorUtils::AddMemberVariable(
        Blueprint,
        TEXT("TargetActor"),
        PinType,
        TEXT("The actor to follow")
    );

    // Create FollowDistance variable
    FEdGraphPinType FloatPinType;
    FloatPinType.PinCategory = UEdGraphSchema_K2::PC_Float;

    // Add variable 
    FBlueprintEditorUtils::AddMemberVariable(
        Blueprint,
        TEXT("FollowDistance"),
        FloatPinType,
        TEXT("Distance to maintain from target")
    );

    // Compile the Blueprint to generate the class
    FKismetEditorUtilities::CompileBlueprint(Blueprint);

    // Now we can access the generated class and set default values
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
    if (!Blueprint)
    {
        return;
    }

    UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
    if (!EventGraph)
    {
        return;
    }

    // Find the Tick event node - Fix third occurrence
    UK2Node_Event* TickNode = FBlueprintEditorUtils::FindOverrideForFunction(
        Blueprint,
        AActor::StaticClass(),  // Changed from UGameplayStatics to AActor
        FName(TEXT("ReceiveTick"))  // Changed to FName
    );
    
    if (!TickNode)
    {
        return;
    }

    // Create Get TargetActor node
    UK2Node_VariableGet* GetTargetNode = NewObject<UK2Node_VariableGet>(EventGraph);
    GetTargetNode->VariableReference.SetSelfMember(TEXT("TargetActor"));
    GetTargetNode->NodePosX = TickNode->NodePosX + 200;
    GetTargetNode->NodePosY = TickNode->NodePosY;
    EventGraph->AddNode(GetTargetNode);

    // Create SimpleMoveToActor node
    UK2Node_CallFunction* MoveToNode = NewObject<UK2Node_CallFunction>(EventGraph);
    MoveToNode->SetFromFunction(UAIBlueprintHelperLibrary::StaticClass()->FindFunctionByName(TEXT("SimpleMoveToActor")));
    MoveToNode->NodePosX = GetTargetNode->NodePosX + 200;
    MoveToNode->NodePosY = GetTargetNode->NodePosY;
    EventGraph->AddNode(MoveToNode);

    // Get self reference for the controller owner
    UK2Node_CallFunction* GetControllerNode = NewObject<UK2Node_CallFunction>(EventGraph);
    GetControllerNode->SetFromFunction(AActor::StaticClass()->FindFunctionByName(TEXT("GetController")));
    GetControllerNode->NodePosX = GetTargetNode->NodePosX;
    GetControllerNode->NodePosY = GetTargetNode->NodePosY + 100;
    EventGraph->AddNode(GetControllerNode);

    // Connect the nodes
    UEdGraphPin* TickThenPin = TickNode->FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* MoveToExecPin = MoveToNode->FindPin(UEdGraphSchema_K2::PN_Execute);
    TickThenPin->MakeLinkTo(MoveToExecPin);

    // Connect controller
    UEdGraphPin* ControllerPin = GetControllerNode->FindPin(TEXT("ReturnValue"));
    UEdGraphPin* MoveToControllerPin = MoveToNode->FindPin(TEXT("Controller"));
    ControllerPin->MakeLinkTo(MoveToControllerPin);

    // Connect target
    UEdGraphPin* TargetPin = GetTargetNode->FindPin(TEXT("TargetActor"));
    UEdGraphPin* MoveToTargetPin = MoveToNode->FindPin(TEXT("Goal"));
    TargetPin->MakeLinkTo(MoveToTargetPin);
}