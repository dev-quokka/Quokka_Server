#pragma once

#include <mysql.h>
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

	ERROR_CODE MysqlLoginCheck(std::string user_id, std::string userPassword) {

		std::string temp_user_id;
		std::string temp_password;
		std::string query_s = "SELECT user_id,password FROM user WHERE user_id = '" + user_id + "'";
		
		const char* Query = &*query_s.begin();
		
		MysqlResult = mysql_query(ConnPtr, Query);

		if (MysqlResult == 0) {
			Result = mysql_store_result(ConnPtr);
			while ((Row = mysql_fetch_row(Result)) != NULL) {
				temp_user_id = Row[0];
				temp_password = Row[1];
			}
			mysql_free_result(Result);
		}

		if (user_id != temp_user_id) {
			return ERROR_CODE::LOGIN_USER_INVALID_ID;
		}

		else if (userPassword != temp_password) {
			return ERROR_CODE::LOGIN_USER_INVALID_PW;
		}

		else return ERROR_CODE::NONE;
	}

	std::string SearchUser() {

	}


private:

	MYSQL Conn;
	MYSQL* ConnPtr = NULL;
	MYSQL_RES* Result;
	MYSQL_ROW Row;
	int MysqlResult;

};