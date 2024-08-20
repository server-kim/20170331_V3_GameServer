#ifndef __DB_ACCOUNT_HEADER__
#define __DB_ACCOUNT_HEADER__

class CDBAccount : public CDBConnector
{
private:
    CDBLog              *_p_DBLog;      // DBLog

public:
    CDBAccount(WCHAR *p_DBIP, WCHAR *p_DBUser, WCHAR *p_DBPassword, WCHAR *p_DBName, int DBPort);
    virtual ~CDBAccount(void);

    bool ReadDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In, LPVOID p_Out);
    bool WriteDB(const en_DB_ACTION_TYPE Type, const LPVOID p_In);

    void SetDBLog(CDBLog *p_DBLog) { _p_DBLog = p_DBLog; }

private:
    void WriteLog(const en_DB_ACTION_TYPE Type, const LPVOID p_In);

    bool ResetStatusAll(const en_DB_ACTION_TYPE Type);
    bool ReadUser(const en_DB_ACTION_TYPE Type, const stDB_ACCOUNT_READ_USER_in *p_UserIn, stDB_ACCOUNT_READ_USER_out *p_UserOut);
    bool WriteStatusLogout(const en_DB_ACTION_TYPE Type, const stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *p_StatusLogoutIn);
};

#endif