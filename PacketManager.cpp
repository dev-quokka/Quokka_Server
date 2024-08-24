#include "PacketManager.h"
#include "UserManager.h"
#include "PartyManager.h"
#include "MySQLManager.h"
#include "ErrorCode.h"

void PacketManager::Init(const UINT32 maxClient_)
{
	RecvFuntionDictionary = std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION>();
	;
	RecvFuntionDictionary[(int)PACKET_ID::SYS_USER_CONNECT] = &PacketManager::UserConnect;
	RecvFuntionDictionary[(int)PACKET_ID::SYS_USER_DISCONNECT] = &PacketManager::UserDisConnect;
	RecvFuntionDictionary[(int)PACKET_ID::LOGIN_REQUEST] = &PacketManager::Login;

	// <친구>
	RecvFuntionDictionary[(int)PACKET_ID::FIND_USER_REQUEST] = &PacketManager::FindUserById; // 친구 검색을 위한 유저 검색
	RecvFuntionDictionary[(int)PACKET_ID::FIND_FRIENDS_REQUEST] = &PacketManager::FindUserFriendsInfo;
	RecvFuntionDictionary[(int)PACKET_ID::FRIEND_REQUEST_REQUEST] = &PacketManager::FriendRequest;
	RecvFuntionDictionary[(int)PACKET_ID::FRIEND_REQUEST_CANCEL_REQUEST] = &PacketManager::FriendRequestCancel;
	RecvFuntionDictionary[(int)PACKET_ID::FRIEND_DELETE_REQUEST] = &PacketManager::DeleteFriend;

	// <파티>
	RecvFuntionDictionary[(int)PACKET_ID::MAKE_PARTY_REQUEST] = &PacketManager::MakeParty;
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
		std::lock_guard<std::mutex> guard(packetLock);
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

	std::lock_guard<std::mutex> guard(packetLock);
	if (SystemPacketQueue.empty())
	{
		return PacketInfo();
	}

	PacketInfo packetData = SystemPacketQueue.front();
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

void PacketManager::EnqueuePacketData(const UINT32 clientIndex_)
{
	std::lock_guard<std::mutex> guard(packetLock);
	UserPacketIndex.push_back(clientIndex_);
}

void PacketManager::PushSystemPacket(PacketInfo packet_)
{
	std::lock_guard<std::mutex> guard(packetLock);
	SystemPacketQueue.push_back(packet_);
}

void PacketManager::ReceivePacketData(const UINT32 clientIndex_, const UINT32 size_, char* pData_) {
	auto pUser = userManager->GetUserByIdx(clientIndex_);
	pUser->SetPacketData(size_, pData_);

	EnqueuePacketData(clientIndex_);
};

void PacketManager::UserConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	printf("[ProcessUserConnect] clientIndex: %d\n", clientIndex_);
	auto User = userManager->GetUserByIdx(clientIndex_);
	User->Clear();
}

void PacketManager::UserDisConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	printf("[ProcessUserDisConnect] clientIndex: %d\n", clientIndex_);
	ClearConnectionInfo(clientIndex_);
}

