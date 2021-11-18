//
// Created by xiaomaotou31 on 2021/11/18.
//

#ifndef MYTINYSERVER_MYSQLPOOLRAII_H
#define MYTINYSERVER_MYSQLPOOLRAII_H
#include "MySqlPool.h"
#include <mysql/mysql.h>
class MySqlPoolRaII
{
public:
    MySqlPoolRaII()
    {
        sql_connection = MySqlPool::GetInstance()->GetConnection();
    }
    ~MySqlPoolRaII()
    {
        if(sql_connection)
        {
            MySqlPool::GetInstance()->FreeConnection(sql_connection);
        }

    }
    MYSQL *getSqlConnection() {
        return sql_connection;
    }
private:
    MYSQL *sql_connection;
};

#endif //MYTINYSERVER_MYSQLPOOLRAII_H
