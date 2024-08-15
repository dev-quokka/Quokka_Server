#pragma once

#include <mutex>

#include "User.h"
#include "UserManager.h"
#include "PartyManager.h"

#include <unordered_map>

class UserManager {
public:

	void Init() {

	}

	void AddUser(std::string userId_, int userIdx_) {
		std::lock_guard<std::mutex> guard(auLock);
		usersMap[userId_] = userIdx_;
	}
	
	// 친구 찾기
	int FindUserIdx(std::string userId_) {
		return usersMap[userId_];
	}

	void DeleteUserInfo(User* user_)
	{
		usersMap.erase(user_->getUserId());
		user_->Clear();
	}



private:

	std::mutex auLock; // adduser Lock
	
	std::vector<User*> user;

	std::unordered_map<std::string, int> usersMap; // 유저 아이디로 그 유저 구조체 알기 위한 idx 찾기 가능

};