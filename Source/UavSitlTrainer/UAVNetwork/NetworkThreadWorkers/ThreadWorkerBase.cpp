#include "ThreadWorkerBase.h"

uint32 FThreadWorkerBase::Run()
{
	OnBeforeLoop();
	while (StopTaskCounter.GetValue() == 0 && IsConnectionValid()) {
		DrainOutbound();  // subclass sends pending data
		ReceiveInbound(); // subclass reads and enNetworkChannels
		OnSleep();
	}
	OnAfterLoop();
	return 0;
}

FSocket* FThreadWorkerBase::CreateSocket(FName Type, const FString& Name)
{
	FSocket* Socket = GetSocketSubsystem()->CreateSocket(Type, Name, true);
	if (Socket) {
		Socket->SetNonBlocking(true);
		Socket->SetReuseAddr(true);
	}
	return Socket;
}

TSharedPtr<FInternetAddr> FThreadWorkerBase::CreateAddr(const FString& IP, int32 Port)
{
	FIPv4Address ParsedIP;
	FIPv4Address::Parse(IP, ParsedIP);
	TSharedPtr<FInternetAddr> Addr = GetSocketSubsystem()->CreateInternetAddr();
	Addr->SetIp(ParsedIP.Value);
	Addr->SetPort(Port);
	return Addr;
}

TSharedPtr<FInternetAddr> FThreadWorkerBase::CreateBindAddr(int32 Port)
{
	TSharedPtr<FInternetAddr> Addr = GetSocketSubsystem()->CreateInternetAddr();
	Addr->SetAnyAddress();
	Addr->SetPort(Port);
	return Addr;
}

void FThreadWorkerBase::DestroySocket(FSocket*& Socket)
{
	if (!Socket) {
		return;
	}
	Socket->Close();
	GetSocketSubsystem()->DestroySocket(Socket);
	Socket = nullptr;
}
