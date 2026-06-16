#include "MavLinkWorkerBase.h"

uint32 FMavLinkWorkerBase::Run()
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

FSocket* FMavLinkWorkerBase::CreateSocket(FName Type, const FString& Name)
{
	FSocket* Socket = GetSocketSubsystem()->CreateSocket(Type, Name, true);
	if (Socket) {
		Socket->SetNonBlocking(true);
		Socket->SetReuseAddr(true);
	}
	return Socket;
}

TSharedPtr<FInternetAddr> FMavLinkWorkerBase::CreateAddr(const FString& IP, int32 Port)
{
	FIPv4Address ParsedIP;
	FIPv4Address::Parse(IP, ParsedIP);
	TSharedPtr<FInternetAddr> Addr = GetSocketSubsystem()->CreateInternetAddr();
	Addr->SetIp(ParsedIP.Value);
	Addr->SetPort(Port);
	return Addr;
}

TSharedPtr<FInternetAddr> FMavLinkWorkerBase::CreateBindAddr(int32 Port)
{
	TSharedPtr<FInternetAddr> Addr = GetSocketSubsystem()->CreateInternetAddr();
	Addr->SetAnyAddress();
	Addr->SetPort(Port);
	return Addr;
}

void FMavLinkWorkerBase::DestroySocket(FSocket*& Socket)
{
	if (!Socket) {
		return;
	}
	Socket->Close();
	GetSocketSubsystem()->DestroySocket(Socket);
	Socket = nullptr;
}