void PacketManager::Login(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	if (LOGIN_REQUEST_PACKET_SIZE != packetSize_)
	{
		return;
	}

	auto LoginReqPacket = reinterpret_cast<LOGIN_REQUEST_PACKET*>(pPacket_);

	auto UserId = LoginReqPacket->UserID;
	auto UserPassword = LoginReqPacket->UserPW;

	auto LoginDBResult = mySQLManager->MysqlLoginCheck(UserId,UserPassword);

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
	if (LoginDBResult->Check == -1) {
		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_INVALID_ID;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		return;
	}

	// 비밀번호가 다를때
	else if (LoginDBResult->Check ==  -2) {
		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_INVALID_PW;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		return;
	}

	else {
		// 이미 유저가 로그인 중 일때
		if (userManager->FindUserByPK(LoginDBResult->userPkNum)!=-1) {
			//접속중인 유저여서 실패를 반환.
			LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_ALREADY;
			SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
			return;
		}

		userManager->AddUser(clientIndex_, LoginDBResult->userPkNum, UserId);

		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::NONE;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		std::cout << "유저 " << UserId << " 접속" << std::endl;

		// 그 유저 친구목록 뿌려주고
		std::vector<FriendInfo*> FriendsInfo = mySQLManager->FindUserFriendsInfo(LoginDBResult->userPkNum);

		// 로그인 한 클라이언트 패킷에서 -1이면 로그아웃상태 1이면 로그인 상태
		for (int i = 0; i < FriendsInfo.size(); i++) {
			if (userManager->FindUserByPK(FriendsInfo[i]->userPkNum) == -1) {
				FriendsInfo[i]->Check = -1;
			}
		}
		auto FindFriendsInfo = reinterpret_cast<char*>(&FriendsInfo);
		FIND_FRIENDS_RESPONSE FindFriendsRes;
		FindFriendsRes.PacketId = (UINT16)PACKET_ID::FIND_FRIENDS_RESPONSE;
		FindFriendsRes.PacketLength = sizeof(FIND_FRIENDS_RESPONSE);
		FindFriendsRes.FriendsInfo = FindFriendsInfo;
		SendPacketFunc(clientIndex_, sizeof(FindFriendsRes), (char*)&FindFriendsRes);

		//친구인 아이들에게 접속했다는 메시지 보내주기
		if (LoginDBResult->Check == 1) {
			for (int i = 0; i < FriendsInfo.size(); i++) {

				if (userManager->FindUserByPK(FriendsInfo[i]->userPkNum) == -1) continue;

				else {
					CONNECT_RESPONSE_TO_FRIENDS ConnResToFriend;
					ConnResToFriend.PacketId = (UINT16)PACKET_ID::CONNECT_RESPONSE_TO_FRIENDS;
					ConnResToFriend.PacketLength = sizeof(CONNECT_RESPONSE_TO_FRIENDS);
					ConnResToFriend.reqUserPKNum = LoginDBResult->userPkNum;
					SendPacketFunc(userManager->FindUserByPK(FriendsInfo[i]->userPkNum), sizeof(CONNECT_RESPONSE_TO_FRIENDS), (char*)&ConnResToFriend);
				}
			}
		}
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

//void PacketManager::FindUserFriends(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {
//
//	auto FindFriendsPacket = reinterpret_cast<FIND_FRIENDS_REQUEST*>(pPacket_);
//	auto UserPKNum = FindFriendsPacket->userPKNum;
//
//	std::vector<FriendInfo> FriendsInfo = mySQLManager->FindUserFriendsInfo(UserPKNum);
//	auto FindFriendsInfo = reinterpret_cast<char*>(&FriendsInfo);
//
//	SendPacketFunc(clientIndex_, sizeof(FindFriendsInfo), FindFriendsInfo);
//}

void PacketManager::FindUserFriendsInfo(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FindFriendsPacket = reinterpret_cast<FIND_FRIENDS_REQUEST*>(pPacket_);
	auto UserPKNum = FindFriendsPacket->userPKNum;

	std::vector<FriendInfo*> FriendsInfo = mySQLManager->FindUserFriendsInfo(UserPKNum);
	auto FindFriendsInfo = reinterpret_cast<char*>(&FriendsInfo);

	FIND_FRIENDS_RESPONSE FindFriendsRes;
	FindFriendsRes.PacketId = (UINT16)PACKET_ID::FIND_FRIENDS_RESPONSE;
	FindFriendsRes.PacketLength = sizeof(FIND_FRIENDS_RESPONSE);
	FindFriendsRes.FriendsInfo = FindFriendsInfo;
	SendPacketFunc(clientIndex_, sizeof(FindFriendsInfo), (char*)&FindFriendsRes);

}

void PacketManager::FriendRequest(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FriendReqPacket = reinterpret_cast<FRIEND_REQUEST_REQUEST*>(pPacket_);
	auto FriendReqResult = mySQLManager->FriendRequest(FriendReqPacket->reqUserPKNum, FriendReqPacket->resUserPKNum);

	FRIEND_REQUEST_RESPONSE FriendsReq_Res;
	FriendsReq_Res.PacketId = (UINT16)PACKET_ID::FRIEND_REQUEST_RESPONSE;
	FriendsReq_Res.PacketLength = sizeof(FRIEND_REQUEST_RESPONSE);
	FriendsReq_Res.FriendsReq_Res = (UINT16)FriendReqResult;

	SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_RESPONSE), (char*)&FriendsReq_Res);
}

void PacketManager::FriendRequestCancel(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FriendReqCancelPacket = reinterpret_cast<FRIEND_REQUEST_CANCEL_REQUEST*>(pPacket_);
	auto FriendReqCancelResult = mySQLManager->FriendRequestCancel(FriendReqCancelPacket->reqUserPKNum, FriendReqCancelPacket->resUserPKNum);

	FRIEND_REQUEST_CANCEL_RESPONSE FriendsReqCancel_Res;
	FriendsReqCancel_Res.PacketId = (UINT16)PACKET_ID::FRIEND_REQUEST_CANCEL_RESPONSE;
	FriendsReqCancel_Res.PacketLength = sizeof(FRIEND_REQUEST_CANCEL_RESPONSE);
	FriendsReqCancel_Res.FriendsReqCancel_Res = (UINT16)FriendReqCancelResult;
	SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_CANCEL_RESPONSE), (char*)&FriendsReqCancel_Res);
}

