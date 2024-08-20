#ifndef __GAME_SERVER_HEADER__
#define __GAME_SERVER_HEADER__

class CGameServer : public CMMOServerV2
{
private:
    enum GAME_SERVER_DEFINE
    {
        DB_WRITER_MESSAGE_POOL_CHUNK_SIZE = 100,

        HEART_BEAT_TICK = 500,             // worker thread 와 update thread 하트비트

        CHECK_TIMEOUT_LOGIN_SESSION_KEY = 10000,
        CHECK_TIMEOUT_CLIENT = 10000,

        TIMEOUT_LOGIN_SESSION_KEY = 20000,   // 로그인서버에서 보내준 세션키 타임아웃
        TIMEOUT_CLIENT = 10000,               // 클라이언트 타임아웃

        DB_HEARTBEAT_TICK = 1000,
    };

    struct st_LOGIN_SESSION_KEY             // 로그인 서버로부터 전달받은 세션키
    {
        __int64         AccountNo;
        BYTE            SessionKey[SESSION_KEY_BYTE_LEN];
        ULONGLONG       AuthTime;
    };

public:
    CGameServer(int MaxSession);
    virtual ~CGameServer(void);

    bool Start(WCHAR *p_ListenIP, USHORT Port, int WorkerThread, bool EnableNagle, BYTE PacketCode, BYTE PacketKey1, BYTE PacketKey2);
    bool Stop(void);

    void SetLanClientAgent(CLanClientAgent *p_LanClientAgent) { _p_LanClient_Agent = p_LanClientAgent; }
    void SetLanClientLogin(CLanClientLogin *p_LanClientLogin) { _p_LanClient_Login = p_LanClientLogin; }
    void SetLanClientMonitoring(CLanClientMonitoring *p_LanClientMonitoring) { _p_LanClient_Monitoring = p_LanClientMonitoring; }
    void SetMap(CMAP *p_Map){ _p_Map = p_Map; }
    void SetPathFinder(CGamePathFinder *p_PathFinder){ _p_PathFinder = p_PathFinder; }

    int GetUseSize_DBWriterMessageQueue(void);
    int GetAllocCount_DBWriterMessagePool(void);

    int GetLoginSessionKeyCount(void);
    void AddLoginSessionKey(__int64 AccountNo, BYTE *p_SessionKey);

private:
    bool ConnectDB(void);
    bool CheckLoginSessionKey(__int64 AccountNo, BYTE *p_SessionKey);
    void TimeoutLoginSessionKey(void);

    OBJECT_ID GetObjectID(__int64 PlayerIndex);

    // Send 관련 함수들
    void SendPacket_Broadcast(Packet *p_Packet);
    void SendPacket_Ranking(void);
    void SendPacket_PlayerPosAlert(void);
    Packet *PacketMake_KillRank(BYTE RankDataSize, st_KILL_RANK_DATA *p_RankData);
    Packet *PacketMake_PlayerPosAlert(st_SECTOR_POS *p_Sector, BYTE Count);

    //virtual void OnAuth_Update(__int64 ThreadNum);
    virtual void OnAuth_Update(void);
    virtual void OnGame_Update(void);

    virtual void OnWorker_HeartBeat(void);
    virtual void OnSend_HeartBeat(void);
    virtual void OnAuth_HeartBeat(void);
    virtual void OnGame_HeartBeat(void);

    virtual void OnError(int ErrorCode, WCHAR *p_Error);

    static unsigned __stdcall DBWriterThread(void *p_Param);
    bool DBWriterThread_Update(void);

public:
    long                                        _Monitor_SessionMiss;
    long                                        _Monitor_SessionNotFound;

    SYSTEMTIME                                  _ScheduleTime;
    st_SCHEDULE                                 _Schedule[SCHEDULE_MAX];

    ////------------------------------------------------
    //// Bot Kill Reset Scheduler
    ////------------------------------------------------
    //bool _IsBotKillResetSchedule = false;

private:
    OBJECT_ID                                   _ObjectIDIndex = 0;

    Player                                      *_PlayerArray;

    // Session Key Map(로그인 서버에서 전달한 키)
    std::map<__int64, st_LOGIN_SESSION_KEY *>   _SessionKeyMap;
    SRWLOCK                                     _SessionKeyMap_srwlock;

    // Agent 접속용도
    CLanClientAgent                             *_p_LanClient_Agent;

    // LoginServer 접속용도
    CLanClientLogin                             *_p_LanClient_Login;

    // MonitoringServer 접속용도
    CLanClientMonitoring                        *_p_LanClient_Monitoring;

    //------------------------------------------------
    // DB
    //------------------------------------------------
    CDBAccount                                  *_p_DBAccount_Auth;
    //CDBAccountTLS                               *_p_DBAccount_Auth;
    CDBAccount                                  *_p_DBAccount_Game;      // 읽기 용도
    CDBAccount                                  *_p_DBAccount_Writer;

    CDBGame                                     *_p_DBGame_Auth;
    //CDBGameTLS                                  *_p_DBGame_Auth;
    CDBGame                                     *_p_DBGame_Game;         // 읽기 용도
    CDBGame                                     *_p_DBGame_Writer;

    CDBLog                                      *_p_DBLog_Auth;
    //CDBLogTLS                                   *_p_DBLog_Auth;
    CDBLog                                      *_p_DBLog_Writer;        // 게임 스레드에서는 언제나 Writer를 이용하여 저장한다.

    //------------------------------------------------
    // DB Writer
    //------------------------------------------------
    LockfreeQueue<_st_DBWRITER_MSG *>           *_DBWriterMessageQueue;
    MemoryPoolTLS<_st_DBWRITER_MSG>             *_DBWriterMessagePool;

    HANDLE                                      _DBWriterEvent;
    HANDLE                                      _DBWriterThread;

    //------------------------------------------------
    // Map
    //------------------------------------------------
    CMAP                                        *_p_Map;
    CGamePathFinder                             *_p_PathFinder;

    st_MAP_TILE                                 **_pp_MapTile;          // 타일 2차원 배열
    st_SECTOR                                   **_pp_Sector;           // 섹터 2차원 배열

    //------------------------------------------------
    // Rank
    //------------------------------------------------
    std::list<Player *>                         _RankPlayerList;
    ULONGLONG                                   _RankTime;
    BYTE                                        _RankDataSize;
    st_KILL_RANK_DATA                           _RankData[KILL_RANK_MAX];

    //------------------------------------------------
    // Player Pos Alert
    //------------------------------------------------
    ULONGLONG                                   _PlayerPosAlertTime;
    BYTE                                        _PlayerPosAlertSectorCount;
    st_SECTOR_POS                               _PlayerPosAlertSectorPos[PLAYER_POS_ALERT_SECTOR_MAX];

    friend Player;
};

#endif