#pragma once

#include "vendor/sql_mm/src/public/mysql_mm.h"
#include "vendor/sql_mm/src/public/sql_mm.h"

class ZDatabase
{
private:
	static IMySQLConnection* pConnection;

public:
	static void Init();
	static void OnConnected(bool connect);
	static IMySQLConnection* GetConnection() { return pConnection; }

	static void OnGenericTxnSuccess(std::vector<ISQLQuery*> queries)
	{
		ConMsg("[DB] Transaction successful.\n");
	}

	static void OnGenericTxnFailure(std::string error, int failIndex)
	{
		ConMsg("[DB] Transaction failed at %i (%s).\n", failIndex, error.c_str());
	}
};