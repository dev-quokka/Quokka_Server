#include "PacketManager.h"

void PacketManager::Init(const UINT32 maxClient_)
{
	RecvFuntionDictionary = std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION>();
	
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
	RecvFuntionDictionary[(int)PACKET_ID::MAKE_PARTY_REQUEST] = &PacketManager::UserDisConnect;
	RecvFuntionDictionary[(int)PACKET_ID::PARTY_ENTER_REQUEST] = &PacketManager::EnterParty;
	/*RecvFuntionDictionary[(int)PACKET_ID::PARTY_LEAVE_REQUEST] = &PacketManager::LeaveParty;
	RecvFuntionDictionary[(int)PACKET_ID::PARTY_CHAT_REQUEST] = &PacketManager::PartyChatMessage;

	RecvFuntionDictionary[(int)PACKET_ID::WHISPER_CHAT_REQUEST] = &PacketManager::Whisper;*/

	mySQLManager = new MySQLManager;

	CreateCompent(maxClient_);
}

void PacketManager::CreateCompent(const UINT32 maxClient_)
{
	userManager = new UserManager;
	userManager->Init(maxClient_);
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

	std::cout << "패킷 쓰레드 시작" << std::endl;

	while (RunProcessThread)
	{
		bool isIdle = true;

		auto packetData= DequePacketData();
		
		// 유저 접속, 로그아웃을 제외한 모든 패킷 처리 부분
		if (packetData.PacketId > (UINT16)PACKET_ID::SYS_END)
		{
			isIdle = false;
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketId, packetData.DataSize, packetData.pDataPtr);
		}

		// 유저 접속,로그아웃 시스템 패킷 처리 부분
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
	std::cout << "유저 접속 clientIndex : " << clientIndex_ << std::endl;
	auto User = userManager->GetUserByIdx(clientIndex_);
	User->Clear();
}

void PacketManager::UserDisConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	userManager->UserClear(clientIndex_);
	std::cout << "유저 정보 초기화" << std::endl;
	std::cout << " 유저유저 접속 종료 clientIndex : "<< clientIndex_<<std::endl;
}

