#include "LocalLLMService.h"
#include "Async/Async.h"

FLocalLLMService::FLocalLLMService(const FString& InHost, int32 InPort)
    : Host(InHost)
    , Port(InPort)
    , Thread(nullptr)
    , Socket(nullptr)
    , bIsConnected(false)
    , bShouldRun(false)
{
}

FLocalLLMService::~FLocalLLMService()
{
    Stop();
    DisconnectFromServer();
}

void FLocalLLMService::Initialize()
{
    bShouldRun = true;
    Thread = FRunnableThread::Create(this, TEXT("LocalLLMService"), 0, TPri_Normal);
}

bool FLocalLLMService::Init()
{
    return true;
}

uint32 FLocalLLMService::Run()
{
    while (bShouldRun)
    {
        if (!bIsConnected)
        {
            if (ConnectToServer())
            {
                bIsConnected = true;
            }
            else
            {
                // Wait before retrying connection
                FPlatformProcess::Sleep(1.0f);
                continue;
            }
        }

        // Check for responses
        FString Response;
        if (ReceiveData(Response))
        {
            // Broadcast response on game thread
            AsyncTask(ENamedThreads::GameThread, [this, Response]()
            {
                OnResponseReceived.Broadcast(Response);
            });
        }

        // Small sleep to prevent tight loop
        FPlatformProcess::Sleep(0.016f);
    }

    return 0;
}

void FLocalLLMService::Stop()
{
    bShouldRun = false;

    if (Thread)
    {
        Thread->Kill(true);
        delete Thread;
        Thread = nullptr;
    }
}

void FLocalLLMService::Exit()
{
    DisconnectFromServer();
}

void FLocalLLMService::SendPrompt(const FString& Prompt)
{
    if (!bIsConnected)
    {
        OnErrorReceived.Broadcast(TEXT("Not connected to LLM server"));
        return;
    }

    if (!SendData(Prompt))
    {
        OnErrorReceived.Broadcast(TEXT("Failed to send prompt to LLM server"));
    }
}

bool FLocalLLMService::ConnectToServer()
{
    FScopeLock Lock(&SocketCritical);

    if (Socket)
    {
        DisconnectFromServer();
    }

    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("LocalLLMConnection"), false);

    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    bool bIsValid = false;
    Addr->SetIp(*Host, bIsValid);
    if (!bIsValid)
    {
        return false;
    }
    Addr->SetPort(Port);

    return Socket->Connect(*Addr);
}

void FLocalLLMService::DisconnectFromServer()
{
    FScopeLock Lock(&SocketCritical);

    if (Socket)
    {
        Socket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
        Socket = nullptr;
    }
    bIsConnected = false;
}

bool FLocalLLMService::SendData(const FString& Data)
{
    FScopeLock Lock(&SocketCritical);
    
    if (!Socket)
    {
        return false;
    }

    // Convert string to UTF8
    FTCHARToUTF8 Converter(*Data);
    int32 BytesSent = 0;
    return Socket->Send((uint8*)Converter.Get(), Converter.Length(), BytesSent);
}

bool FLocalLLMService::ReceiveData(FString& OutData)
{
    FScopeLock Lock(&SocketCritical);
    
    if (!Socket)
    {
        return false;
    }

    uint32 Size;
    if (!Socket->HasPendingData(Size))
    {
        return false;
    }

    TArray<uint8> ReceivedData;
    ReceivedData.SetNumUninitialized(Size);
    int32 BytesRead = 0;

    if (Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead))
    {
        // Convert received data to string
        FUTF8ToTCHAR Converter((const ANSICHAR*)ReceivedData.GetData(), BytesRead);
        OutData = FString(Converter.Length(), Converter.Get());
        return true;
    }

    return false;
}