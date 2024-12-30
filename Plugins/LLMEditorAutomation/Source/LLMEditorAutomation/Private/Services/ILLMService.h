// ILLMService.h
#pragma once

#include "CoreMinimal.h"

// Delegate for handling LLM responses
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLLMResponse, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLLMError, const FString&);

/**
 * Interface for LLM services (local or remote)
 */
class ILLMService
{
public:
	virtual ~ILLMService() = default;
    
	/** Initialize the service */
	virtual void Initialize() = 0;
    
	/** Send a prompt to the LLM */
	virtual void SendPrompt(const FString& Prompt) = 0;
    
	/** Check if the service is ready to accept prompts */
	virtual bool IsReady() const = 0;
    
	/** Response delegates */
	FOnLLMResponse OnResponseReceived;
	FOnLLMError OnErrorReceived;
};