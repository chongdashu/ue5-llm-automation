// SLLMChatWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"

class SLLMChatWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLLMChatWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// UI Elements
	TSharedPtr<SMultiLineEditableTextBox> PromptInputBox;
	TSharedPtr<SScrollBox> ChatHistoryBox;
    
	// UI Callbacks
	FReply OnSendPromptClicked();
	void AddMessageToHistory(const FString& Message, bool bIsUser);
    
	// UI Styles
	const FSlateBrush* GetBackgroundBrush() const 
	{ 
		return FCoreStyle::Get().GetBrush("ChatMessage.Background"); 
	}
	FSlateColor GetUserMessageColor() const { return FLinearColor(0.8f, 0.9f, 1.0f); }
	FSlateColor GetAIMessageColor() const { return FLinearColor(0.9f, 0.9f, 0.9f); }
};