void PacketManager::Login(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	if (LOGIN_REQUEST_PACKET_SIZE != packetSize_)
	{
		return;
	}

	auto LoginReqPacket = reinterpret_cast<LOGIN_REQUEST_PACKET*>(pPacket_);

	auto UserId = LoginReqPacket->UserID;
	auto UserPassword = LoginReqPacket->UserPW;

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

	auto LoginDBResult = mySQLManager->MysqlLoginCheck(UserId,UserPassword);

	// 아이디가 다를때
	if (LoginDBResult == 1) {
		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_INVALID_ID;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		return;
	}

	// 비밀번호가 다를때
	else if (LoginDBResult == 2) {
		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_INVALID_PW;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		return;
	}

	else {
		// 이미 유저가 로그인 중 일때
		if (userManager->FindUserByPK(LoginDBResult)!=-1) {
			//접속중인 유저여서 실패를 반환.
			LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_ALREADY;
			SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
			return;
		}

		FriendInfo k = mySQLManager->MyInfo(LoginDBResult);
		userManager->AddUser(clientIndex_, &k);

		std::cout << "유저 " << UserId << " 접속" << std::endl;

		// 그 유저 친구목록 뿌려주고
		std::vector<FriendInfo> FriendsInfo = mySQLManager->FindUserFriendsInfo(LoginDBResult);

		LoginResPacket.LoginResult = (UINT16)FriendsInfo.size();
		LoginResPacket.MyInfo = k; //FriendInfo*
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
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

void PacketManager::FindUserFriendsInfo(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FindFriendsPacket = reinterpret_cast<FIND_FRIENDS_REQUEST*>(pPacket_);
	auto UserPKNum = FindFriendsPacket->userPKNum;

	std::vector<FriendInfo> FriendsInfo = mySQLManager->FindUserFriendsInfo(UserPKNum);

	// 로그인 한 클라이언트 패킷에서 -1이면 로그아웃상태 1이면 로그인 상태
		for (int i = 0; i < FriendsInfo.size(); i++) {
			if (userManager->FindUserByPK(FriendsInfo[i].userPkNum) == -1) {
				FriendsInfo[i].Check = -1;
			}
		}

		FIND_FRIENDS_RESPONSE FindFriendsRes;
		FindFriendsRes.PacketId = (UINT16)PACKET_ID::FIND_FRIENDS_RESPONSE;
		FindFriendsRes.PacketLength = sizeof(FIND_FRIENDS_RESPONSE);
		FindFriendsRes.friendInfo = FriendsInfo; 
		SendPacketFunc(clientIndex_, sizeof(FIND_FRIENDS_RESPONSE), (char*)&FindFriendsRes);
}

void PacketManager::FriendRequest(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FriendReqPacket = reinterpret_cast<FRIEND_REQUEST_REQUEST*>(pPacket_);
	auto FriendReqResult = mySQLManager->FriendRequest(FriendReqPacket->reqUserPKNum, FriendReqPacket->resUserPKNum);

	FRIEND_REQUEST_RESPONSE FriendsReq_Res;
	FriendsReq_Res.PacketId = (UINT16)PACKET_ID::FRIEND_REQUEST_RESPONSE;
	FriendsReq_Res.PacketLength = sizeof(FRIEND_REQUEST_RESPONSE);

	// FRIEND_REQUEST_ALREADY
	if (FriendReqResult == 1) {
		FriendsReq_Res.FriendsReq_Res = (UINT32)FriendReqResult;
		SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_RESPONSE), (char*)&FriendsReq_Res);
	}
	// FRIEND_REQUEST_FAIL
	else if (FriendReqResult == 2) {
		FriendsReq_Res.FriendsReq_Res = (UINT32)FriendReqResult;
		SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_RESPONSE), (char*)&FriendsReq_Res);
	}

	// 1,2 외 다른 번호 받으면 요청 성공
	else {
		FRIEND_REQUEST_RESPONSE_TO_RESPONSE_USER FriendsReq_Res_Res;
		FriendsReq_Res_Res.friends_request_id = (UINT32)FriendReqResult;
		FriendsReq_Res_Res.reqUserPKNum = (UINT32)FriendReqPacket->reqUserPKNum;

		// 요청 받는 유저가 접속중일때 
		// 유저에게도 바로 메시지 전송 후 데이터베이스 수정
		if (userManager->FindUserByPK(FriendReqPacket->resUserPKNum) != -1) {
			if (mySQLManager->FriendRequestUpdate(FriendReqPacket->resUserPKNum)) {
				SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_RESPONSE_TO_RESPONSE_USER), (char*)&FriendsReq_Res_Res);
			}
			else {
				// 상대에게 요청 전송 실패
				FriendReqResult = 3;
				FriendsReq_Res.FriendsReq_Res = (UINT32)FriendReqResult;
				SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_RESPONSE), (char*)&FriendsReq_Res);
			}
		}

		// 요청 받는 유저가 접속중이 아닐때 
		// 요청받는 유저 테이블에 비접속동안 요청이 있었다는 데이터 전송
		else {
			// 요청 전송 성공
			if (mySQLManager->FriendRequestUpdate(FriendReqPacket->resUserPKNum)) {
				// 요청한 유저에게 성공 응답 전송
				FriendsReq_Res.FriendsReq_Res = (UINT32)FriendReqResult;
				SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_RESPONSE), (char*)&FriendsReq_Res);
			}
			// 요청 전송 실패
			else {
				// 상대에게 요청 전송 실패
				FriendReqResult = 3;
				FriendsReq_Res.FriendsReq_Res = (UINT32)FriendReqResult;
				SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_RESPONSE), (char*)&FriendsReq_Res);
			}
		}
	}
}

void PacketManager::FriendRequestCancel(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto FriendReqCancelPacket = reinterpret_cast<FRIEND_REQUEST_CANCEL_REQUEST*>(pPacket_);
	auto FriendReqCancelResult = mySQLManager->FriendRequestCancel(FriendReqCancelPacket->reqUserPKNum, FriendReqCancelPacket->resUserPKNum);

	FRIEND_REQUEST_CANCEL_RESPONSE FriendsReqCancel_Res;
	FriendsReqCancel_Res.PacketId = (UINT16)PACKET_ID::FRIEND_REQUEST_CANCEL_RESPONSE;
	FriendsReqCancel_Res.PacketLength = sizeof(FRIEND_REQUEST_CANCEL_RESPONSE);
	FriendsReqCancel_Res.FriendsReqCancel_Res = (UINT16)FriendReqCancelResult;
	SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_CANCEL_RESPONSE), (char*)&FriendsReqCancel_Res);
	
	// 친구 추가 요청 취소를 받은 유저가 접속중이면 해당 유저에게도 요청 삭제 메시지 전송
	if (userManager->FindUserByPK(FriendReqCancelPacket->resUserPKNum) != -1) {
		SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_CANCEL_RESPONSE_TO_RESPONSE_USER), (char*)&FriendReqCancelPacket->reqUserPKNum);
	}
}

void PacketManager::DeleteFriend(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {

	auto DelFriendPakcet = reinterpret_cast<DELETE_FRIEND_REQUEST*>(pPacket_);
	auto DelFriendResult = mySQLManager->DeleteFriend(DelFriendPakcet->reqUserPKNum, DelFriendPakcet->resUserPKNum);
	/*auto DelFriendResult = mySQLManager->DeleteFriend(DelFriendPakcet->resUserPKNum, DelFriendPakcet->reqUserPKNum);*/

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



