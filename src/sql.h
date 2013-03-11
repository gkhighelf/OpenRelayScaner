/**
 * Created by GKHighElf - gkhighelf@gmail.com
 * in near 2000 year, for self education and use.
**/

#include <mysql.h>

#define USE_MYSQL4

#ifdef USE_MYSQL4
MYSQL *mysql;
#else
MYSQL mysql;
#endif

MYSQL_RES *res;
MYSQL_ROW row;

MYSQL_RES *res_;
MYSQL_ROW row_;

#define SQL_HOST "127.0.0.1"
#define SQL_LOGIN "statistics"
#define SQL_PWD "statistics"
#define SQL_DB "statistics"

