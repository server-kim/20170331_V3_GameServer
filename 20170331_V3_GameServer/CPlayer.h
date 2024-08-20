#ifndef __PLAYER_HEADER__
#define __PLAYER_HEADER__

class Player : public MMOSession
{
private:
    CGameServer                                 *_p_GameServer;

    int                                         _MaxSession;
    Player                                      *_p_PlayerArray;

    //------------------------------------------------
    // Map
    //------------------------------------------------
    CMAP                                        *_p_Map;
    CGamePathFinder                             *_p_PathFinder;

    st_MAP_TILE                                 **_pp_MapTile;
    st_SECTOR                                   **_pp_Sector;           // 섹터 2차원 배열

    //------------------------------------------------
    // Time
    //------------------------------------------------
    TimeManager                                 *_TimeManager;

public:
    //--------------------------------------
    // UnderAttack 타이머
    //--------------------------------------
    static ULONGLONG        UnderAttackLastTime;

    //--------------------------------------
    // 유저의 고유 데이터
    //--------------------------------------
    OBJECT_ID               _ObjectID;              // 컨텐츠 처리할 때 플레이어의 고유값

    // Login 할 때 세팅
    bool                    _LoginFlag;             // 로그인 이후에 Auth Logic을 수행할 때 필요.
    BYTE                    _Party;                 // 파티 고유값(default:0, dfGAME_LOGIN_PARTY1, dfGAME_LOGIN_PARTY2)

    GAME_USER_TYPE_DEFINE   _UserType;
    INT64                   _AccountNo;
    WCHAR                   _ID[256];
    WCHAR                   _Nickname[256];

    // 캐릭터 선택할 때 세팅(Pos, Tile, Sector, hp가 이 때 세팅)
    BYTE                    _CharacterType;         // 캐릭터 타입 고유값(GAME_CHARACTER_TYPE_DEFINE)

    //--------------------------------------
    // 서버 내부에서 쓸 데이터
    //--------------------------------------

    // Game Update 기준 시간. 한 프레임당 이 시간을 기준으로 로직을 처리한다.
    ULONGLONG               _UpdateTime;

    // 최초 접속시 공격불가 플래그
    bool                    _IsFirstJoin;

    // GameUpdate에서 설정하는 플래그
    bool                    _IsMove;
    bool                    _IsAttack;
    //bool                    _IsFind;                // 이거는 지역변수로 빼도 될 듯.

    // 플레이어의 목적지
    int                     _DestTileX;
    int                     _DestTileY;

    // Delay 관련
    ULONGLONG               _MoveDelay;             // 이동 딜레이 시간
    ULONGLONG               _AttackDelay;           // 공격 딜레이 시간

    // 이동 관련
    bool                    _IsActionReset;

    int                     _PathSize;
    int                     _PathIndex;
    //st_PATH                 _Path[dfPATH_POINT_MAX];
    st_TILE_PATH            _TilePath[dfPATH_POINT_MAX];

    ULONGLONG               _RemainMoveTime;        // 서버 보정용 이전 이동시간.
    ULONGLONG               _NextMoveTime;          // 이동처리할 시작 시간.

    // 좌표 관련
    int                     _TileX;                 // 서버에서 기준이 되는 좌표
    int                     _TileY;
    short                   _SectorX;
    short                   _SectorY;
    short                   _OldSectorX;            // SendPacket_SectorUpdate() 용도
    short                   _OldSectorY;

    // 공격 관련
    Player                  *_p_Target;             // 공격대상 저장용
    OBJECT_ID               _TargetID;              // 공격대상 이전 오브젝트 아이디
    int                     _TargetTileX;           // 공격대상 이전 좌표
    int                     _TargetTileY;           // 공격대상 이전 좌표
    GAME_ATTACK_TYPE        _AttackType;

    ULONGLONG               _NextAttackTime;        // 공격 쿨타임
    ULONGLONG               _AttackMoveCheckTime;   // 공격 이동시 길찾기 쿨타임

