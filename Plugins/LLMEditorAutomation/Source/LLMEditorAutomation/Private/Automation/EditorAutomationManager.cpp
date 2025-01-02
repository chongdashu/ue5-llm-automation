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
#include "K2Node_VariableSet.h"
#include "Engine/Blueprint.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Components/StaticMeshComponent.h"


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
                FVector SpawnLocation = FVector(1640.0f, 1140.0f, 250.0f); // Target position
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

    // Add static mesh component for visualization
    // Get or create the SCS
    USimpleConstructionScript* SCS = Blueprint->SimpleConstructionScript;
    if (!SCS)
    {
        return;
    }

    // Create the static mesh component node
    USCS_Node* MeshNode = SCS->CreateNode(UStaticMeshComponent::StaticClass(), TEXT("VisualMesh"));
    if (MeshNode)
    {
        UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(MeshNode->ComponentTemplate);
        if (MeshComponent)
        {
            // Load the default cube mesh
            UStaticMesh* CubeMesh = Cast<UStaticMesh>(StaticLoadObject(
                UStaticMesh::StaticClass(),
                nullptr,
                TEXT("/Engine/BasicShapes/Cube")
            ));

            if (CubeMesh)
            {
                MeshComponent->SetStaticMesh(CubeMesh);
                MeshComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.0f));
                MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
            }
        }

        // Add to root
        SCS->AddNode(MeshNode);
    }

    // Set auto possess AI
    if (ACharacter* DefaultChar = Cast<ACharacter>(Blueprint->GeneratedClass->GetDefaultObject()))
    {
        DefaultChar->AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
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
    if (!Blueprint) return;

    UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
    if (!EventGraph) return;

    // Create BeginPlay event
    UK2Node_Event* BeginPlayNode = FBlueprintEditorUtils::FindOverrideForFunction(Blueprint, AActor::StaticClass(), FName(TEXT("ReceiveBeginPlay")));
    if (!BeginPlayNode)
    {
        BeginPlayNode = NewObject<UK2Node_Event>(EventGraph);
        BeginPlayNode->EventReference.SetExternalMember(TEXT("ReceiveBeginPlay"), ACharacter::StaticClass());
        BeginPlayNode->NodePosX = 0;
        BeginPlayNode->NodePosY = 0;
        EventGraph->AddNode(BeginPlayNode);
        BeginPlayNode->AllocateDefaultPins();
    }

    // Create GetPlayerCharacter node
    UK2Node_CallFunction* GetPlayerNode = NewObject<UK2Node_CallFunction>(EventGraph);
    UFunction* GetPlayerFunc = UGameplayStatics::StaticClass()->FindFunctionByName(TEXT("GetPlayerCharacter"));
    if (GetPlayerFunc)
    {
        GetPlayerNode->SetFromFunction(GetPlayerFunc);
        GetPlayerNode->NodePosX = BeginPlayNode->NodePosX + 250;
        GetPlayerNode->NodePosY = BeginPlayNode->NodePosY;
        EventGraph->AddNode(GetPlayerNode);
        GetPlayerNode->AllocateDefaultPins();

        // Set the PlayerIndex pin to 0
        if (UEdGraphPin* PlayerIndexPin = GetPlayerNode->FindPin(TEXT("PlayerIndex")))
        {
            PlayerIndexPin->DefaultValue = TEXT("0");
        }
    }

    // Create Set TargetActor node
    UK2Node_VariableSet* SetTargetNode = NewObject<UK2Node_VariableSet>(EventGraph);
    SetTargetNode->VariableReference.SetSelfMember(TEXT("TargetActor"));
    SetTargetNode->NodePosX = GetPlayerNode->NodePosX + 250;
    SetTargetNode->NodePosY = GetPlayerNode->NodePosY;
    EventGraph->AddNode(SetTargetNode);
    SetTargetNode->AllocateDefaultPins();

    // Connect execution pins
    if (UEdGraphPin* BeginPlayThenPin = BeginPlayNode->FindPin(UEdGraphSchema_K2::PN_Then))
    {
        if (UEdGraphPin* SetTargetExecPin = SetTargetNode->FindPin(UEdGraphSchema_K2::PN_Execute))
        {
            BeginPlayThenPin->MakeLinkTo(SetTargetExecPin);
        }
    }

    // Connect GetPlayerCharacter to Set TargetActor
    if (UEdGraphPin* PlayerPin = GetPlayerNode->FindPin(TEXT("ReturnValue")))
    {
        if (UEdGraphPin* TargetPin = SetTargetNode->FindPin(TEXT("TargetActor")))
        {
            PlayerPin->MakeLinkTo(TargetPin);
        }
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
        TickNode->NodePosY = 0;
        EventGraph->AddNode(TickNode);
        TickNode->AllocateDefaultPins();
    }

    // Create SimpleMoveToActor node
    UK2Node_CallFunction* MoveToNode = NewObject<UK2Node_CallFunction>(EventGraph);
    UFunction* SimpleMoveToFunc = UAIBlueprintHelperLibrary::StaticClass()->FindFunctionByName(TEXT("SimpleMoveToActor"));
    if (SimpleMoveToFunc)
    {
        MoveToNode->SetFromFunction(SimpleMoveToFunc);
        MoveToNode->NodePosX = TickNode->NodePosX + 300;
        MoveToNode->NodePosY = TickNode->NodePosY;
        EventGraph->AddNode(MoveToNode);
        MoveToNode->AllocateDefaultPins();
    }
    
    // Create Get Controller node
    UK2Node_CallFunction* GetControllerNode = NewObject<UK2Node_CallFunction>(EventGraph);
    UFunction* GetControllerFunc = APawn::StaticClass()->FindFunctionByName(TEXT("GetController"));
    if (GetControllerFunc)
    {
        GetControllerNode->SetFromFunction(GetControllerFunc);
        GetControllerNode->NodePosX = 200;
        GetControllerNode->NodePosY = 200;
        EventGraph->AddNode(GetControllerNode);
        GetControllerNode->AllocateDefaultPins();
    }

    // Create Get TargetActor variable node
    UK2Node_VariableGet* GetTargetNode = NewObject<UK2Node_VariableGet>(EventGraph);
    GetTargetNode->VariableReference.SetSelfMember(TEXT("TargetActor"));
    GetTargetNode->NodePosX = MoveToNode->NodePosX - 150;
    GetTargetNode->NodePosY = MoveToNode->NodePosY + 50;
    EventGraph->AddNode(GetTargetNode);
    GetTargetNode->AllocateDefaultPins();

    // Connect Execution pins
    if (UEdGraphPin* TickThenPin = TickNode->FindPin(UEdGraphSchema_K2::PN_Then))
    {
        if (UEdGraphPin* MoveToExecPin = MoveToNode->FindPin(UEdGraphSchema_K2::PN_Execute))
        {
            TickThenPin->MakeLinkTo(MoveToExecPin);
        }
    }

    // Connect Get Controller to Simple Move To
    if (UEdGraphPin* ControllerPin = GetControllerNode->FindPin(TEXT("ReturnValue")))
    {
        if (UEdGraphPin* MoveToControllerPin = MoveToNode->FindPin(TEXT("Controller")))
        {
            ControllerPin->MakeLinkTo(MoveToControllerPin);
        }
    }

    // Connect TargetActor to Simple Move To Goal
    if (UEdGraphPin* TargetPin = GetTargetNode->FindPin(TEXT("TargetActor")))
    {
        if (UEdGraphPin* GoalPin = MoveToNode->FindPin(TEXT("Goal")))
        {
            TargetPin->MakeLinkTo(GoalPin);
        }
    }

    

    // Compile the blueprint
    FKismetEditorUtilities::CompileBlueprint(Blueprint);
    UE_LOG(LogTemp, Log, TEXT("Follow logic implemented for blueprint: %s"), *Blueprint->GetName());
}
