#include "PacketManager.h"

void PacketManager::Init(const UINT32 maxClient_)
{
	RecvFuntionDictionary = std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION>();
	
	RecvFuntionDictionary[(int)PACKET_ID::SYS_USER_CONNECT] = &PacketManager::UserConnect;
	RecvFuntionDictionary[(int)PACKET_ID::SYS_USER_DISCONNECT] = &PacketManager::UserDisConnect;
	RecvFuntionDictionary[(int)PACKET_ID::LOGIN_REQUEST] = &PacketManager::Login;

	// <ģ��>
	RecvFuntionDictionary[(int)PACKET_ID::FIND_USER_REQUEST] = &PacketManager::FindUserById; // ģ�� �˻��� ���� ���� �˻�
	RecvFuntionDictionary[(int)PACKET_ID::FIND_FRIENDS_REQUEST] = &PacketManager::FindUserFriendsInfo;
	RecvFuntionDictionary[(int)PACKET_ID::FRIEND_REQUEST_REQUEST] = &PacketManager::FriendRequest;
	RecvFuntionDictionary[(int)PACKET_ID::FRIEND_REQUEST_CANCEL_REQUEST] = &PacketManager::FriendRequestCancel;
	RecvFuntionDictionary[(int)PACKET_ID::FRIEND_DELETE_REQUEST] = &PacketManager::DeleteFriend;

	// <��Ƽ>
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
	// ���⿡�� ������ ���̽� ��������
	mySQLManager->Run();

	//�� �κ��� ��Ŷ ó�� �κ����� �̵� ��Ų��.
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

	std::cout << "��Ŷ ������ ����" << std::endl;

	while (RunProcessThread)
	{
		bool isIdle = true;

		auto packetData= DequePacketData();
		
		// ���� ����, �α׾ƿ��� ������ ��� ��Ŷ ó�� �κ�
		if (packetData.PacketId > (UINT16)PACKET_ID::SYS_END)
		{
			isIdle = false;
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketId, packetData.DataSize, packetData.pDataPtr);
		}

		// ���� ����,�α׾ƿ� �ý��� ��Ŷ ó�� �κ�
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
	std::cout << "���� ���� clientIndex : " << clientIndex_ << std::endl;
	auto User = userManager->GetUserByIdx(clientIndex_);
	User->Clear();
}

void PacketManager::UserDisConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	userManager->UserClear(clientIndex_);
	std::cout << "���� ���� �ʱ�ȭ" << std::endl;
	std::cout << " �������� ���� ���� clientIndex : "<< clientIndex_<<std::endl;
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

	// ������ ���� �� ������
	if (userManager->GetCurrentUserCnt() >= userManager->GetUserMaxCnt())
	{
		//�����ڼ��� �ִ���� �����ؼ� ���ӺҰ�
		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_FULL;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		return;
	}

	auto LoginDBResult = mySQLManager->MysqlLoginCheck(UserId,UserPassword);

	// ���̵� �ٸ���
	if (LoginDBResult == 1) {
		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_INVALID_ID;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		return;
	}

	// ��й�ȣ�� �ٸ���
	else if (LoginDBResult == 2) {
		LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_INVALID_PW;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
		return;
	}

	else {
		// �̹� ������ �α��� �� �϶�
		if (userManager->FindUserByPK(LoginDBResult)!=-1) {
			//�������� �������� ���и� ��ȯ.
			LoginResPacket.LoginResult = (UINT16)ERROR_CODE::LOGIN_USER_ALREADY;
			SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&LoginResPacket);
			return;
		}

		FriendInfo k = mySQLManager->MyInfo(LoginDBResult);
		userManager->AddUser(clientIndex_, &k);

		std::cout << "���� " << UserId << " ����" << std::endl;

		// �� ���� ģ����� �ѷ��ְ�
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

	// �� ���̵� ģ�� ���� ��
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

	// �α��� �� Ŭ���̾�Ʈ ��Ŷ���� -1�̸� �α׾ƿ����� 1�̸� �α��� ����
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

	// 1,2 �� �ٸ� ��ȣ ������ ��û ����
	else {
		FRIEND_REQUEST_RESPONSE_TO_RESPONSE_USER FriendsReq_Res_Res;
		FriendsReq_Res_Res.friends_request_id = (UINT32)FriendReqResult;
		FriendsReq_Res_Res.reqUserPKNum = (UINT32)FriendReqPacket->reqUserPKNum;

		// ��û �޴� ������ �������϶� 
		// �������Ե� �ٷ� �޽��� ���� �� �����ͺ��̽� ����
		if (userManager->FindUserByPK(FriendReqPacket->resUserPKNum) != -1) {
			if (mySQLManager->FriendRequestUpdate(FriendReqPacket->resUserPKNum)) {
				SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_RESPONSE_TO_RESPONSE_USER), (char*)&FriendsReq_Res_Res);
			}
			else {
				// ��뿡�� ��û ���� ����
				FriendReqResult = 3;
				FriendsReq_Res.FriendsReq_Res = (UINT32)FriendReqResult;
				SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_RESPONSE), (char*)&FriendsReq_Res);
			}
		}

		// ��û �޴� ������ �������� �ƴҶ� 
		// ��û�޴� ���� ���̺� �����ӵ��� ��û�� �־��ٴ� ������ ����
		else {
			// ��û ���� ����
			if (mySQLManager->FriendRequestUpdate(FriendReqPacket->resUserPKNum)) {
				// ��û�� �������� ���� ���� ����
				FriendsReq_Res.FriendsReq_Res = (UINT32)FriendReqResult;
				SendPacketFunc(clientIndex_, sizeof(FRIEND_REQUEST_RESPONSE), (char*)&FriendsReq_Res);
			}
			// ��û ���� ����
			else {
				// ��뿡�� ��û ���� ����
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
	
	// ģ�� �߰� ��û ��Ҹ� ���� ������ �������̸� �ش� �������Ե� ��û ���� �޽��� ����
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

		//ģ�� ���� ���� �������׵� ���� �޽��� ������
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

	//�� �� ������ �� �Լ� ����
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