void PacketManager::DeleteFriend(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto DelFriendPakcet = reinterpret_cast<DELETE_FRIEND_REQUEST*>(pPacket_);
	auto DelFriendResult = mySQLManager->DeleteFriend(DelFriendPakcet->reqUserPKNum, DelFriendPakcet->resUserPKNum);

	DELETE_FRIEND_RESPONSE DelFriend_Res;
	DelFriend_Res.PacketId = (UINT16)PACKET_ID::FRIEND_DELETE_RESPONSE;
	DelFriend_Res.PacketLength = sizeof(DELETE_FRIEND_RESPONSE);
	DelFriend_Res.DelFriendRes = (UINT16)DelFriendResult;

	if (DelFriendResult==ERROR_CODE::FRIEND_DELETE_FAIL) {
		SendPacketFunc(clientIndex_, sizeof(DELETE_FRIEND_RESPONSE), (char*)&DelFriend_Res);
	}
	else if(DelFriendResult==ERROR_CODE::NONE){
		SendPacketFunc(clientIndex_, sizeof(DELETE_FRIEND_RESPONSE), (char*)&DelFriend_Res);
		auto DelResFriend = DelFriendPakcet->resUserPKNum;

		//친구 삭제 당한 유저한테도 삭제 메시지 보내기
		if (userManager->FindUserByPK(DelResFriend) != -1) {
			DELETE_FRIEND_RESPONSE_TO_RESPONSE_USER DelToResFriend;
			DelToResFriend.PacketId = (UINT16)PACKET_ID::DELETE_FRIEND_RESPONSE_TO_RESPONSE_USER;
			DelToResFriend.PacketLength = sizeof(DELETE_FRIEND_RESPONSE_TO_RESPONSE_USER);
			DelToResFriend.reqUserPKNum = DelFriendPakcet->reqUserPKNum;
			SendPacketFunc(userManager->FindUserByPK(DelResFriend), sizeof(DELETE_FRIEND_RESPONSE_TO_RESPONSE_USER), (char*)&DelToResFriend);
		}
	}
}

void PacketManager::EnterParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	std::lock_guard<std::mutex> pguard(partyLock);
	auto EnterPartyPacket = reinterpret_cast<PARTY_ENTER_REQUEST*>(pPacket_);
	auto ReqUserPKNum = EnterPartyPacket->reqUserPKNum;
	auto PartyIdx = EnterPartyPacket->partyIdx;

	PARTY_ENTER_RESPONSE PartyEnterRes;
	PartyEnterRes.PacketId = (UINT16)PACKET_ID::PARTY_ENTER_RESPONSE;
	PartyEnterRes.PacketLength = sizeof(PARTY_ENTER_RESPONSE);

	//들어갈 수 있으면 이 함수 실행
	if (partyManager->UsableEnterCheck(PartyIdx)!=0) {
		PartyEnterRes.partyRes = (UINT16)mySQLManager->EnterParty(ReqUserPKNum, PartyIdx, partyManager->UsableEnterCheck(PartyIdx));
		SendPacketFunc(clientIndex_, sizeof(PARTY_ENTER_RESPONSE), (char*)&PartyEnterRes);
	}
	else{
		PartyEnterRes.partyRes = (UINT16)ERROR_CODE::PARTY_FULL;
		SendPacketFunc(clientIndex_, sizeof(PARTY_ENTER_RESPONSE), (char*)&PartyEnterRes);
	}
}

void PacketManager::MakeParty(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	std::lock_guard<std::mutex> pguard(partyLock);
	INT16 makePartyCheck = partyManager->MakePartyCheck();

	if (makePartyCheck == -1) {

		MAKE_PARTY_RESPONSE MakeParty_Res;
		MakeParty_Res.PacketId = (UINT16)PACKET_ID::MAKE_PARTY_REQUEST;
		MakeParty_Res.PacketLength = sizeof(MAKE_PARTY_RESPONSE);
		MakeParty_Res.partyRes = (UINT16)ERROR_CODE::PARTY_FULL;

		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&MakeParty_Res);
	}

	else {

		auto MakePartyPakcet = reinterpret_cast<MAKE_PARTY_REQUEST*>(pPacket_);
		auto MakePartyResult = mySQLManager->MakeParty(makePartyCheck, MakePartyPakcet->reqUserPKNum, MakePartyPakcet->resUserPKNum);

		MAKE_PARTY_RESPONSE MakeParty_Res;
		MakeParty_Res.PacketId = (UINT16)PACKET_ID::MAKE_PARTY_REQUEST;
		MakeParty_Res.PacketLength = sizeof(MAKE_PARTY_RESPONSE);

		if (MakePartyResult == ERROR_CODE::NONE) {
			MakeParty_Res.partyRes = (UINT16)MakePartyResult;
			SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&MakeParty_Res);
			partyManager->MakeParty(makePartyCheck, MakePartyPakcet->reqUserPKNum, MakePartyPakcet->resUserPKNum);
		}

		else {
			MakeParty_Res.partyRes = (UINT16)ERROR_CODE::PARTY_MAKE_FAIL;
			SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&MakeParty_Res);
		}
	}

}



