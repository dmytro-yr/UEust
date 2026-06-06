// Fill out your copyright notice in the Description page of Project Settings.

#include "MavLinkTCPWorker.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"

FMavLinkTCPWorker::FMavLinkTCPWorker(const FVehicleNetworkConfig InConfig, callback_frame_recieved_t InOnFrameReceived)
	: Config(InConfig), OnTelemetryFrameReceived(InOnFrameReceived)
{
	StopTaskCounter.Reset();
}

bool FMavLinkTCPWorker::Init()
{
	if (MavlinkTCPSocket && bIsConnected) {
		return true;
	}

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem) {
		UE_LOG(LogTemp, Error, TEXT("[MavLinkTCPWorker::Init] Failed to get socket subsystem"))
		return false;
	}
	MavlinkTCPSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("SITL_MavLink_TCP_Socket"), false);
	if (!MavlinkTCPSocket) {
		UE_LOG(LogTemp, Error, TEXT("[MavLinkTCPWorker::Init] Failed to create Mavlink socket"))
		return false;
	}
	MavlinkTCPSocket->SetReuseAddr(true);
	MavlinkTCPSocket->SetNonBlocking(false);

	FIPv4Address InterAddr;
	FIPv4Address::Parse(Config.IPAdress, InterAddr);
	RemoteAddr = SocketSubsystem->CreateInternetAddr();
	RemoteAddr->SetIp(InterAddr.Value);
	RemoteAddr->SetPort(Config.MavlinkTCPPort);

	UE_LOG(LogTemp, Log, TEXT("[MavLinkTCPWorker::Init] Vehicle %d opening TCP tunnel to %s:%d..."),
		Config.VehicleId, *Config.IPAdress, Config.MavlinkTCPPort);

	bIsConnected = MavlinkTCPSocket->Connect(*RemoteAddr);
	if (!bIsConnected) {
		UE_LOG(LogTemp, Error, TEXT("[MavLinkTCPWorker::Init] Vehicle %d TCP Connection refused."), Config.VehicleId);
	}

	MavlinkTCPSocket->SetNonBlocking(true);

	UE_LOG(LogTemp, Log, TEXT("[MavLinkTCPWorker::Init] Vehicle %d connected. Non-blocking data stream engaged."), Config.VehicleId)

	return true;
}

uint32 FMavLinkTCPWorker::Run()
{
	TArray<uint8> ReceiveBuffer;
	ReceiveBuffer.SetNumUninitialized(512);

	mavlink_status_t  Status;
	mavlink_message_t Msg;
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	while (StopTaskCounter.GetValue() == 0 && bIsConnected) {
		int32 BytesRead = 0;
		bool  bReadSuccess = MavlinkTCPSocket->Recv(ReceiveBuffer.GetData(), ReceiveBuffer.Num(), BytesRead);
		if (bReadSuccess && BytesRead > 0) {
			for (int32 i = 0; BytesRead > i; ++i) {
				if (mavlink_parse_char(MAVLINK_COMM_0, ReceiveBuffer[i], &Msg, &Status)) {
					FMavTelemetryFrame Frame;
					Frame.VehicleId = Config.VehicleId;
					Frame.MessageId = Msg.msgid;
					Frame.Payload = Msg;
					if (OnTelemetryFrameReceived) {
						OnTelemetryFrameReceived(Frame);
					}
				}
			}
		}
		else {
			ESocketErrors LastError = SocketSubsystem->GetLastErrorCode();
			if (LastError == SE_EWOULDBLOCK) {
				FPlatformProcess::Sleep(0.01f);
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("[MavLinkTCPWorker::Run] Vehicle %d TCP Socket lost connection"), Config.VehicleId);
				bIsConnected = false;
			}
		}
	}
	return 0;
}

FMavLinkTCPWorker::~FMavLinkTCPWorker()
{
	Stop();
}

void FMavLinkTCPWorker::Stop()
{
	StopTaskCounter.Increment();
	bIsConnected = false;
	if (MavlinkTCPSocket) {
		MavlinkTCPSocket->Close();
		if (auto SubSystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)) {
			SubSystem->DestroySocket(MavlinkTCPSocket);
		}
		MavlinkTCPSocket = nullptr;
	}
}

bool FMavLinkTCPWorker::SendRawBytes(const uint8* Data, int32 Count)
{
	if (!MavlinkTCPSocket || !bIsConnected) {
		UE_LOG(LogTemp, Error, TEXT("[MavLinkTCPWorker::SendRawBytes] Vehicle %d TCP Socket cannott send raw bytes due to connection lost."), Config.VehicleId);
		return false;
	}
	int32 BytesSent = 0;
	return MavlinkTCPSocket->Send(Data, Count, BytesSent);
}
