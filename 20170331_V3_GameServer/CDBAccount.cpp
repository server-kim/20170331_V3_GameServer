#include "stdafx.h"
#include "TotalHeader.h"

CDBAccount::CDBAccount(WCHAR *p_DBIP, WCHAR *p_DBUser, WCHAR *p_DBPassword, WCHAR *p_DBName, int DBPort) : CDBConnector(p_DBIP, p_DBUser, p_DBPassword, p_DBName, DBPort)
{
    _p_DBLog = nullptr;
}
CDBAccount::~CDBAccount(void)
{

}

bool CDBAccount::ReadDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In, LPVOID p_Out)
{
    bool IsRead = false;
    switch (Type)
    {
    case enDB_ACCOUNT_READ_RESET_STATUS_ALL:
        IsRead = ResetStatusAll(Type);
        break;
    case enDB_ACCOUNT_READ_USER:
        IsRead = ReadUser(Type, (stDB_ACCOUNT_READ_USER_in *)p_In, (stDB_ACCOUNT_READ_USER_out *)p_Out);
        break;
    default:
        CrashDump::Crash();
        return false;
        break;
    }
    return IsRead;
}
bool CDBAccount::WriteDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In)
{
    bool IsWrite = false;
    switch (Type)
    {
    case enDB_ACCOUNT_WRITE_STATUS_LOGOUT:
        IsWrite = WriteStatusLogout(Type, (stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *)p_In);
        break;
    default:
        CrashDump::Crash();
        return false;
        break;
    }
    return IsWrite;
}

void CDBAccount::WriteLog(const en_DB_ACTION_TYPE Type, const LPVOID p_In)
{
    if (nullptr == _p_DBLog)
    {
        CrashDump::Crash();
        return;
    }
    if (false == _p_DBLog->WriteDB(Type, p_In))
    {
        // 로그 남길 것.
        SYSLOG(L"DBAccount", LOG_ERROR, L"WriteLog Failed # [Type:%d]", Type);
    }
}

// Out은 없다.
bool CDBAccount::ResetStatusAll(const en_DB_ACTION_TYPE Type)
{
    if (false == QuerySave(L"update accountdb.status set status = 0;"))
    {
        // DB 접속에 문제가 있다.
        CrashDump::Crash();
        return false;
    }
    return true;
}
// accountdb 에서 회원 정보 얻기 & status login 상태로
bool CDBAccount::ReadUser(const en_DB_ACTION_TYPE Type, const stDB_ACCOUNT_READ_USER_in *p_UserIn, stDB_ACCOUNT_READ_USER_out *p_UserOut)
{
    if (nullptr == p_UserIn || nullptr == p_UserOut)
        return false;

    MYSQL_ROW Row;
    int PartyNum;

    // 초기값은 로그인 실패.
    p_UserOut->Status = dfGAME_LOGIN_FAIL;

    //-----------------------------------------
    // Status Check
    //-----------------------------------------
    if (false == QuerySelect(L"select status from accountdb.status where accountno = %lld;"
        , p_UserIn->AccountNo))
    {
        // DB 접속에 문제가 있다.
        CrashDump::Crash();
        return false;
    }
    
    Row = FetchRow();
    if (nullptr == Row)
    {
        return false;
    }
    
    if (atoi(Row[0]) != 0)
    {
        // 중복 접속을 처리하여야 하는데 이미 플레이하는 유저를 끊어야 할듯.
    
        FreeResult();
        p_UserOut->Status = dfGAME_LOGIN_DUPLICATE;
        return false;
    }
    FreeResult();
    
    //-----------------------------------------
    // Status Change
    //-----------------------------------------
    // status를 login 상태로 바꾼다.
    if (false == QuerySave(L"update accountdb.status set status = 1 where accountno = %lld;",
        p_UserIn->AccountNo))
    {
        // DB 접속에 문제가 있다.
        CrashDump::Crash();
        return false;
    }
    
    //-----------------------------------------
    // Data Loading
    //-----------------------------------------
    //-----------------------------------------
    // Account Select
    //-----------------------------------------
    if (false == QuerySelect(L"select userid, usernick, gamecodi_party from accountdb.account where accountno = %lld;"
        , p_UserIn->AccountNo))
    {
        // DB 접속에 문제가 있다.
        CrashDump::Crash();
        return false;
    }

    Row = FetchRow();
    if (nullptr == Row)
    {
        return false;
    }

    // 일단 밖에서 결정한다.
    if (false == UTF8toUTF16(Row[0], p_UserOut->szID, ID_MAX_LEN))
    {
        FreeResult();
        return false;
    }
    if (false == UTF8toUTF16(Row[1], p_UserOut->szNick, NICK_MAX_LEN))
    {
        FreeResult();
        return false;
    }

    PartyNum = atoi(Row[2]);
    if (PartyNum != dfGAME_LOGIN_PARTY1 && PartyNum != dfGAME_LOGIN_PARTY2)
    {
        FreeResult();
        return false;
    }
    p_UserOut->Party = (BYTE)PartyNum;
    FreeResult();

    // Status를 갱신한다.(현재 status는 파티 넘버다.)
    p_UserOut->Status = p_UserOut->Party;

    // 로그인 로그를 남긴다.
    WriteLog(Type, (LPVOID)p_UserIn);

    return true;
}
// 플레이어 로그아웃시 status 를 로그아웃 상태로 변경
bool CDBAccount::WriteStatusLogout(const en_DB_ACTION_TYPE Type, const stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *p_StatusLogoutIn)
{
    if (nullptr == p_StatusLogoutIn)
        return false;

    //-----------------------------------------
    // Status Change
    //-----------------------------------------
    if (false == QuerySave(L"update accountdb.status set status = 0 where accountno = %lld;",
        p_StatusLogoutIn->AccountNo))
    {
        // DB 접속에 문제가 있다.
        CrashDump::Crash();
        return false;
    }
    return true;
}