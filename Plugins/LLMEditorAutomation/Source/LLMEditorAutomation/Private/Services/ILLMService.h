#pragma once

#include "CoreMinimal.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLLMResponse, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLLMError, const FString&);

class ILLMService
{
public:
	virtual ~ILLMService() = default;
	virtual void Initialize() = 0;
	virtual void SendPrompt(const FString& Prompt) = 0;
	virtual bool IsReady() const = 0;
    
	FOnLLMResponse OnResponseReceived;
	FOnLLMError OnErrorReceived;
};