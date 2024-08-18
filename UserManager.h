#pragma once

#include <mutex>

#include "User.h"
#include "PartyManager.h"

#include <unordered_map>

class UserManager {
public:

	int GetUserMaxCnt() {
		return maxUserCnt;
	}

	void Init(int maxUserCnt_) {
		maxUserCnt = maxUserCnt_;

	}

	void AddUser(int userIdx_,int userPKNum_,char* userID_) {
		std::lock_guard<std::mutex> guard(auLock);
		users[userIdx_]->SetLogin(userID_,userPKNum_);
		usersMap[userPKNum_] = userIdx_;
	}
	
	// 친구 찾기
	int FindUserIdx(char* userId_) {
		
	}

	void DeleteUserInfo(User* user_)
	{
		usersMap.erase(user_->GetUserPKNum());
		user_->Clear();
	}

	User* GetUserByConnIdx(INT32 clientIndex_)
	{
		return users[clientIndex_];
	}

private:

	int maxUserCnt;
	int CurrentUserCnt = 0;
	
	std::mutex auLock; // adduser Lock
	
	std::vector<User*> users;

	std::unordered_map<int, int> usersMap; // 유저 아이디로 그 유저 구조체 알기 위한 idx 찾기 가능

};