#pragma once

#include "ILLMService.h"
#include "Common/TcpListener.h"
#include "HAL/Runnable.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

/**
 * Implementation of ILLMService that communicates with a local LLM server
 */
class FLocalLLMService : public ILLMService, public FRunnable
{
public:
    FLocalLLMService(const FString& InHost = TEXT("127.0.0.1"), int32 InPort = 5000);
    virtual ~FLocalLLMService();

    // ILLMService Interface
    virtual void Initialize() override;
    virtual void SendPrompt(const FString& Prompt) override;
    virtual bool IsReady() const override { return bIsConnected; }

    // FRunnable Interface
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;

private:
    bool ConnectToServer();
    void DisconnectFromServer();
    bool SendData(const FString& Data);
    bool ReceiveData(FString& OutData);

private:
    FString Host;
    int32 Port;
    
    FRunnableThread* Thread;
    FSocket* Socket;
    FCriticalSection SocketCritical;
    
    bool bIsConnected;
    bool bShouldRun;
};