    //--------------------------------------
    // 클라와 공유하는 데이터
    //--------------------------------------
    float                   _PosX;                  // 클라 좌표
    float                   _PosY;
    USHORT                  _Rotation;
    int                     _Cristal;               // 미사용
    int                     _HP;
    INT64                   _Exp;                   // 미사용
    USHORT                  _Level;                 // 미사용

    int                     _KillCount;             // 플레이어의 총 킬수.
    int                     _GuestKillCount;        // 플레이어의 총 게스트 킬수.(이건 구현해야함.)

public:
    Player(void);
    virtual ~Player(void);

    void SetGameServer(CGameServer *p_GameServer) { _p_GameServer = p_GameServer; }
    void SetGameData(int MaxSession, Player *p_PlayerArray, CMAP *p_Map, CGamePathFinder *p_PathFinder, st_MAP_TILE **pp_MapTile, st_SECTOR **pp_Sector, TimeManager *p_TimeManager)
    {
        _MaxSession = MaxSession;
        _p_PlayerArray = p_PlayerArray;
        _p_Map = p_Map;
        _p_PathFinder = p_PathFinder;
        _pp_MapTile = pp_MapTile;
        _pp_Sector = pp_Sector;
        _TimeManager = p_TimeManager;
    }

    //------------------------------------------
    // auth packet proc
    //------------------------------------------
    bool PacketProc_Login(Packet *p_Packet);
    bool PacketProc_CharacterSelect(Packet *p_Packet);      // game mode로 전환을 캐릭터 선택 패킷 받고 해야할 듯.

    Packet *PacketMake_Login(BYTE Status, INT64 AccountNo);
    Packet *PacketMake_CharacterSelect(BYTE Status);

    //------------------------------------------
    // game packet proc
    //------------------------------------------
    bool PacketProc_MoveCharacter(Packet *p_Packet);
    bool PacketProc_StopCharacter(Packet *p_Packet);
    bool PacketProc_Attack1Target(Packet *p_Packet);
    bool PacketProc_Attack2Target(Packet *p_Packet);
    bool CheckAttackTarget(OBJECT_ID TargetID);

    Packet *PacketMake_CreateMyCharacter(void);
    Packet *PacketMake_CreateOtherCharacter(Player *p_Other, BYTE Respawn);
    Packet *PacketMake_RemoveObject(OBJECT_ID ObjectID);

    Packet *PacketMake_MoveCharacter(Player *p_Player);
    Packet *PacketMake_StopCharacter(OBJECT_ID ObjectID, float X, float Y, USHORT Rotation);
    Packet *PacketMake_CharacterSync(OBJECT_ID ObjectID, float X, float Y);

    Packet *PacketMake_Attack(BYTE AttackType, OBJECT_ID AttackID, OBJECT_ID TargetID, int CoolTime, float AttackPosX, float AttackPosY);
    Packet *PacketMake_Damage(OBJECT_ID TargetID, int DamageValue, float PushPosX, float PushPosY);
    Packet *PacketMake_Die(OBJECT_ID DeadID);

    Packet *PacketMake_DamageGroup(st_DAMAGE *p_Damage, BYTE Count);
    Packet *PacketMake_PlayerHP(int HP);
    Packet *PacketMake_UnderattackPos(float PosX, float PosY);
    Packet *PacketMake_BotKill(void);
    //Packet *PacketMake_KillRank(BYTE RankDataSize, st_RANK_DATA *p_RankData);
    //Packet *PacketMake_PlayerPosAlert(st_SECTOR_POS *p_Sector, BYTE Count);

    //------------------------------------------
    // Contents proc
    //------------------------------------------
    // Player의 GameUpdate 메인함수.
    void GameUpdate(ULONGLONG CurTime);

    // Action 관련
    void Action_Move(void);
    //void Action_Move_Advanced(void);
    void Action_Attack(void);

    // Action Reset 관련
    //void ActionResetProc(void);
    void ActionReset(void);
    void MoveReset(void);
    void AttackReset(void);

    // 시간 보정 관련
    void ResetRemainMoveTime(void);
    void StartRemainMoveTime(void);
    void EndRemainMoveTime(void);

