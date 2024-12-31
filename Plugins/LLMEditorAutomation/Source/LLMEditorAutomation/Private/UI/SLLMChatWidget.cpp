// SLLMChatWidget.cpp
#include "SLLMChatWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

// In SLLMChatWidget.cpp
// SLLMChatWidget.cpp
void SLLMChatWidget::Construct(const FArguments& InArgs)
{
    // LLM Service
    LLMService = InArgs._LLMService;
    
    if (LLMService.IsValid())
    {
        LLMService->OnResponseReceived.AddRaw(this, &SLLMChatWidget::OnLLMResponse);
        LLMService->OnErrorReceived.AddRaw(this, &SLLMChatWidget::OnLLMError);
    }

    // UI Construction
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

SLLMChatWidget::~SLLMChatWidget()
{
    if (LLMService.IsValid())
    {
        LLMService->OnResponseReceived.RemoveAll(this);
        LLMService->OnErrorReceived.RemoveAll(this);
    }
}

FReply SLLMChatWidget::OnSendPromptClicked()
{
    FString PromptText = PromptInputBox->GetText().ToString();
    if (!PromptText.IsEmpty())
    {
        // Add user message to history
        AddMessageToHistory(PromptText, true);
        
        // Send to LLM Service
        if (LLMService.IsValid() && LLMService->IsReady())
        {
            LLMService->SendPrompt(PromptText);
        }
        else
        {
            AddMessageToHistory(TEXT("Error: LLM Service is not ready"), false);
        }
        
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
            .ColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
        ];
    
    ChatHistoryBox->AddSlot()
    .Padding(2.0f)
    [
        MessageWidget.ToSharedRef()
    ];
    
    // Scroll to bottom
    ChatHistoryBox->ScrollToEnd();
}

void SLLMChatWidget::OnLLMResponse(const FString& Response)
{
    AddMessageToHistory(Response, false);
}

void SLLMChatWidget::OnLLMError(const FString& Error)
{
    AddMessageToHistory(FString::Printf(TEXT("Error: %s"), *Error), false);
}