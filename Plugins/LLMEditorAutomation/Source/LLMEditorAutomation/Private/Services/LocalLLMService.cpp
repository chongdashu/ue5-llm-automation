#include "LocalLLMService.h"
#include "WebSocketsModule.h"

FLocalLLMService::FLocalLLMService(const FString& InHost, int32 InPort)
    : Host(InHost)
    , Port(InPort)
{
}

FLocalLLMService::~FLocalLLMService()
{
    if (WebSocket)
    {
        WebSocket->Close();
    }
}

void FLocalLLMService::Initialize()
{
    FString ServerURL = FString::Printf(TEXT("ws://%s:%d"), *Host, Port);
    
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        FModuleManager::Get().LoadModule("WebSockets");
    }

    WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, TEXT("ws"));

    WebSocket->OnConnected().AddRaw(this, &FLocalLLMService::OnWebSocketConnected);
    WebSocket->OnMessage().AddRaw(this, &FLocalLLMService::OnWebSocketMessage);
    WebSocket->OnClosed().AddRaw(this, &FLocalLLMService::OnWebSocketClosed);
    WebSocket->OnConnectionError().AddRaw(this, &FLocalLLMService::OnWebSocketError);
    
    WebSocket->Connect();
}

void FLocalLLMService::SendPrompt(const FString& Prompt)
{
    if (!IsReady())
    {
        OnErrorReceived.Broadcast(TEXT("WebSocket not connected"));
        return;
    }

    WebSocket->Send(Prompt);
}

void FLocalLLMService::OnWebSocketConnected()
{
    UE_LOG(LogTemp, Log, TEXT("LLMService: Connected to server"));
}

void FLocalLLMService::OnWebSocketMessage(const FString& Message)
{
    OnResponseReceived.Broadcast(Message);
}

void FLocalLLMService::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogTemp, Warning, TEXT("LLMService: Connection closed - Status: %d, Reason: %s"), StatusCode, *Reason);
}

void FLocalLLMService::OnWebSocketError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("LLMService: Connection error - %s"), *Error);
    OnErrorReceived.Broadcast(Error);
}