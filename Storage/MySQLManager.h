#pragma once

#include <mysql.h>
#include <string>
#include "ErrorCode.h"

#pragma comment (lib, "libmysql.lib") // mysql 연동

class MySQLManager {

public:

	MYSQL getConn() {
		return Conn;
	}

	void Run() {
		mysql_init(&Conn);
		ConnPtr = mysql_real_connect(&Conn, "127.0.0.1", "root", "1234", "quokka_server", 3306, (char*)NULL, 0);

		if (ConnPtr == NULL) {
			std::cout << "데이터베이스 연결 실패" << std::endl;
		}

		std::cout << "데이터베이스 연결 성공" << std::endl;

	}

	void CloseMySQL() {
		std::cout << "MySQL 종료" << std::endl;
		mysql_close(ConnPtr);
	}

	UINT32 MysqlLoginCheck(std::string user_id, std::string userPassword) {

		std::string temp_user_pk;
		std::string temp_user_id;
		std::string temp_password;
		std::string query_s = "SELECT user_id,id,password FROM user_tb WHERE id = '" + user_id + "'";
		
		const char* Query = &*query_s.begin();
		
		MysqlResult = mysql_query(ConnPtr, Query);

		if (MysqlResult == 0) {
			Result = mysql_store_result(ConnPtr);
			while ((Row = mysql_fetch_row(Result)) != NULL) {
				temp_user_pk = Row[0];
				temp_user_id = Row[1];
				temp_password = Row[2];
			}
			mysql_free_result(Result);
		}

		if (user_id != temp_user_id) {
			return 1;
		}

		else if (userPassword != temp_password) {
			return 2;
		}

		else {
			return (UINT32)std::stoi(temp_user_pk);
		}
	}

	FriendInfo MyInfo(UINT8 LoginDBResult_) {

		std::string query_s = "SELECT user_id,id,user_level,user_party_num,friends_request FROM user_tb WHERE user_id = " + std::to_string(LoginDBResult_);

		const char* Query = &*query_s.begin();

		Query = &*query_s.begin();

		MysqlResult = mysql_query(ConnPtr, Query);

		FriendInfo userInfo;

		if (MysqlResult == 0) {
			Result = mysql_store_result(ConnPtr);
			while ((Row = mysql_fetch_row(Result)) != NULL) {
				userInfo.userPkNum = (UINT32)std::stoi(Row[0]);
				userInfo.id = (Row[1]);
				userInfo.userLevel = (UINT32)std::stoi(Row[2]);
				userInfo.partyIdx = (UINT32)std::stoi(Row[3]);
				userInfo.Check = (UINT32)std::stoi(Row[4]);
			}
			mysql_free_result(Result);
		}

		return userInfo;
	}

	UINT32  FindUserById(std::string userId_) {
		std::string temp_user_id;
		std::string query_s = "SELECT user_id FROM user_tb  WHERE id = '" + userId_ + "'";

		const char* Query = &*query_s.begin();

		MysqlResult = mysql_query(ConnPtr, Query);

		if (MysqlResult == 0) {
			Result = mysql_store_result(ConnPtr);
			while ((Row = mysql_fetch_row(Result)) != NULL) {
				temp_user_id = Row[0];
			}
			if (Row[0] == NULL) {
				return -1;
			}
			mysql_free_result(Result);
		}
		return std::stoi(Row[0]);
	}

	std::vector<FriendInfo> FindUserFriendsInfo(UINT32 userPKNum_) {

		std::vector<FriendInfo> FriendsInfo;
		std::string query_s = "select u.user_id,u.id,u.user_level,u.user_party_num from friends_tb f LEFT JOIN user_tb u on f.user_pk2 = u.user_id where user_pk1 = " + std::to_string(userPKNum_);

		const char* Query = &*query_s.begin();
		MysqlResult = mysql_query(ConnPtr, Query);

		if (MysqlResult == 0) {
			Result = mysql_store_result(ConnPtr);
			while ((Row = mysql_fetch_row(Result)) != NULL) {
				FriendInfo temp_friendInfo;
				temp_friendInfo.userPkNum = std::stoi(Row[0]);
				temp_friendInfo.id = (Row[1]);
				temp_friendInfo.userLevel = std::stoi(Row[2]);
				temp_friendInfo.partyIdx = std::stoi(Row[3]);
				FriendsInfo.emplace_back(temp_friendInfo);
			}
			mysql_free_result(Result);
		}
		return FriendsInfo;
	}

