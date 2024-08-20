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

	}

	void CloseMySQL() {
		std::cout << "MySQL 종료" << std::endl;
		mysql_close(ConnPtr);
	}

	UINT32  MysqlLoginCheck(std::string user_id, std::string userPassword) {

		std::string temp_user_pk;
		std::string temp_user_id;
		std::string temp_password;
		std::string query_s = "SELECT id,user_id,password FROM user WHERE user_id = '" + user_id + "'";
		
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
			return -1;
		}

		else if (userPassword != temp_password) {
			return -2;
		}

		else return std::stoi(Row[0]);
	}

	UINT32  FindUserById(std::string userId_) {
		std::string temp_user_id;
		std::string query_s = "SELECT id user WHERE user_id = '" + userId_ + "'";

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

	UINT32 FriendRequest(int userPKNum_, int friendPKNum_) {
		
	}

	std::vector<std::vector<std::string>> FindUserFriends(int userPKNum_) {

		std::vector<std::vector<std::string>> Friends;
		std::string query_s = "select u.id,u.user_level,u.user_id from friends_tb f LEFT JOIN user_tb u on f.user_pk2 = u.id where user_pk1 = '" + std::to_string(userPKNum_) + "'";

		const char* Query = &*query_s.begin();

		if (MysqlResult == 0) {
			Result = mysql_store_result(ConnPtr);
			int cnt = 0;
			while ((Row = mysql_fetch_row(Result)) != NULL) {
				Friends[cnt++].emplace_back(Row[0]);
				Friends[cnt++].emplace_back(Row[1]);
				Friends[cnt++].emplace_back(Row[2]);
			}
			if (Row[0] == NULL) {
				return Friends;
			}
			mysql_free_result(Result);
		}
		return Friends;
	}

	UINT32 NewFriendRequest() {

	}

	UINT32 DeleteFriend() {

	}
	
private:

	MYSQL Conn;
	MYSQL* ConnPtr = NULL;
	MYSQL_RES* Result;
	MYSQL_ROW Row;
	int MysqlResult;

};