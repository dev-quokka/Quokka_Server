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

	// ���� ���������� Ȯ��
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

	void UserClear(UINT32 userIdx_) {
		currentUserMap.erase(users[userIdx_]->GetUserPKNum());
		users[userIdx_]->Clear();
	}

	void AddUser(int userIdx_,FriendInfo* friendInfo_) {
		std::lock_guard<std::mutex> guard(auLock);
		users[userIdx_]->SetLogin(friendInfo_);
		currentUserMap[friendInfo_->userPkNum] = userIdx_;
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

	std::unordered_map<UINT32, UINT32> currentUserMap; // ���� pk�� �� ���� ����ü �˱� ���� idx ã�� ���� <userPK, userIdx>

};