#ifndef __DB_GAME_HEADER__
#define __DB_GAME_HEADER__

class CDBGame : public CDBConnector
{
private:
    CDBLog              *_p_DBLog;      // DBLog

public:
    CDBGame(WCHAR *p_DBIP, WCHAR *p_DBUser, WCHAR *p_DBPassword, WCHAR *p_DBName, int DBPort);
    virtual ~CDBGame(void);

    bool ReadDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In, LPVOID p_Out);
    bool WriteDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In);

    void SetDBLog(CDBLog *p_DBLog) { _p_DBLog = p_DBLog; }

private:
    void WriteLog(const en_DB_ACTION_TYPE Type, const LPVOID p_In);

    bool ReadPlayerCheck(const en_DB_ACTION_TYPE Type, const stDB_GAME_READ_PLAYER_CHECK_in *p_PlayerCheckIn, stDB_GAME_READ_PLAYER_CHECK_out * p_PlayerCheckOut);
    bool WritePlayerDie(const en_DB_ACTION_TYPE Type, const stDB_GAME_WRITE_PLAYER_DIE_in *p_PlayerDieIn);
    bool WritePlayerKill(const en_DB_ACTION_TYPE Type, const stDB_GAME_WRITE_PLAYER_KILL_in *p_PlayerKillIn);

};

#endif