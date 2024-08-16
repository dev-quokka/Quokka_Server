#pragma once

#include "Packet.h"

class User {
public :
	enum class User_STATE
	{
		NONE = 0,
		LOGIN = 1,
		PARYTY = 2,
		PLAY = 3
	};

	// 나한테 파티 따라오기 할때 처리 함수 (yes or no)
	bool followRequest(){


		return partyRequest;
	}

	// 초대 요청하는 함수
	


	// 따라오기 수락을 누르면 처리되는 함수
	bool partyRequest(int clientIdx_) {
		
	}

	// 따라오기 수락을 누르면 처리되는 함수
	bool partyFollow(int clientIdx_) {
		
	}

	std::string getUserId() {
		return userId;
	}

private:

	UINT16 userPkNum;
	UINT8 userIdx;
	UINT8 partyIdx = 0;

	std::string userId;

	User_STATE mCurDomainState = User_STATE::NONE;

};
