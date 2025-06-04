#pragma once

#include "Packet.h"
#include "PacketManager.h"
#include "UserManager.h"
#include "PartyManager.h"
#include "MySQLManager.h"
#include "ErrorCode.h"

#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>
#include <functional>

class UserManager;
class PartyManager;
class MySQLManager;

class PacketManager {

public:

	PacketManager() = default;
	~PacketManager() = default;

	bool Run();

	void Init(const UINT32 maxClient_);

	void PushSystemPacket(PacketInfo packet_);

	void ReceivePacketData(const UINT32 clientIndex_, const UINT32 size_, char* pData_);

	void End();

	std::function<void(UINT32, UINT32, char*)> SendPacketFunc;

private:

	void CreateCompent(const UINT32 maxClient_);

	void EnqueuePacketData(const UINT32 clientIndex_);

	PacketInfo DequePacketData();

	PacketInfo DequeSystemPacketData();

	UserManager* userManager;
	PartyManager* partyManager;
	MySQLManager* mySQLManager;

	typedef void(PacketManager::* PROCESS_RECV_PACKET_FUNCTION)(UINT32, UINT16, char*);
	std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION> RecvFuntionDictionary;

	void ProcessPacket();

	void ProcessRecvPacket(const UINT32 clientIndex_, const UINT16 packetId_, const UINT16 packetSize_, char* pPacket_);

	bool RunProcessThread = false;

	std::thread ProcessThread;

	std::mutex packetLock;
	std::mutex partyLock;

	std::deque<UINT32> UserPacketIndex;
	std::deque<PacketInfo> SystemPacketQueue;

	//==== <네트워크> ====

	// 로그인 요청
	void Login(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// 접속
	void UserConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// 접속종료
	void UserDisConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// 로그아웃 요청 (로그아웃 한다고 접속종료는 아님)
	void Logout(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// 유저 검색
	void FindUserById(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);


	//==== <친구> ====
	void FindUserFriends(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	void FindUserFriendsInfo(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	void FriendRequest(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	//친구요청 취소
	void FriendRequestCancel(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	//친구 삭제
	void DeleteFriend(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	//==== <파티> ====

	// 파티 생성
	void MakeParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// 파티 참가하기 (인원만 안차있으면 다른 유저가 마음대로 어떤 파티로 참가하기)
	void EnterParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);


	// 파티 초대 (초대를 하고 요청하거나 받기)
	void PartyInvite(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// 파티 나가기
	void LeaveParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// 파티 메시지 보내기
	void PartyChatMessage(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);


	// 귓속말 보내기
	void Whisper(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);



};