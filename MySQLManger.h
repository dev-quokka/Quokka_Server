#pragma once

#include <mysql.h>
#pragma comment (lib, "libmysql.lib") // mysql ¿¬µ¿

MYSQL Conn;
MYSQL* ConnPtr = NULL;
MYSQL_RES* Result;
MYSQL_ROW Row;
int MysqlResult;