#include "PacketManager.h"
#include "UserManager.h"
#include "PartyManager.h"
#include "MySQLManager.h"

void PacketManager::Init(const UINT32 maxClient_)
{
	RecvFuntionDictionary = std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION>();

	RecvFuntionDictionary[(int)PACKET_ID::SYS_USER_CONNECT] = &PacketManager::UserConnect;
	RecvFuntionDictionary[(int)PACKET_ID::SYS_USER_DISCONNECT] = &PacketManager::UserDisConnect;

	RecvFuntionDictionary[(int)PACKET_ID::LOGIN_REQUEST] = &PacketManager::Login;

	RecvFuntionDictionary[(int)PACKET_ID::PARTY_ENTER_REQUEST] = &PacketManager::EnterParty;
	RecvFuntionDictionary[(int)PACKET_ID::PARTY_LEAVE_REQUEST] = &PacketManager::LeaveParty;
	RecvFuntionDictionary[(int)PACKET_ID::PARTY_CHAT_REQUEST] = &PacketManager::PartyChatMessage;

	RecvFuntionDictionary[(int)PACKET_ID::WHISPER_CHAT_REQUEST] = &PacketManager::Whisper;
	
}

bool PacketManager::Run()
{
	// 여기에서 데이터 베이스 시작하자
	mySQLManager->Run();

	//이 부분을 패킷 처리 부분으로 이동 시킨다.
	RunProcessThread = true;
	ProcessThread = std::thread([this]() { ProcessPacket(); });

	return true;
}

void PacketManager::ProcessRecvPacket(const UINT32 clientIndex_, const UINT16 packetId_, const UINT16 packetSize_, char* pPacket_)
{
	auto iter = RecvFuntionDictionary.find(packetId_);
	if (iter != RecvFuntionDictionary.end())
	{
		(this->*(iter->second))(clientIndex_, packetSize_, pPacket_);
	}

}

void PacketManager::End()
{

	RunProcessThread = false;

	if (ProcessThread.joinable())
	{
		ProcessThread.join();
	}
}

void EnterParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {
	
};

PacketInfo PacketManager::DequePacketData()
{
	UINT32 userIndex = 0;

	{
		std::lock_guard<std::mutex> guard(dpLock);
		if (UserPacketIndex.empty())
		{
			return PacketInfo();
		}

		userIndex = UserPacketIndex.front();
		UserPacketIndex.pop_front();
	}

	auto pUser = userManager->GetUserByConnIdx(userIndex);
	auto packetData = pUser->GetPacket();
	packetData.ClientIndex = userIndex;
	return packetData;
}

PacketInfo PacketManager::DequeSystemPacketData()
{

	std::lock_guard<std::mutex> guard(dpLock);
	if (SystemPacketQueue.empty())
	{
		return PacketInfo();
	}

	auto packetData = SystemPacketQueue.front();
	SystemPacketQueue.pop_front();

	return packetData;
}

void PacketManager::ProcessPacket()
{
	while (RunProcessThread)
	{
		bool isIdle = true;

		auto packetData = DequePacketData();
		if (packetData.PacketId > (UINT16)PACKET_ID::SYS_END)
		{
			isIdle = false;
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketId, packetData.DataSize, packetData.pDataPtr);
		}

		packetData = DequeSystemPacketData();
		if (packetData.PacketId != 0)
		{
			isIdle = false;
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketId, packetData.DataSize, packetData.pDataPtr);
		}

		if (isIdle)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void PacketManager::ReceivePacketData(const UINT32 clientIndex_, const UINT32 size_, char* pData_) {
	auto pUser = userManager->GetUserByConnIdx(clientIndex_);
	pUser->SetPacketData(size_, pData_);

	EnqueuePacketData(clientIndex_);
};

void PacketManager::Login(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	if (LOGIN_REQUEST_PACKET_SIZE != packetSize_)
	{
		return;
	}

	auto LoginReqPacket = reinterpret_cast<LOGIN_REQUEST_PACKET*>(pPacket_);

	auto UserId = LoginReqPacket->UserID;
	auto UserPassword = LoginReqPacket->UserPW;


}