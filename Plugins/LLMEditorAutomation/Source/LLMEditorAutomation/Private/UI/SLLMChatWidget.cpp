// SLLMChatWidget.cpp
#include "SLLMChatWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

// In SLLMChatWidget.cpp
// SLLMChatWidget.cpp
void SLLMChatWidget::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SVerticalBox)
        
        // Chat History
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(5.0f)
        [
            SAssignNew(ChatHistoryBox, SScrollBox)
        ]
        
        // Input Area
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(5.0f)
        [
            SNew(SHorizontalBox)
            
            // Prompt Input
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SAssignNew(PromptInputBox, SMultiLineEditableTextBox)
                .HintText(NSLOCTEXT("LLMChat", "PromptHint", "Enter your instruction here..."))
                .Style(&FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
                .AutoWrapText(true)
            ]
            
            // Send Button
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(5.0f, 0.0f, 0.0f, 0.0f)
            [
                SNew(SButton)
                .Text(NSLOCTEXT("LLMChat", "SendButton", "Send"))
                .OnClicked(this, &SLLMChatWidget::OnSendPromptClicked)
            ]
        ]
    ];
}

FReply SLLMChatWidget::OnSendPromptClicked()
{
    FString PromptText = PromptInputBox->GetText().ToString();
    if (!PromptText.IsEmpty())
    {
        // Add user message to history
        AddMessageToHistory(PromptText, true);
        
        // Clear input
        PromptInputBox->SetText(FText::GetEmpty());
        
        // TODO: Send to LLM Service
        
        return FReply::Handled();
    }
    
    return FReply::Unhandled();
}

void SLLMChatWidget::AddMessageToHistory(const FString& Message, bool bIsUser)
{
    TSharedPtr<SBorder> MessageWidget =
        SNew(SBorder)
        .Padding(FMargin(5.0f))
        .BorderImage(GetBackgroundBrush())
        .BorderBackgroundColor(bIsUser ? GetUserMessageColor() : GetAIMessageColor())
        [
            SNew(STextBlock)
            .Text(FText::FromString(Message))
            .AutoWrapText(true)
        ];
    
    ChatHistoryBox->AddSlot()
    .Padding(2.0f)
    [
        MessageWidget.ToSharedRef()
    ];
    
    // Scroll to bottom
    ChatHistoryBox->ScrollToEnd();
}