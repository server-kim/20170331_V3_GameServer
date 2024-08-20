#include "stdafx.h"
#include "TotalHeader.h"

CGameServerConfig::CGameServerConfig(void)
{

}
CGameServerConfig::~CGameServerConfig(void)
{

}

bool CGameServerConfig::ParseINI(const WCHAR *p_FileName)
{
    //-----------------------------------------------------------------
    // 설정파일 로드
    //-----------------------------------------------------------------
    WParser MyParser;

    HRESULT hResult;

    int IntBuf;
    WCHAR WordBuf[WParser::WORD_SIZE + 1];

    //---------------------------------------------------------------
    // Config File Loading
    //---------------------------------------------------------------
    if (false == MyParser.Parser_LoadFile(p_FileName))
        return false;

    //---------------------------------------------------------------
    // Network Parsing
    //---------------------------------------------------------------
    if (false == MyParser.Parser_ProvideArea(L"NETWORK"))
        return false;

    // server name
    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"SERVER_NAME", WordBuf))
        return false;
    if (wcslen(WordBuf) > SERVER_NAME_LEN)
        return false;
    hResult = StringCchCopyW(_ServerName, SERVER_NAME_LEN + 1, WordBuf);
    if (FAILED(hResult))
        return false;

    // bind ip & port
    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"BIND_IP", WordBuf))
        return false;
    if (false == StrToIP(WordBuf, _BindIP))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"BIND_PORT", &IntBuf))
        return false;
    if (false == IntToUShort(IntBuf, &_BindPort))
        return false;

    // login server
    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"LOGIN_SERVER_IP", WordBuf))
        return false;
    if (false == StrToIP(WordBuf, _LoginServerIP))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"LOGIN_SERVER_PORT", &IntBuf))
        return false;
    if (false == IntToUShort(IntBuf, &_LoginServerPort))
        return false;

    // monitoring server
    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"MONITORING_SERVER_IP", WordBuf))
        return false;
    if (false == StrToIP(WordBuf, _MonitoringServerIP))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"MONITORING_SERVER_PORT", &IntBuf))
        return false;
    if (false == IntToUShort(IntBuf, &_MonitoringServerPort))
        return false;

    // Agent
    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"AGENT_IP", WordBuf))
        return false;
    if (false == StrToIP(WordBuf, _AgentIP))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"AGENT_PORT", &IntBuf))
        return false;
    if (false == IntToUShort(IntBuf, &_AgentPort))
        return false;

    // worker thread cnt
    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"WORKER_THREAD", &IntBuf))
        return false;
    _WorkerThreadCnt = IntBuf;

    //---------------------------------------------------------------
    // System Parsing
    //---------------------------------------------------------------
    if (false == MyParser.Parser_ProvideArea(L"SYSTEM"))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"VERSION", &IntBuf))
        return false;
    _Version = IntBuf;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"CLIENT_MAX", &IntBuf))
        return false;
    _ClientMax = IntBuf;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"PACKET_CODE", &IntBuf))
        return false;
    if (false == IntToByte(IntBuf, &_PacketCode))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"PACKET_KEY1", &IntBuf))
        return false;
    if (false == IntToByte(IntBuf, &_PacketKey1))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"PACKET_KEY2", &IntBuf))
        return false;
    if (false == IntToByte(IntBuf, &_PacketKey2))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"LOG_LEVEL", WordBuf))
        return false;
    hResult = StringCchCopyW(_LogLevelStr, WParser::WORD_SIZE + 1, WordBuf);
    if (FAILED(hResult))
        return false;
    LOG_LEVEL_Define LogLevel = g_SystemLOG->GetLogLevel(_LogLevelStr);
    if (LOG_LEVEL_DEFAULT == LogLevel)
        return false;

    //---------------------------------------------------------------
    // Database Parsing
    //---------------------------------------------------------------
    if (false == MyParser.Parser_ProvideArea(L"DATABASE"))
        return false;

    // Account DB
    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"ACCOUNT_IP", WordBuf))
        return false;
    if (false == StrToIP(WordBuf, _AccountIP))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"ACCOUNT_PORT", &IntBuf))
        return false;
    if (false == IntToUShort(IntBuf, &_AccountPort))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"ACCOUNT_USER", WordBuf))
        return false;
    if (wcslen(WordBuf) > DB_USER_MAX_LEN)
        return false;
    hResult = StringCchCopyW(_AccountUser, DB_USER_MAX_LEN + 1, WordBuf);
    if (FAILED(hResult))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"ACCOUNT_PASSWORD", WordBuf))
        return false;
    if (wcslen(WordBuf) > DB_PASSWORD_MAX_LEN)
        return false;
    hResult = StringCchCopyW(_AccountPassword, DB_PASSWORD_MAX_LEN + 1, WordBuf);
    if (FAILED(hResult))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"ACCOUNT_DBNAME", WordBuf))
        return false;
    if (wcslen(WordBuf) > DB_NAME_MAX_LEN)
        return false;
    hResult = StringCchCopyW(_AccountDBName, DB_NAME_MAX_LEN + 1, WordBuf);
    if (FAILED(hResult))
        return false;

    // Game DB
    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"GAME_IP", WordBuf))
        return false;
    if (false == StrToIP(WordBuf, _GameIP))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"GAME_PORT", &IntBuf))
        return false;
    if (false == IntToUShort(IntBuf, &_GamePort))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"GAME_USER", WordBuf))
        return false;
    if (wcslen(WordBuf) > DB_USER_MAX_LEN)
        return false;
    hResult = StringCchCopyW(_GameUser, DB_USER_MAX_LEN + 1, WordBuf);
    if (FAILED(hResult))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"GAME_PASSWORD", WordBuf))
        return false;
    if (wcslen(WordBuf) > DB_PASSWORD_MAX_LEN)
        return false;
    hResult = StringCchCopyW(_GamePassword, DB_PASSWORD_MAX_LEN + 1, WordBuf);
    if (FAILED(hResult))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"GAME_DBNAME", WordBuf))
        return false;
    if (wcslen(WordBuf) > DB_NAME_MAX_LEN)
        return false;
    hResult = StringCchCopyW(_GameDBName, DB_NAME_MAX_LEN + 1, WordBuf);
    if (FAILED(hResult))
        return false;

    // Log DB
    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"LOG_IP", WordBuf))
        return false;
    if (false == StrToIP(WordBuf, _LogIP))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_int(L"LOG_PORT", &IntBuf))
        return false;
    if (false == IntToUShort(IntBuf, &_LogPort))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"LOG_USER", WordBuf))
        return false;
    if (wcslen(WordBuf) > DB_USER_MAX_LEN)
        return false;
    hResult = StringCchCopyW(_LogUser, DB_USER_MAX_LEN + 1, WordBuf);
    if (FAILED(hResult))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"LOG_PASSWORD", WordBuf))
        return false;
    if (wcslen(WordBuf) > DB_PASSWORD_MAX_LEN)
        return false;
    hResult = StringCchCopyW(_LogPassword, DB_PASSWORD_MAX_LEN + 1, WordBuf);
    if (FAILED(hResult))
        return false;

    MyParser.Parser_Initial();
    if (false == MyParser.Parser_GetValue_string(L"LOG_DBNAME", WordBuf))
        return false;
    if (wcslen(WordBuf) > DB_NAME_MAX_LEN)
        return false;
    hResult = StringCchCopyW(_LogDBName, DB_NAME_MAX_LEN + 1, WordBuf);
    if (FAILED(hResult))
        return false;

    return true;
}