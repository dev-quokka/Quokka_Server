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

	//==== <��Ʈ��ũ> ====

	// �α��� ��û
	void Login(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// ����
	void UserConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// ��������
	void UserDisConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// �α׾ƿ� ��û (�α׾ƿ� �Ѵٰ� ��������� �ƴ�)
	void Logout(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// ���� �˻�
	void FindUserById(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);


	//==== <ģ��> ====
	void FindUserFriends(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	void FindUserFriendsInfo(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	void FriendRequest(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	//ģ����û ���
	void FriendRequestCancel(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	//ģ�� ����
	void DeleteFriend(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	//==== <��Ƽ> ====

	// ��Ƽ ����
	void MakeParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// ��Ƽ �����ϱ� (�ο��� ���������� �ٸ� ������ ������� � ��Ƽ�� �����ϱ�)
	void EnterParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);


	// ��Ƽ �ʴ� (�ʴ븦 �ϰ� ��û�ϰų� �ޱ�)
	void PartyInvite(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// ��Ƽ ������
	void LeaveParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	// ��Ƽ �޽��� ������
	void PartyChatMessage(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);


	// �ӼӸ� ������
	void Whisper(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);



};