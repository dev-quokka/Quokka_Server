#pragma once

#include <mutex>

#include "User.h"
#include <unordered_map>

class UserManager {
public:

	INT32 GetUserMaxCnt() {
		return maxUserCnt;
	}

	INT32 GetCurrentUserCnt() {
		return CurrentUserCnt;
	}

	// 친구 찾기
	INT32 FindUserByPK(int userPKNum_) {
		auto res = currentUserMap.find(userPKNum_);
		if (res != currentUserMap.end())
		{
			return (*res).second;
		}

		return -1;
	}
		 
	void Init(int maxUserCnt_) {
		maxUserCnt = maxUserCnt_;
		users = std::vector<User*>(maxUserCnt_);

		for (auto i = 0; i < maxUserCnt_; i++)
		{
			users[i] = new User();
			users[i]->Init(i);
		}
	}

	void AddUser(int userIdx_,int userPKNum_,char* userID_) {
		std::lock_guard<std::mutex> guard(auLock);
		users[userIdx_]->SetLogin(userID_,userPKNum_);
		currentUserMap[userPKNum_] = userIdx_;
		// 친구 유저 정보 요청
	}

	void DeleteUserInfo(User* user_)
	{
		users[currentUserMap[user_->GetUserPKNum()]]->Clear();
		currentUserMap.erase(user_->GetUserPKNum());
	}

	User* GetUserByIdx(INT32 clientIndex_)
	{
		return users[clientIndex_];
	}

private:

	int maxUserCnt;
	int CurrentUserCnt = 0;
	
	std::mutex auLock; // adduser Lock
	
	std::vector<User*> users;

	std::unordered_map<int, int> currentUserMap; // 유저 pk로 그 유저 구조체 알기 위한 idx 찾기 가능 <userPK, userIdx>

};