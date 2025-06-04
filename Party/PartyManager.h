#pragma once

#include "Party.h"
#include "ErrorCode.h"
#include "UserManager.h"

#include <unordered_map>
#include <vector>
#include <queue>
#include <iostream>

class PartyManager {
public :

	PartyManager() = default;
	~PartyManager() = default;

	void Init(int maxPartyCnt_) {

		MaxPartyCnt = maxPartyCnt_;
		for (int i = 0; i < MaxPartyCnt; i++) {
			UsableCheckQ.push(i);
		}
		for (int i = 0; i < MaxPartyCnt; i++) {
			Partys[i]->init(i);
		}
	}

	INT16 MakePartyCheck() {

		if (UsableCheckQ.empty()) {
			return -1;
		}

		else {
			UINT16 TempVal = UsableCheckQ.front();
			UsableCheckQ.pop();
			return TempVal;
		}
	}

	// �� �� ������ 1,2,3,4 �� �� �� �ִ� ��ġ ������
	UINT8 UsableEnterCheck(UINT16 partyIdx_) {

		if ((UINT16)(Partys[partyIdx_]->GetProperty())>Partys[partyIdx_]->GetPartySize()){
			return (Partys[partyIdx_]->GetPartySize()+1);
		}

		else 
			return 0;

	}

	//��Ƽ ����� ���� ��� : �ٸ� ���̰� ��Ƽ ���� ���̿��� �����ϱ� ������(�ο��� �ȳѾ������� ���� ���ص���) or �������� �ʴ��ؼ� �� ����� �ʴ� ��û�� �޾��� ��(���� ��û Ȯ��������)
	bool MakeParty(UINT32 partyIdx_, UINT32 reqUserPKNum_, UINT32 resUserPKNum_) {
		std::lock_guard<std::mutex> guard(plLock);
		Partys[partyIdx_]->AddUser(reqUserPKNum_);
		Partys[partyIdx_]->AddUser(resUserPKNum_);
		INT32 tempIdx = userManager->FindUserByPK(resUserPKNum_);
		Partys[partyIdx_]->SetPartyProperty(userManager->GetUserByIdx(tempIdx)->GetUserPartyProperty());
		return true;
	}

	bool JoinParty(int partyIdx_, int partyNum_) {

	}

	bool LeaveParty(int partyIdx_, FriendInfo*) {
		
	};

private :
	
	UserManager* userManager;

	std::queue<int> UsableCheckQ;
	
	std::mutex plLock;

	std::vector<Party*> Partys;
	
	UINT16 PartyCnt = 1;
	UINT16 MaxPartyCnt = 0;

};