    // Path 관련
    bool FindMovePath(void);
    bool FindAttackPath(void);

    // Attack 관련
    bool IsAttackRange(int TargetTileX, int TargetTileY);
    bool IsAttackMove(int TargetTileX, int TargetTileY);
    bool GetTileTarget(int TargetTileX, int TargetTileY, st_SECTOR_TARGET *SectorTarget);
    void SectorAttackProc(st_SECTOR_TARGET *SectorTarget, short SectorX, short SectorY, int Damage, bool IsPush);
    void AreaAttackProc(int CenterTileX, int CenterTileY, int Damage, bool IsPush = true);
    void SingleAttackProc(Player *p_Target, int Damage, bool IsPush = true);
    void AttackProc(Player *p_Target, st_DAMAGE *p_Damage, bool IsPush);

    // Delay 관련
    bool CheckMoveDelay(void);
    bool CheckAttackDelay(void);
    void SetStunDelay(ULONGLONG Delay);
    void SetMoveDelay(ULONGLONG Delay);
    void SetAttackDelay(ULONGLONG Delay);

    // Pos 및 Tile 관련
    bool SetPosition(float PosX, float PosY);           // Pos 변경 + 타일, 섹터까지 갱신.
    bool TileUpdate(int NewTileX, int NewTileY);        // Tile 갱신.

    bool RemoveTileObject(Player *p_Object, int TileX, int TileY, int ObjectType);
    bool AddTileObject(Player *p_Object, int TileX, int TileY, int ObjectType);

    // Sector 관련
    bool SectorUpdate(void);                            // Sector 갱신.
    void SendPacket_SectorUpdate(void);                 // SectorUpdate가 true라면 호출.
    void SendPacket_SectorUpdateNewPlayer(void);        // 새로운 유저 전용 함수.
    bool LeaveSector(short SectorX, short SectorY);
    bool EnterSector(short SectorX, short SectorY);
    void GetSectorAround(short SectorX, short SectorY, st_SECTOR_AROUND *p_SectorAround);
    void GetSectorUpdateAround(short OldSectorX, short OldSectorY, short NewSectorX, short NewSectorY, st_SECTOR_AROUND *p_RemoveSectorAround, st_SECTOR_AROUND *p_AddSectorAround);
    void GetNewSectorByTheDirection(short OldSectorX, short OldSectorY, short NewSectorX, short NewSectorY, st_SECTOR_AROUND *p_NewSector);

    // Player 관련
    Player *FindPlayer(OBJECT_ID ObjectID);

    // Send 관련
    void SendPacket_SectorOne(Packet *p_Packet, short SectorX, short SectorY, Player *p_ExceptPlayer);
    void SendPacket_SectorAround(Packet *p_Packet, Player *p_ExceptPlayer = nullptr);
    void SendPacket_SectorAround(Packet *p_Packet, short SectorX, short SectorY, Player *p_ExceptPlayer);

    // Account DB 관련
    bool DBAccountWriteStatusLogout(void);

    // Game DB 관련
    bool DBGameWriteLogJoin(void);
    bool DBGameWriteLogLeave(void);
    bool DBGameWritePlayerDie(__int64 AttackerAccountNo);
    bool DBGameWritePlayerKill(__int64 TargetAccountNo, int TargetTileX, int TargetTileY, bool IsGuestTarget);

    bool DBGameWriteDieLog(__int64 AttackerAccountNo);
    bool DBGameWriteKillLog(__int64 TargetAccountNo, int TargetTileX, int TargetTileY, bool IsGuestTarget);

    virtual void OnAuth_ClientJoin(void);
    virtual void OnAuth_ClientLeave(bool ToGame);
    virtual void OnAuth_Packet(Packet *p_Packet);
    virtual void OnAuth_Timeout(void);

    virtual void OnGame_ClientJoin(void);
    virtual void OnGame_ClientLeave(void);
    virtual void OnGame_Packet(Packet *p_Packet);
    virtual void OnGame_Timeout(void);

    virtual void OnGame_ClientRelease(void);
};

#endif