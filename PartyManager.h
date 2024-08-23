#pragma once
#include "Party.h"
#include <unordered_map>
#include <vector>

class PartyManager {
public :

	PartyManager() = default;
	~PartyManager() = default;

	void Init(int maxPartyCnt_) {
		MaxPartyCnt = maxPartyCnt_;
	}

	bool UsableEnterCheck(int partyIdx_) {

	}

	//파티 만들어 지는 경우 : 다른 아이가 파티 없는 아이에게 참가하기 했을때(인원만 안넘어있으면 응답 안해도됌) or 누군가를 초대해서 그 사람이 초대 요청을 받았을 때(응답 요청 확인했을때)
	bool MakeParty(int partyIdx_, std::vector<FriendInfo*> friendsInfo_) {
		

		if (MaxPartyCnt == 0 || MaxPartyCnt <= PartyCnt) {
			return false;
		}
		
		PartyList[partyIdx_] = friendsInfo_;
		PartyCnt++;
		return true;

	}

	bool JoinParty(int partyIdx_, std::vector<FriendInfo*> friendsInfo_) {

		if (PartyList[partyIdx_].size() > 4) {
			return false;
		}
		PartyList[partyIdx_].emplace_back(friendsInfo_);
		return true;
	}

	bool LeaveParty(int partyIdx_, FriendInfo*) {
		
	};

	UINT16 MakePartyCheck() {

		if (MaxPartyCnt == 0 || MaxPartyCnt <= PartyCnt) {
			return 0;
		}

		return PartyCnt;
	}

private :

	std::unordered_map<int, std::vector<FriendInfo*>> PartyList; // 파티번호, 파티원 수

	UINT16 PartyCnt = 1;
	UINT16 MaxPartyCnt = 0;

};