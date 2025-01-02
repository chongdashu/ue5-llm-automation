#pragma once

#include "ILLMService.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"

class FLocalLLMService : public ILLMService
{
public:
    FLocalLLMService(const FString& InHost = TEXT("127.0.0.1"), int32 InPort = 8765);
    virtual ~FLocalLLMService();

    virtual void Initialize() override;
    virtual void SendPrompt(const FString& Prompt) override;
    virtual bool IsReady() const override { return WebSocket && WebSocket->IsConnected(); }

private:
    void OnWebSocketConnected();
    void OnWebSocketMessage(const FString& Message);
    void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
    void OnWebSocketError(const FString& Error);

    FString Host;
    int32 Port;
    TSharedPtr<IWebSocket> WebSocket;
};