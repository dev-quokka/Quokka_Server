#include "PacketManager.h"

void PacketManager::Init(const UINT32 maxClient_)
{
	mRecvFuntionDictionary = std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION>();

	mRecvFuntionDictionary[(int)PACKET_ID::SYS_USER_CONNECT] = &PacketManager::UserConnect;
	mRecvFuntionDictionary[(int)PACKET_ID::SYS_USER_DISCONNECT] = &PacketManager::UserDisConnect;

	mRecvFuntionDictionary[(int)PACKET_ID::LOGIN_REQUEST] = &PacketManager::Login;


	mRecvFuntionDictionary[(int)PACKET_ID::PARTY_ENTER_REQUEST] = &PacketManager::EnterParty;
	mRecvFuntionDictionary[(int)PACKET_ID::PARTY_LEAVE_REQUEST] = &PacketManager::LeaveParty;
	mRecvFuntionDictionary[(int)PACKET_ID::PARTY_CHAT_REQUEST] = &PacketManager::PartyChatMessage;

	mRecvFuntionDictionary[(int)PACKET_ID::WHISPER_CHAT_REQUEST] = &PacketManager::Whisper;

}

