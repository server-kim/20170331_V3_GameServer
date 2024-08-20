#ifndef __DB_LOG_HEADER__
#define __DB_LOG_HEADER__

class CDBLog : public CDBConnector
{
private:
    enum LOG_TYPE : int
    {
        GAME_LOG_CONNECTION = 1,
        GAME_LOG_CHARACTER = 2
    };
    enum LOG_CODE : int
    {
        // GAME_LOG_CONNECTION
        GAME_LOGIN = 11,
        GAME_JOIN = 12,
        GAME_LEAVE = 13,

        // GAME_LOG_CHARACTER
        GAME_DIE = 21,
        GAME_KILL = 23
    };

public:
    CDBLog(WCHAR *p_DBIP, WCHAR *p_DBUser, WCHAR *p_DBPassword, WCHAR *p_DBName, int DBPort);
    virtual ~CDBLog(void);

    bool ReadDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In, LPVOID p_Out);
    bool WriteDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In);

private:
    bool WriteLogLogin(const stDB_ACCOUNT_READ_USER_in *p_LogLoginIn);
    bool WriteLogJoin(const stDB_GAME_WRITE_LOG_JOIN_in *p_LogJoinIn);
    bool WriteLogLeave(const stDB_GAME_WRITE_LOG_LEAVE_in *p_LogLeaveIn);
    bool WriteLogPlayerDie(const stDB_GAME_WRITE_PLAYER_DIE_in *p_PlayerDieIn);
    bool WriteLogPlayerKill(const stDB_GAME_WRITE_PLAYER_KILL_in *p_PlayerKillIn);

};

#endif