#include "stdafx.h"
#include "TotalHeader.h"

CDBLog::CDBLog(WCHAR *p_DBIP, WCHAR *p_DBUser, WCHAR *p_DBPassword, WCHAR *p_DBName, int DBPort) : CDBConnector(p_DBIP, p_DBUser, p_DBPassword, p_DBName, DBPort)
{

}
CDBLog::~CDBLog(void)
{

}

bool CDBLog::ReadDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In, LPVOID p_Out)
{
    return false;
}
bool CDBLog::WriteDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In)
{
    bool IsWrite = false;
    switch (Type)
    {
    case enDB_ACCOUNT_READ_USER:
        IsWrite = WriteLogLogin((stDB_ACCOUNT_READ_USER_in *)p_In);
        break;
    case enDB_GAME_WRITE_LOG_JOIN:
        IsWrite = WriteLogJoin((stDB_GAME_WRITE_LOG_JOIN_in *)p_In);
        break;
    case enDB_GAME_WRITE_LOG_LEAVE:
        IsWrite = WriteLogLeave((stDB_GAME_WRITE_LOG_LEAVE_in *)p_In);
        break;
    case enDB_GAME_WRITE_LOG_PLAYER_DIE:
    case enDB_GAME_WRITE_PLAYER_DIE:
        IsWrite = WriteLogPlayerDie((stDB_GAME_WRITE_PLAYER_DIE_in *)p_In);
        break;
    case enDB_GAME_WRITE_LOG_PLAYER_KILL:
    case enDB_GAME_WRITE_PLAYER_KILL:
        IsWrite = WriteLogPlayerKill((stDB_GAME_WRITE_PLAYER_KILL_in *)p_In);
        break;
    default:
        CrashDump::Crash();
        return false;
        break;
    }
    return IsWrite;
}

bool CDBLog::WriteLogLogin(const stDB_ACCOUNT_READ_USER_in *p_UserIn)
{
    if (nullptr == p_UserIn)
        return false;

    if (false == QuerySave(L"INSERT INTO `logdb`.`gamelog` ( `type`, `code`, `AccountNo`, `ServerName`, `param4`, `message`) VALUES(%d, %d, %lld, '%s', %u, '%s');",
        GAME_LOG_CONNECTION, GAME_LOGIN, p_UserIn->AccountNo, g_GameServerConfig._ServerName,
        p_UserIn->ConnectInfo._Port, p_UserIn->ConnectInfo._IP))
    {
        // DB 접속에 문제가 있다.
        CrashDump::Crash();
        return false;
    }
    return true;
}
bool CDBLog::WriteLogJoin(const stDB_GAME_WRITE_LOG_JOIN_in *p_LogJoinIn)
{
    if (nullptr == p_LogJoinIn)
        return false;

    if (false == QuerySave(L"INSERT INTO `logdb`.`gamelog` ( `type`, `code`, `AccountNo`, `ServerName`, `param1`, `param2`, `param3`) VALUES(%d, %d, %lld, '%s', %d, %d, %u);",
        GAME_LOG_CONNECTION, GAME_JOIN, p_LogJoinIn->AccountNo, g_GameServerConfig._ServerName,
        p_LogJoinIn->TileX, p_LogJoinIn->TileY, p_LogJoinIn->CharacterType))
    {
        // DB 접속에 문제가 있다.
        CrashDump::Crash();
        return false;
    }
    return true;
}
bool CDBLog::WriteLogLeave(const stDB_GAME_WRITE_LOG_LEAVE_in *p_LogLeaveIn)
{
    if (nullptr == p_LogLeaveIn)
        return false;

    if (false == QuerySave(L"INSERT INTO `logdb`.`gamelog` ( `type`, `code`, `AccountNo`, `ServerName`, `param1`, `param2`, `param3`, `param4`) VALUES(%d, %d, %lld, '%s', %d, %d, %d, %d);",
        GAME_LOG_CONNECTION, GAME_LEAVE, p_LogLeaveIn->AccountNo, g_GameServerConfig._ServerName,
        p_LogLeaveIn->TileX, p_LogLeaveIn->TileY, p_LogLeaveIn->KillCount, p_LogLeaveIn->GuestKillCount))
    {
        // DB 접속에 문제가 있다.
        CrashDump::Crash();
        return false;
    }
    return true;
}
bool CDBLog::WriteLogPlayerDie(const stDB_GAME_WRITE_PLAYER_DIE_in *p_PlayerDieIn)
{
    if (nullptr == p_PlayerDieIn)
        return false;

    if (false == QuerySave(L"INSERT INTO `logdb`.`gamelog` ( `type`, `code`, `AccountNo`, `ServerName`, `param1`, `param2`, `param3`) VALUES(%d, %d, %lld, '%s', %d, %d, %lld);",
        GAME_LOG_CHARACTER, GAME_DIE, p_PlayerDieIn->AccountNo, g_GameServerConfig._ServerName,
        p_PlayerDieIn->DiePosX, p_PlayerDieIn->DiePosY, p_PlayerDieIn->AttackerAccountNo))
    {
        // DB 접속에 문제가 있다.
        CrashDump::Crash();
        return false;
    }
    return true;
}
bool CDBLog::WriteLogPlayerKill(const stDB_GAME_WRITE_PLAYER_KILL_in *p_PlayerKillIn)
{
    if (nullptr == p_PlayerKillIn)
        return false;

    if (false == QuerySave(L"INSERT INTO `logdb`.`gamelog` ( `type`, `code`, `AccountNo`, `ServerName`, `param1`, `param2`, `param3`) VALUES(%d, %d, %lld, '%s', %d, %d, %lld);",
        GAME_LOG_CHARACTER, GAME_KILL, p_PlayerKillIn->AccountNo, g_GameServerConfig._ServerName,
        p_PlayerKillIn->KillPosX, p_PlayerKillIn->KillPosY, p_PlayerKillIn->TargetAccountNo))
    {
        // DB 접속에 문제가 있다.
        CrashDump::Crash();
        return false;
    }
    return true;
}