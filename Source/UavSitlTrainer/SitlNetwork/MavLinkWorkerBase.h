
#pragma once

#include "HAL/Runnable.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"

class FMavLinkWorkerBase : public FRunnable
{
public:
	virtual bool   Init() override { return true; }
	virtual uint32 Run() override final;
	virtual void   Stop() override { StopTaskCounter.Increment(); }

protected:
	virtual void DrainOutbound() = 0;
	virtual void ReceiveInbound() = 0;
	virtual bool IsConnectionValid() { return true; }
	virtual void OnBeforeLoop() {}
	virtual void OnAfterLoop() {}
	virtual void OnSleep() { FPlatformProcess::Sleep(SleepSeconds); }

	static ISocketSubsystem*		 GetSocketSubsystem() { return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM); }
	static FSocket*					 CreateSocket(FName Type, const FString& Name);
	static TSharedPtr<FInternetAddr> CreateAddr(const FString& IP, int32 Port);
	static TSharedPtr<FInternetAddr> CreateBindAddr(int32 Port);
	static void						 DestroySocket(FSocket*& Socket);

	FThreadSafeCounter StopTaskCounter;
	float			   SleepSeconds = 0.001f;
};