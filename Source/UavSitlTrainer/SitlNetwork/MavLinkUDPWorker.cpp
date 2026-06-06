// Fill out your copyright notice in the Description page of Project Settings.

#include "MavLinkUDPWorker.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"

FMavLinkUDPWorker::FMavLinkUDPWorker(FVehicleNetworkConfig Config, mavlink_callback_t OnReceivedCallback)
	: Config(Config), OnMavlinkReceivedCallback(OnReceivedCallback)
{
	StopTaskCounter.Reset();
}

FMavLinkUDPWorker::~FMavLinkUDPWorker()
{
	Stop();
}

bool FMavLinkUDPWorker::Init()
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem) {
		UE_LOG(LogTemp, Error, TEXT("[MavLinkUDPWorker::Init] Failed to get socket subsystem"));
		return false;
	}

	MavlinkUDPSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("SITL_Mavlink_UDP_Socket"), true);
	if (!MavlinkUDPSocket) {
		UE_LOG(LogTemp, Error, TEXT("[MavLinkUDPWorker::Init] Failed to create Mavlink socket"));
		return false;
	}
	MavlinkUDPSocket->SetNonBlocking(true);
	MavlinkUDPSocket->SetReuseAddr(true);

	FIPv4Address TargetIP;
	FIPv4Address::Parse(Config.IPAdress, TargetIP);
	RemoteAddr = SocketSubsystem->CreateInternetAddr();
	RemoteAddr->SetIp(TargetIP.Value);
	RemoteAddr->SetPort(Config.MavlinkUDPPort);

	TSharedPtr<FInternetAddr> ListenAddr = SocketSubsystem->CreateInternetAddr();
	ListenAddr->SetAnyAddress();
	ListenAddr->SetPort(Config.MavlinkUDPPort);
	MavlinkUDPSocket->Bind(*ListenAddr);
	return MavlinkUDPSocket != nullptr;
}

uint32 FMavLinkUDPWorker::Run()
{
	if (!MavlinkUDPSocket) {
		UE_LOG(LogTemp, Error, TEXT("[MavLinkUDPWorker::Run] Mavlink socket is not initialized. Exiting thread."));
		return 1; // Exit with error code
	}

	TArray<uint8> ReceiveBuffer;
	ReceiveBuffer.SetNumUninitialized(2096);
	uint8 TxBuffer[MAVLINK_MAX_PACKET_LEN];
	int32 InitialCounterValue = StopTaskCounter.GetValue();
	checkf(!InitialCounterValue, TEXT("[MavLinkUDPWorker::Run] Assert Failed: StopTaskCounter initialized with a dirty state! Value: %d"), InitialCounterValue);

	while (StopTaskCounter.GetValue() == 0) {
		mavlink_message_t OutMessage;
		while (OutboundMavlinkUDPQueue.Dequeue(OutMessage)) {
			uint16 Lenght = mavlink_msg_to_send_buffer(TxBuffer, &OutMessage);
			int32  BytesSent = 0;
			UE_LOG(LogTemp, Warning, TEXT("[FMavLinkUDPWorker::Run] OutboundMavlinkUDPQueue Dequeue"));
			MavlinkUDPSocket->SendTo(TxBuffer, Lenght, BytesSent, *RemoteAddr);
		}
		uint32 PendingDataSize = 0;
		if (MavlinkUDPSocket->HasPendingData(PendingDataSize)) {
			int32					  BytesRead = 0;
			TSharedPtr<FInternetAddr> SendAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
			FMemory::Memzero(ReceiveBuffer.GetData(), ReceiveBuffer.Num());
			if (MavlinkUDPSocket->RecvFrom(ReceiveBuffer.GetData(), ReceiveBuffer.Num(), BytesRead, *SendAddr)) {
				mavlink_status_t  LocalMavStatus;
				mavlink_message_t LocalMavMsg;
				FMemory::Memzero(&LocalMavStatus, sizeof(mavlink_status_t));
				FMemory::Memzero(&LocalMavMsg, sizeof(mavlink_message_t));
				UE_LOG(LogTemp, Warning, TEXT("[UDP] HasPendingData сказав ТАК. Реально прочитано байт (BytesRead): %d"), BytesRead);

				for (int32 i = 0; i < BytesRead; ++i) {
					if (mavlink_parse_char(MAVLINK_COMM_0, ReceiveBuffer[i], &LocalMavMsg, &LocalMavStatus) && OnMavlinkReceivedCallback) {
						// TODO find another solution Сюди код дійде ТІЛЬКИ тоді, коли прилетить пакет від справжнього ArduPilot (Sys ID 1)
						if (LocalMavMsg.sysid == GCS_SYSTEM_ID) {
							continue;
						}
						UE_LOG(LogTemp, Warning, TEXT("[MavLink] ОТРИМАНО СТРУКТУРНИЙ ПАКЕТ ВІД БПЛА! Msg ID: %d | Sys ID: %d"),
							LocalMavMsg.msgid,
							LocalMavMsg.sysid);
						OnMavlinkReceivedCallback(LocalMavMsg);
					}
				}
			}
		}

		FPlatformProcess::Sleep(0.001f);
	}
	return 0;
}

void FMavLinkUDPWorker::Stop()
{
	StopTaskCounter.Increment();
	if (MavlinkUDPSocket) {
		MavlinkUDPSocket->Close();
		if (auto SubSys = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)) {
			SubSys->DestroySocket(MavlinkUDPSocket);
		}
		MavlinkUDPSocket = nullptr;
	}
}