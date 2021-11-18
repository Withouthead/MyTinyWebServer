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
    MySqlPoolRaII(MYSQL** sql_)
    {
        *sql_ = MySqlPool::GetInstance();
        sql = *sql_;
    }
    ~MySqlPoolRaII()
    {
        if(sql)
        {
            MySqlPool::GetInstance()->FreeConnection(sql);
        }

    }
private:
    MYSQL *sql;
};

#endif //MYTINYSERVER_MYSQLPOOLRAII_H
