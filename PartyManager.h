#pragma once
#include "Party.h"
#include <vector>
#include <queue> // 비어있는 룸 확인용 큐

class PartyManager {
public :

	PartyManager() = default;
	~PartyManager() = default;

	int getUsablePartySize() {
		return ;
	}

	void init(int maxPartyCnt_) {

		PartyIdx = PartyIdx;

		for (int i = 0; i < maxPartyCnt_; i++) {
			partyList[i] = new Party();
			partyList[i]->init(i);
			usableCheckQueue.push(i);
		}

	}

	//파티 만들어 지는 경우 : 다른 아이가 파티 없는 아이에게 참가하기 했을때(인원만 안넘어있으면 응답 안해도됌) or 누군가를 초대해서 그 사람이 초대 요청을 받았을 때(응답 요청 확인했을때)
	bool makeParty(UserInfo* organizer_, UserInfo* member_) {

		std::lock_guard<std::mutex> guard(qcLock);
		if (usableCheckQueue.empty()) {
			return false;
		}

		int usableQ = usableCheckQueue.front();
		usableCheckQueue.pop();

		partyList[usableQ]->MakeParty(organizer_,member_);

	}

	bool addParty(UserInfo* user_, int partyIndx) {

		std::lock_guard<std::mutex> guard(pmLock);
		if (partyList[partyIndx]->addUser(user_)) {
			return true;
		}
		
		return false;
	}

	void closeParty() {

	}

private :
	std::vector<Party*> partyList;

	std::queue<int> usableCheckQueue;

	std::mutex pmLock;
	std::mutex qcLock;

	int PartyCnt = 0;
	int PartyIdx;

};