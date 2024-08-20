#include "stdafx.h"
#include "TotalHeader.h"

CDBGame::CDBGame(WCHAR *p_DBIP, WCHAR *p_DBUser, WCHAR *p_DBPassword, WCHAR *p_DBName, int DBPort) : CDBConnector(p_DBIP, p_DBUser, p_DBPassword, p_DBName, DBPort)
{
    _p_DBLog = nullptr;
}
CDBGame::~CDBGame(void)
{

}

// ���� ��� ��� �ϳ�?
bool CDBGame::ReadDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In, LPVOID p_Out)
{
    bool IsRead = false;
    switch (Type)
    {
    case enDB_GAME_READ_PLAYER_CHECK:
        IsRead = ReadPlayerCheck(Type, (stDB_GAME_READ_PLAYER_CHECK_in *)p_In, (stDB_GAME_READ_PLAYER_CHECK_out *)p_Out);
        break;
    default:
        CrashDump::Crash();
        return false;
        break;
    }
    return IsRead;
}
bool CDBGame::WriteDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In)
{
    bool IsWrite = false;
    //bool IsWrite = true;
    switch (Type)
    {
    case enDB_GAME_WRITE_LOG_JOIN:
    case enDB_GAME_WRITE_LOG_LEAVE:
    case enDB_GAME_WRITE_LOG_PLAYER_DIE:
    case enDB_GAME_WRITE_LOG_PLAYER_KILL:
        // �α׸� �����.
        WriteLog(Type, p_In);
        IsWrite = true;
        break;
    case enDB_GAME_WRITE_PLAYER_DIE:
        IsWrite = WritePlayerDie(Type, (stDB_GAME_WRITE_PLAYER_DIE_in *)p_In);
        break;
    case enDB_GAME_WRITE_PLAYER_KILL:
        IsWrite = WritePlayerKill(Type, (stDB_GAME_WRITE_PLAYER_KILL_in *)p_In);
        break;
    default:
        CrashDump::Crash();
        return false;
        break;
    }
    return IsWrite;
}

void CDBGame::WriteLog(const en_DB_ACTION_TYPE Type, const LPVOID p_In)
{
    if (nullptr == _p_DBLog)
    {
        CrashDump::Crash();
        return;
    }
    if (false == _p_DBLog->WriteDB(Type, p_In))
    {
        // �α� ���� ��.
        SYSLOG(L"DBGame", LOG_ERROR, L"WriteLog Failed # [Type:%d]", Type);
    }
}

// gamedb ���� �÷��̾� ������ �ִ��� Ȯ��, ���ٸ� ����.
// Out�� ����. -> guestkill �������� �ɷ� ��ġ.
bool CDBGame::ReadPlayerCheck(const en_DB_ACTION_TYPE Type, const stDB_GAME_READ_PLAYER_CHECK_in *p_PlayerCheckIn, stDB_GAME_READ_PLAYER_CHECK_out * p_PlayerCheckOut)
{
    if (nullptr == p_PlayerCheckIn || nullptr == p_PlayerCheckOut)
        return false;

    MYSQL_ROW Row;

    p_PlayerCheckOut->GuestKillCount = 0;

    if (false == QuerySave(L"INSERT INTO `gamedb`.`player` ( `accountno` ) SELECT %lld FROM DUAL WHERE NOT EXISTS ( SELECT `accountno` FROM  `gamedb`.`player` WHERE `accountno` = %lld );",
        p_PlayerCheckIn->AccountNo, p_PlayerCheckIn->AccountNo))
    {
        // DB ���ӿ� ������ �ִ�.
        CrashDump::Crash();
        return false;
    }

    if (false == QuerySelect(L"SELECT `guest_kill` from `gamedb`.`player` where accountno = %lld;"
        , p_PlayerCheckIn->AccountNo))
    {
        // DB ���ӿ� ������ �ִ�.
        CrashDump::Crash();
        return false;
    }

    Row = FetchRow();
    if (nullptr == Row)
    {
        return false;
    }

    p_PlayerCheckOut->GuestKillCount = atoi(Row[0]);
    FreeResult();

    return true;
}
bool CDBGame::WritePlayerDie(const en_DB_ACTION_TYPE Type, const stDB_GAME_WRITE_PLAYER_DIE_in *p_PlayerDieIn)
{
    if (nullptr == p_PlayerDieIn)
        return false;

    if (false == QuerySave(L"UPDATE `gamedb`.`player` SET `total_die` = `total_die` + 1 where `accountno` = %lld;",
        p_PlayerDieIn->AccountNo))
    {
        // DB ���ӿ� ������ �ִ�.
        CrashDump::Crash();
        return false;
    }

    // �α׸� �����.
    WriteLog(Type, (LPVOID)p_PlayerDieIn);
    return true;
}
bool CDBGame::WritePlayerKill(const en_DB_ACTION_TYPE Type, const stDB_GAME_WRITE_PLAYER_KILL_in *p_PlayerKillIn)
{
    if (nullptr == p_PlayerKillIn)
        return false;
    
    if (true == p_PlayerKillIn->IsGuestTarget)
    {
        ////if (false == QuerySave(L"UPDATE `gamedb`.`player` SET `total_kill` = `total_kill` + 1, `guest_kill` = `guest_kill` + 1 where `accountno` = %lld;",
        ////    p_PlayerKillIn->AccountNo))
        //if (false == QuerySave(L"UPDATE `gamedb`.`player` SET `guest_kill` = `guest_kill` + 1 where `accountno` = %lld;",
        //    p_PlayerKillIn->AccountNo))
        //{
        //    // DB ���ӿ� ������ �ִ�.
        //    CrashDump::Crash();
        //    return false;
        //}
    }
    else
    {
        if (false == QuerySave(L"UPDATE `gamedb`.`player` SET `total_kill` = `total_kill` + 1 where `accountno` = %lld;",
            p_PlayerKillIn->AccountNo))
        {
            // DB ���ӿ� ������ �ִ�.
            CrashDump::Crash();
            return false;
        }
    }

    // �α׸� �����.
    WriteLog(Type, (LPVOID)p_PlayerKillIn);
    return true;
}