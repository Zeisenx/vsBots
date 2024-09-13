
#include "cs2fixes.h"
#include "database.h"
#include "vsbots.h"

#include "vendor/sql_mm/src/public/mysql_mm.h"
#include "vendor/sql_mm/src/public/sql_mm.h"

IMySQLConnection* ZDatabase::pConnection;

void ZDatabase::Init()
{
	int ret;
	ISQLInterface *sqlInterface = (ISQLInterface*)g_SMAPI->MetaFactory(SQLMM_INTERFACE, &ret, NULL);

	if (ret == META_IFACE_FAILED)
	{
		ConMsg("[CS2Fixes DB] Failed to lookup mysql client. Aborting\n");
		return;
	}

	KeyValues *kvConfig = new KeyValues("ZDatabase");
	//KeyValues::AutoDelete autoDelete(kvConfig);
	const char* pszPath = "addons/cs2fixes/configs/database.cfg";
	if (!kvConfig->LoadFromFile(g_pFullFileSystem, pszPath))
	{
		ConMsg("[CS2Fixes DB] Failed to find database config. Aborting\n");
		return;
	}

	MySQLConnectionInfo info{.host = kvConfig->GetString("host"), .user = kvConfig->GetString("user"), .pass = kvConfig->GetString("pass"), .database = kvConfig->GetString("database")};
	pConnection = sqlInterface->GetMySQLClient()->CreateMySQLConnection(info);

	ConMsg("[CS2Fixes DB] Connecting...\n");
	pConnection->Connect(OnConnected);
}

void ZDatabase::OnConnected(bool connect)
{
	if (!connect)
	{
		ConMsg("[CS2Fixes DB] Failed to connect\n");

		// make sure to properly destroy the connection
		pConnection->Destroy();
		pConnection = nullptr;
		return;
	}

	pConnection->Query("set names utf8mb4", [](ISQLQuery* test){});
	VSBots::OnDBConnected();
}