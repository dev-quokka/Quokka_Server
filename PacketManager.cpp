#include "PacketManager.h"
#include "UserManager.h"
#include "PartyManager.h"
#include "MySQLManager.h"
#include "ErrorCode.h"

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

	RecvFuntionDictionary[(int)PACKET_ID::FIND_USER_REQUEST] = &PacketManager::FindUserById;
	
	RecvFuntionDictionary[(int)PACKET_ID::FRIEND_REQUEST] = &PacketManager::FriendRequest;
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

	auto pUser = userManager->GetUserByIdx(userIndex);
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
	auto pUser = userManager->GetUserByIdx(clientIndex_);
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

	int LoginDBResult = mySQLManager->MysqlLoginCheck(UserId,UserPassword);

	LOGIN_RESPONSE_PACKET LoginResPacket;
	LoginResPacket.PacketId = (UINT16)PACKET_ID::LOGIN_RESPONSE;
	LoginResPacket.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);

	// 유저가 가득 차 있을때
	if (userManager->GetCurrentUserCnt() >= userManager->GetUserMaxCnt())
	{
		//접속자수가 최대수를 차지해서 접속불가
		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_FULL;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		return;
	}


	// 아이디가 다를때
	if (LoginDBResult == -1) {
		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_INVALID_ID;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		return;
	}

	// 비밀번호가 다를때
	else if (LoginDBResult ==  -2) {
		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_INVALID_PW;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		return;
	}

	else {
		userManager->AddUser(clientIndex_, LoginDBResult, UserId);

		// 이미 유저가 로그인 중 일때
		if (userManager->FindUserByPK(LoginDBResult)!=-1) {
			//접속중인 유저여서 실패를 반환.
			LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_ALREADY;
			SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
			return;
		}

		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::NONE;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		std::cout << "유저 " << UserId << " 접속" << std::endl;

		// 그 유저 친구목록 뿌려주고 친구인 아이들에게 접속했다는 메시지 보내주기
		return;
	}
}

void PacketManager::FindUserById(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FindUserPacket = reinterpret_cast<FIND_USER_REQUEST*>(pPacket_);
	auto UserId = FindUserPacket->UserID;

	UINT32 LoginResult = mySQLManager->FindUserById(UserId);

	FIND_USER_RESPONSE FindUserRes;

	FindUserRes.PacketId = (UINT16)PACKET_ID::FIND_USER_RESPONSE;
	FindUserRes.PacketLength = sizeof(FIND_USER_RESPONSE);

	// 그 아이디 친구 없을 때
	if (LoginResult==-1) {
		FindUserRes.userPKNum = (UINT16)ERROR_CODE::FIND_NO_USER;
		SendPacketFunc(clientIndex_, sizeof(FIND_USER_RESPONSE), (char*)&FindUserRes);
		return;
	}

	FindUserRes.userPKNum = LoginResult;
	SendPacketFunc(clientIndex_, sizeof(FIND_USER_RESPONSE), (char*)&FindUserRes);
}

void PacketManager::FindUserFriends(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FindFriendsPacket = reinterpret_cast<FIND_FRIENDS_REQUEST*>(pPacket_);
	auto UserPKNum = FindFriendsPacket->userPKNum;

	std::vector<FriendInfo> FriendsInfo = mySQLManager->FindUserFriends(UserPKNum);
	auto FindFriendsInfo = reinterpret_cast<char*>(&FriendsInfo);

	SendPacketFunc(clientIndex_, sizeof(FindFriendsInfo), FindFriendsInfo);
}

void PacketManager::FindUserFriendsInfo(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FindFriendsPacket = reinterpret_cast<FIND_FRIENDS_REQUEST*>(pPacket_);
	auto UserPKNum = FindFriendsPacket->userPKNum;

	std::vector<int> Friends = userManager->GetUserByIdx(clientIndex_)->GetUserFriendsPKNums();
	

}

void PacketManager::FriendRequest(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FriendReqPacket = reinterpret_cast<FRIEND_REQUEST_REQUEST*>(pPacket_);
	auto FriendReqResult = mySQLManager->FriendRequest(FriendReqPacket->reqUserPKNum, FriendReqPacket->resUserPKNum);

	FRIEND_REQUEST_RESPONSE FriendsReq_Res;
	FriendsReq_Res.FriendsReq_Res = (UINT16)FriendReqResult;
	SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&FriendsReq_Res);
}

void PacketManager::FriendRequestCancel(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FriendReqCancelPacket = reinterpret_cast<FRIEND_REQUEST_CANCEL_REQUEST*>(pPacket_);
	auto FriendReqCancelResult = mySQLManager->FriendRequestCancel(FriendReqCancelPacket->reqUserPKNum, FriendReqCancelPacket->resUserPKNum);

	FRIEND_REQUEST_CANCEL_RESPONSE FriendsReqCancel_Res;
	FriendsReqCancel_Res.FriendsReqCancel_Res = (UINT16)FriendReqCancelResult;
	SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&FriendsReqCancel_Res);
}

void PacketManager::DeleteFriend(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto DelFriendPakcet = reinterpret_cast<DELETE_FRIEND_REQUEST*>(pPacket_);
	auto DelFriendResult = mySQLManager->DeleteFriend(DelFriendPakcet->reqUserPKNum, DelFriendPakcet->resUserPKNum);

	DELETE_FRIEND_RESPONSE DelFriend_Res;
	DelFriend_Res.DelFriendRes = (UINT16)DelFriendResult;
	SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&DelFriend_Res);
}

void PacketManager::MakeParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto MakePartyPakcet = reinterpret_cast<MAKE_PARTY_REQUEST*>(pPacket_);
	auto MakePartyResult = mySQLManager->MakeParty(MakePartyPakcet->reqUserPKNum, MakePartyPakcet->resUserPKNum);
	MAKE_PARTY_RESPONSE MakeParty_Res;

	// 파티 생성 실패
	if (MakePartyResult == -1 || MakePartyResult == -2) {
		MakeParty_Res.partyNum = (UINT16)ERROR_CODE::PARTY_MAKE_FAIL;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&MakeParty_Res);
	}

	// 파티 생성 후 그 파티 번호 불러오기 실패 (1.클라한테는 파티 생성 실패 메시지 보내고 INSERT한거 DELETE, 2. 일단 파티 생성 해놓고 다시 번호 요청하기)
	// 이 케이스는 보류
	/*else if (MakePartyResult == -2) {
		MakeParty_Res.partyNum = (UINT16)ERROR_CODE::PARTYNUM_CHECK_FAIL;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&MakeParty_Res);
	}*/

	else {
		MakeParty_Res.partyNum = (UINT16)MakePartyResult;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&MakeParty_Res);
	}
}



