// Fill out your copyright notice in the Description page of Project Settings.

#include "MavLinkTCPWorker.h"

FMavLinkTCPWorker::FMavLinkTCPWorker(const FVehicleNetworkConfig& Config, FNetworkChannels* InNetworkChannels)
	: Config(Config), NetworkChannels(InNetworkChannels)
{
	StopTaskCounter.Reset();
	SleepSeconds = 0.005f;
}

FMavLinkTCPWorker::~FMavLinkTCPWorker()
{
	Stop();
}

bool FMavLinkTCPWorker::Init()
{
	Socket = CreateSocket(NAME_Stream, TEXT("SITL_TCP"));
	if (!Socket) {
		return false;
	}

	Socket->SetNonBlocking(false); // blocking for connect

	TSharedPtr<FInternetAddr> RemoteAddr = CreateAddr(Config.IPAdress, Config.MavlinkTCPPort);

	bIsConnected = Socket->Connect(*RemoteAddr);
	if (!bIsConnected) {
		UE_LOG(LogTemp, Error, TEXT("[TCPWorker::Init] Handshake failed for Vehicle %d"), Config.VehicleId);
		DestroySocket(Socket);
		return false;
	}

	Socket->SetNonBlocking(true);
	RecvBuf.SetNumUninitialized(512);
	return true;
}

void FMavLinkTCPWorker::Stop()
{
	FMavLinkWorkerBase::Stop();
	bIsConnected = false;
	DestroySocket(Socket);
}

void FMavLinkTCPWorker::DrainOutbound()
{
	mavlink_message_t Msg;
	uint8			  TxBuf[MAVLINK_MAX_PACKET_LEN];
	while (NetworkChannels->OutboundMavTCP.Dequeue(Msg)) {
		uint16 Len = mavlink_msg_to_send_buffer(TxBuf, &Msg);
		int32  Sent = 0;
		if (!Socket->Send(TxBuf, Len, Sent)) {
			bIsConnected = false;
			return;
		}
	}
}

void FMavLinkTCPWorker::ReceiveInbound()
{
	int32 Read = 0;
	if (!Socket->Recv(RecvBuf.GetData(), RecvBuf.Num(), Read)) {
		ESocketErrors Err = GetSocketSubsystem()->GetLastErrorCode();
		if (Err != SE_EWOULDBLOCK) {
			bIsConnected = false;
		}
		return;
	}

	mavlink_message_t Msg;
	for (int32 i = 0; i < Read; ++i) {
		if (mavlink_parse_char(MAVLINK_COMM_0, RecvBuf[i], &Msg, &ParseStatus)) {
			UE_LOG(LogTemp, Warning, TEXT("[FMavLinkTCPWorker::ReceiveInbound] Message Exist"));
			NetworkChannels->InboundMavTCP.Enqueue(Msg);
		}
	}
}

void FMavLinkTCPWorker::OnAfterLoop()
{
	UE_LOG(LogTemp, Warning, TEXT("[TCPWorker::OnAfterLoop] Vehicle %d connection closed."), Config.VehicleId);
}
