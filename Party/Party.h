#pragma once

#include "UserManager.h"
#include <vector>
#include <mutex>

// 접속해서 룸 만들어지자마자 채팅 쓰레드 돌아감 (패킷 계속 수신받을 수 있게 만들기)
class Party {
public: 
	
	void init(int partyIdx_) {
		PartyIdx = partyIdx_;
	}

	PartyProperty GetProperty() {
		return Property;
	}

	void AddUser(int user_) {

		std::lock_guard<std::mutex> guard(psLock);
		return;

	}

	UINT16 GetPartySize() {
		return partyUsers.size();
	}

	// 파티장 넘기기 (파티장만 설정 가능하게)
	bool SetOrganizer(UINT32 reqUserPKNum_) {
		Organizer = reqUserPKNum_;
	}

	// 파티속성 변경 (파티장만 설정 가능하게)
	void SetPartyProperty(PartyProperty property_) {
		Property = property_;
	}

private:

	PartyProperty Property = PartyProperty::DUO;
	UINT32 Organizer;
	UINT16 PartyIdx;

	std::mutex psLock;

	std::vector<int> partyUsers;
};