	UINT32 FriendRequest(UINT32 reqUserPK_, UINT32 resUserPK_) {

		// 그 사람에게 요청 이미 있는지 체크
		std::string query_s = "select friends_request_id FROM friends_request_tb where user_pk1="+ std::to_string(reqUserPK_)+"AND user_pk2 = "+ std::to_string(resUserPK_);
		bool CheckRequest = false;
		const char* Query = &*query_s.begin();
		MysqlResult = mysql_query(ConnPtr, Query);

		if (MysqlResult == 0) {
			Result = mysql_store_result(ConnPtr);
			int cnt = 0;
			while ((Row = mysql_fetch_row(Result)) != NULL) {
			}
			if (Row[0] == NULL) {
				CheckRequest = true;
			}
			mysql_free_result(Result);
		}

		if (CheckRequest) {
			query_s = "INSERT INTO friends_request_tb values(NULL," + std::to_string(reqUserPK_) + "," + std::to_string(resUserPK_) + ")";
			Query = &*query_s.begin();
			MysqlResult = mysql_query(ConnPtr, Query);

			if (MysqlResult != 0) {
				// FRIEND_REQUEST_FAIL
				return 2;
			}
			// INSERT 성공
			else {
				query_s = "select friends_request_id FROM friends_request_tb where user_pk1 ="+ std::to_string(reqUserPK_)+"AND user_pk2 = "+ std::to_string(resUserPK_);
				Query = &*query_s.begin();
				MysqlResult = mysql_query(ConnPtr, Query);
				std::string temp_friends_request_id = 0;
				if (MysqlResult == 0) {
					Result = mysql_store_result(ConnPtr);
					while ((Row = mysql_fetch_row(Result)) != NULL) {
						temp_friends_request_id = Row[0];
					}
					mysql_free_result(Result);
				}
				return (UINT32)std::stod(temp_friends_request_id);
			}
		}
		else {
			// ERROR_CODE::FRIEND_REQUEST_ALREADY
			return 1;
		}

	}

	bool FriendRequestUpdate(UINT32 resUserPK_) {
		std::string query_s = "update user_tb set friends_request = 1 where user_id" + std::to_string(resUserPK_);
		bool CheckRequest = false;
		const char* Query = &*query_s.begin();
		MysqlResult = mysql_query(ConnPtr, Query);

		if (MysqlResult == 0) {
			Result = mysql_store_result(ConnPtr);
			int cnt = 0;
			while ((Row = mysql_fetch_row(Result)) != NULL) {
			}
			if (Row[0] == NULL) {
				return true;
			}
			mysql_free_result(Result);
		}
		return false;
	}

	ERROR_CODE FriendRequestCancel(UINT32 reqUserPK_, UINT32 resUserPK_) {

		std::string query_s = "delete FROM friends_request_tb WHERE user_pk1 = " + std::to_string(reqUserPK_) + "AND user_pk2 = " + std::to_string(resUserPK_);
		
		const char* Query = &*query_s.begin();
		MysqlResult = mysql_query(ConnPtr, Query);

		if (MysqlResult != 0) {
			return ERROR_CODE::FRIEND_REQUEST_CANCEL_FAIL;
		}

		else return ERROR_CODE::NONE;
	}

	ERROR_CODE DeleteFriend(UINT32 reqUserPK_, UINT32 resUserPK_) {

		std::string query_s = "delete FROM friends_tb WHERE user_pk1 = " + std::to_string(reqUserPK_) + "AND user_pk2 = " + std::to_string(resUserPK_);

		const char* Query = &*query_s.begin();
		MysqlResult = mysql_query(ConnPtr, Query);

		if (MysqlResult != 0) {
			return ERROR_CODE::FRIEND_DELETE_FAIL;
		}

		else return ERROR_CODE::NONE;
	}

	ERROR_CODE MakeParty(UINT16 partyIdx_,UINT32 reqUserPK_, UINT32 resUserPK_) {

		std::string query_s = "INSERT INTO party_tb VALUES(NULL," + std::to_string(resUserPK_) +","+ std::to_string(resUserPK_) + "," + std::to_string(reqUserPK_) + ",null,null,"+ std::to_string(partyIdx_) + ")";

		const char* Query = &*query_s.begin();
		MysqlResult = mysql_query(ConnPtr, Query);

		if (MysqlResult != 0) return ERROR_CODE::PARTY_MAKE_FAIL;

		else return ERROR_CODE::NONE;
	}

	ERROR_CODE EnterParty(UINT32 reqUserPK_, UINT32 partyIdx_, UINT8 InsertNum_) {
		std::string query_s = "UPDATE party_tb SET user"+ std::to_string(InsertNum_) +" = " +std::to_string(reqUserPK_) + " where party_idx = "+ std::to_string(partyIdx_);
		
		const char* Query = &*query_s.begin();
		MysqlResult = mysql_query(ConnPtr, Query);

		if (MysqlResult != 0) return ERROR_CODE::PARTY_ENTER_FAIL;

		else return ERROR_CODE::NONE;
	}

	ERROR_CODE ChangePartyOrganizer(UINT32 reqUserPK_, UINT32 partyNum) {

	}

	ERROR_CODE OutParty(UINT32 reqUserPK_) {

	}

	ERROR_CODE ExpelUser(UINT32 reqUserPK_, UINT32 partyNum) {

	}
	
private:
	MYSQL Conn;
	MYSQL* ConnPtr = NULL;
	MYSQL_RES* Result;
	MYSQL_ROW Row;
	int MysqlResult;
};