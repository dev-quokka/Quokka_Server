#pragma once

#include <mysql.h>
#pragma comment (lib, "libmysql.lib") // mysql 연동

class MySQLManger {

public:

	MYSQL getConn() {
		return Conn;
	}

	void Init() {
		mysql_init(&Conn);
		ConnPtr = mysql_real_connect(&Conn, "127.0.0.1", "root", "1234", "quokka_server", 3306, (char*)NULL, 0);
	}

	void CloseMySQL() {
		std::cout << "MySQL 종료" << std::endl;
		mysql_close(ConnPtr);
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