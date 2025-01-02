// SLLMChatWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Services/ILLMService.h"
#include "Automation//EditorAutomationManager.h"

class SLLMChatWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLLMChatWidget) : _LLMService() {} SLATE_ARGUMENT(TSharedPtr<ILLMService>, LLMService)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual ~SLLMChatWidget();

private:
	// UI Elements
	TSharedPtr<SMultiLineEditableTextBox> PromptInputBox;
	TSharedPtr<SScrollBox> ChatHistoryBox;

	// Service reference
	TSharedPtr<ILLMService> LLMService;
	void OnLLMResponse(const FString& Response);
	void OnLLMError(const FString& Error);
    
	// UI Callbacks
	FReply OnSendPromptClicked();
	void AddMessageToHistory(const FString& Message, bool bIsUser);
    
	// UI Styles
	const FSlateBrush* GetBackgroundBrush() const 
	{ 
		return FCoreStyle::Get().GetBrush("WhiteBrush"); 
	}
	FSlateColor GetUserMessageColor() const { return FLinearColor(0.8f, 0.9f, 1.0f); }
	FSlateColor GetAIMessageColor() const { return FLinearColor(0.9f, 0.9f, 0.9f); }

	// Automation
	void ProcessAutomationRequest(const FString& Response);
};