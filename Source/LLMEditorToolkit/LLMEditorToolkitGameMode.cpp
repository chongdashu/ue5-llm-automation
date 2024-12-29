// Copyright Epic Games, Inc. All Rights Reserved.

#include "LLMEditorToolkitGameMode.h"
#include "LLMEditorToolkitCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALLMEditorToolkitGameMode::ALLMEditorToolkitGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
