#ifndef __GAME_SERVER_CONFIG_HEADER__
#define __GAME_SERVER_CONFIG_HEADER__

// 게임서버 설정파일 데이터 및 로딩을 담당한다.
class CGameServerConfig
{
public:
    // :NETWORK
    WCHAR               _ServerName[SERVER_NAME_LEN + 1];

    WCHAR               _BindIP[IP_V4_MAX_LEN + 1];
    USHORT              _BindPort;

    WCHAR               _LoginServerIP[IP_V4_MAX_LEN + 1];
    USHORT              _LoginServerPort;

    WCHAR               _MonitoringServerIP[IP_V4_MAX_LEN + 1];
    USHORT              _MonitoringServerPort;

    WCHAR               _AgentIP[IP_V4_MAX_LEN + 1];
    USHORT              _AgentPort;

    int                 _WorkerThreadCnt;

    // :SYSTEM
    int                 _Version;
    int                 _ClientMax;
    BYTE                _PacketCode;
    BYTE                _PacketKey1;
    BYTE                _PacketKey2;
    WCHAR               _LogLevelStr[WParser::WORD_SIZE + 1];
    LOG_LEVEL_Define    _LogLevel;

    //-----------------------------------
    // :DATABASE
    //-----------------------------------
    // AccountDB 정보
    //-----------------------------------
    WCHAR       _AccountIP[IP_V4_MAX_LEN + 1];
    USHORT      _AccountPort;
    WCHAR       _AccountUser[DB_USER_MAX_LEN + 1];
    WCHAR       _AccountPassword[DB_PASSWORD_MAX_LEN + 1];
    WCHAR       _AccountDBName[DB_NAME_MAX_LEN + 1];

    //-----------------------------------
    // GameDB 정보
    //-----------------------------------
    WCHAR       _GameIP[IP_V4_MAX_LEN + 1];
    USHORT      _GamePort;
    WCHAR       _GameUser[DB_USER_MAX_LEN + 1];
    WCHAR       _GamePassword[DB_PASSWORD_MAX_LEN + 1];
    WCHAR       _GameDBName[DB_NAME_MAX_LEN + 1];

    //-----------------------------------
    // Game Log DB 정보
    //-----------------------------------
    WCHAR       _LogIP[IP_V4_MAX_LEN + 1];
    USHORT      _LogPort;
    WCHAR       _LogUser[DB_USER_MAX_LEN + 1];
    WCHAR       _LogPassword[DB_PASSWORD_MAX_LEN + 1];
    WCHAR       _LogDBName[DB_NAME_MAX_LEN + 1];

    CGameServerConfig(void);
    virtual ~CGameServerConfig(void);

    bool ParseINI(const WCHAR *p_FileName);

};

#endif