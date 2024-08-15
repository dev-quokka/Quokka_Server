#pragma once

#include "Packet.h"

#include <unordered_map>
#include <thread>
#include <mutex>

class PacketManager {

public:

	void PushSystemPacket(PacketInfo packet_){}

	void ReceivePacketData(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {};

private:

	void Init(const UINT32 maxClient_) {}

	//==== <네트워크> ====

	// 회원가입 요청
	void JoinNewUser(){}

	// 로그인 요청
	void Login(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_){}

	// 접속
	void UserConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {}

	// 접속종료
	void UserDisConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {}

	// 로그아웃 요청
	void Logout(){}


	//==== <파티> ====
	
	// 파티 초대 (초대를 하고 요청하거나 받기)
	void PartyInvite(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_){}

	// 파티 참가하기 (인원만 안차있으면 다른 유저가 마음대로 어떤 파티로 참가하기)
	void EnterParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_){}

	// 파티 나가기
	void LeaveParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_){}

	// 파티 메시지 보내기
	void PartyChatMessage(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_){}



	// 귓속말 보내기
	void Whisper(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {}


private:

	typedef void(PacketManager::* PROCESS_RECV_PACKET_FUNCTION)(UINT32, UINT16, char*);
	std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION> mRecvFuntionDictionary;

};