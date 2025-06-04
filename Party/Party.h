#pragma once

#include "UserManager.h"
#include <vector>
#include <mutex>

// �����ؼ� �� ��������ڸ��� ä�� ������ ���ư� (��Ŷ ��� ���Ź��� �� �ְ� �����)
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

	// ��Ƽ�� �ѱ�� (��Ƽ�常 ���� �����ϰ�)
	bool SetOrganizer(UINT32 reqUserPKNum_) {
		Organizer = reqUserPKNum_;
	}

	// ��Ƽ�Ӽ� ���� (��Ƽ�常 ���� �����ϰ�)
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