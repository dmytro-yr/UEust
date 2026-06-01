// Fill out your copyright notice in the Description page of Project Settings.

#include "PhysicsUDPWorker.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"

FPhysicsUDPWorker::FPhysicsUDPWorker(FVehicleNetworkConfig InConfig, callback_t OnReceivedCallback)
	: Config(InConfig), OnDataReceivedCallback(OnReceivedCallback)
{
	StopTaskCounter.Reset();
}

FPhysicsUDPWorker::~FPhysicsUDPWorker()
{
	Stop();
}

bool FPhysicsUDPWorker::Init()
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get socket subsystem"));
		return false;
	}

	TxSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("SITL_Tx_Socket"), true);
	if (!TxSocket) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create Tx socket"));
		return false;
	}
	TxSocket->SetNonBlocking(true);
	TxSocket->SetReuseAddr(true);

	FIPv4Address TargetIP;
	FIPv4Address::Parse(Config.IPAdress, TargetIP);
	RemoteAddr = SocketSubsystem->CreateInternetAddr();
	RemoteAddr->SetIp(TargetIP.Value);
	RemoteAddr->SetPort(Config.PhysicsTxPort);

	RxSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("SITL_Rx_Socket"), true);
	if (!RxSocket) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create Rx socket"))
		return false;
	}
	RxSocket->SetNonBlocking(true);
	RxSocket->SetReuseAddr(true);

	TSharedPtr<FInternetAddr> ListenAddr = SocketSubsystem->CreateInternetAddr();
	ListenAddr->SetAnyAddress();
	ListenAddr->SetPort(Config.PhysicsRxPort);
	RxSocket->Bind(*ListenAddr);
	return TxSocket && RxSocket;
}

uint32 FPhysicsUDPWorker::Run()
{
	TArray<uint8> ReceiveBuffer;
	ReceiveBuffer.SetNumUninitialized(4096);

	while (StopTaskCounter.GetValue() == 0) {
		// UE5 to AP
		FString OutboundJson;
		while (OutboundPhysicsQueue.Dequeue(OutboundJson)) {
			FTCHARToUTF8 Converter(*OutboundJson);
			int32		 BytesSent = 0; // todo ask how it used
			TxSocket->SendTo((uint8*)Converter.Get(), Converter.Length(), BytesSent, *RemoteAddr);
		}
		// AP to UE5
		uint32 PendingDataSize = 0;
		while (RxSocket->HasPendingData(PendingDataSize)) {
			int32					  BytesRead = 0;
			TSharedRef<FInternetAddr> SenderAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

			if (RxSocket->RecvFrom(ReceiveBuffer.GetData(), ReceiveBuffer.Num(), BytesRead, *SenderAddr)) {
				if (BytesRead > 0) {
					TArray<uint8> PayloadData;
					PayloadData.Append(ReceiveBuffer.GetData(), BytesRead);
					InboundActuatorQueue.Enqueue(PayloadData);
					if (OnDataReceivedCallback) {
						OnDataReceivedCallback(PayloadData);
					}
				}
			}
			FPlatformProcess::Sleep(0.001f);
		}
	}
	return 0;
}

void FPhysicsUDPWorker::Stop()
{
	StopTaskCounter.Increment();

	if (TxSocket) {
		TxSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(TxSocket);
		TxSocket = nullptr;
	}
	if (RxSocket) {
		RxSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(RxSocket);
		RxSocket = nullptr;
	}
}