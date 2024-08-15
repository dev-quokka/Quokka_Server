#pragma once

#include "UserInfo.h"
#include <vector>
#include <mutex>

// 접속해서 룸 만들어지자마자 채팅 쓰레드 돌아감 (패킷 계속 수신받을 수 있게 만들기)
class Party {
public: 

	enum class PartyProperty {
		NONE = 0,
		SOLO = 1,
		DUO = 2,
		SQUARD = 4
	};

	void init(int partyIdx_) {
		PartyIdx = partyIdx_;
	}

	PartyProperty GetProperty() {
		return Property;
	}

	void MakeParty(UserInfo* organizer_, UserInfo* member_) {
		Organizer = organizer_->getUserIdx();
		party.emplace_back(organizer_);
		party.emplace_back(member_);
	}

	void closeParty() {

	}

	// 유저 추방
	void expelUser(int Organizer_) {

	}

	void ChatToAll() {

	}

	bool addUser(UserInfo* user_) {

		std::lock_guard<std::mutex> guard(psLock);
		if (getPartySize() < 4) {
			party.emplace_back(user_);
			return true;
		}

		return false;

	}

	int getPartySize() {
		return party.size(); 
	}

	// 파티장 넘기기 (파티장만 설정 가능하게)
	bool setOrganizer(UserInfo* user_) {
		Organizer = user_->getUserIdx();
	}

private:

	PartyProperty Property = PartyProperty::NONE;
	int Organizer;
	int PartyIdx;

	std::mutex psLock;

	std::vector<User*> party;
};