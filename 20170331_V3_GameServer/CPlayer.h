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
    st_SECTOR                                   **_pp_Sector;           // ���� 2���� �迭

    //------------------------------------------------
    // Time
    //------------------------------------------------
    TimeManager                                 *_TimeManager;

public:
    //--------------------------------------
    // UnderAttack Ÿ�̸�
    //--------------------------------------
    static ULONGLONG        UnderAttackLastTime;

    //--------------------------------------
    // ������ ���� ������
    //--------------------------------------
    OBJECT_ID               _ObjectID;              // ������ ó���� �� �÷��̾��� ������

    // Login �� �� ����
    bool                    _LoginFlag;             // �α��� ���Ŀ� Auth Logic�� ������ �� �ʿ�.
    BYTE                    _Party;                 // ��Ƽ ������(default:0, dfGAME_LOGIN_PARTY1, dfGAME_LOGIN_PARTY2)

    GAME_USER_TYPE_DEFINE   _UserType;
    INT64                   _AccountNo;
    WCHAR                   _ID[256];
    WCHAR                   _Nickname[256];

    // ĳ���� ������ �� ����(Pos, Tile, Sector, hp�� �� �� ����)
    BYTE                    _CharacterType;         // ĳ���� Ÿ�� ������(GAME_CHARACTER_TYPE_DEFINE)

    //--------------------------------------
    // ���� ���ο��� �� ������
    //--------------------------------------

    // Game Update ���� �ð�. �� �����Ӵ� �� �ð��� �������� ������ ó���Ѵ�.
    ULONGLONG               _UpdateTime;

    // ���� ���ӽ� ���ݺҰ� �÷���
    bool                    _IsFirstJoin;

    // GameUpdate���� �����ϴ� �÷���
    bool                    _IsMove;
    bool                    _IsAttack;
    //bool                    _IsFind;                // �̰Ŵ� ���������� ���� �� ��.

    // �÷��̾��� ������
    int                     _DestTileX;
    int                     _DestTileY;

    // Delay ����
    ULONGLONG               _MoveDelay;             // �̵� ������ �ð�
    ULONGLONG               _AttackDelay;           // ���� ������ �ð�

    // �̵� ����
    bool                    _IsActionReset;

    int                     _PathSize;
    int                     _PathIndex;
    //st_PATH                 _Path[dfPATH_POINT_MAX];
    st_TILE_PATH            _TilePath[dfPATH_POINT_MAX];

    ULONGLONG               _RemainMoveTime;        // ���� ������ ���� �̵��ð�.
    ULONGLONG               _NextMoveTime;          // �̵�ó���� ���� �ð�.

    // ��ǥ ����
    int                     _TileX;                 // �������� ������ �Ǵ� ��ǥ
    int                     _TileY;
    short                   _SectorX;
    short                   _SectorY;
    short                   _OldSectorX;            // SendPacket_SectorUpdate() �뵵
    short                   _OldSectorY;

    // ���� ����
    Player                  *_p_Target;             // ���ݴ�� �����
    OBJECT_ID               _TargetID;              // ���ݴ�� ���� ������Ʈ ���̵�
    int                     _TargetTileX;           // ���ݴ�� ���� ��ǥ
    int                     _TargetTileY;           // ���ݴ�� ���� ��ǥ
    GAME_ATTACK_TYPE        _AttackType;

    ULONGLONG               _NextAttackTime;        // ���� ��Ÿ��
    ULONGLONG               _AttackMoveCheckTime;   // ���� �̵��� ��ã�� ��Ÿ��

    //--------------------------------------
    // Ŭ��� �����ϴ� ������
    //--------------------------------------
    float                   _PosX;                  // Ŭ�� ��ǥ
    float                   _PosY;
    USHORT                  _Rotation;
    int                     _Cristal;               // �̻��
    int                     _HP;
    INT64                   _Exp;                   // �̻��
    USHORT                  _Level;                 // �̻��

    int                     _KillCount;             // �÷��̾��� �� ų��.
    int                     _GuestKillCount;        // �÷��̾��� �� �Խ�Ʈ ų��.(�̰� �����ؾ���.)

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
    bool PacketProc_CharacterSelect(Packet *p_Packet);      // game mode�� ��ȯ�� ĳ���� ���� ��Ŷ �ް� �ؾ��� ��.

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
    // Player�� GameUpdate �����Լ�.
    void GameUpdate(ULONGLONG CurTime);

    // Action ����
    void Action_Move(void);
    //void Action_Move_Advanced(void);
    void Action_Attack(void);

    // Action Reset ����
    //void ActionResetProc(void);
    void ActionReset(void);
    void MoveReset(void);
    void AttackReset(void);

    // �ð� ���� ����
    void ResetRemainMoveTime(void);
    void StartRemainMoveTime(void);
    void EndRemainMoveTime(void);

    // Path ����
    bool FindMovePath(void);
    bool FindAttackPath(void);

    // Attack ����
    bool IsAttackRange(int TargetTileX, int TargetTileY);
    bool IsAttackMove(int TargetTileX, int TargetTileY);
    bool GetTileTarget(int TargetTileX, int TargetTileY, st_SECTOR_TARGET *SectorTarget);
    void SectorAttackProc(st_SECTOR_TARGET *SectorTarget, short SectorX, short SectorY, int Damage, bool IsPush);
    void AreaAttackProc(int CenterTileX, int CenterTileY, int Damage, bool IsPush = true);
    void SingleAttackProc(Player *p_Target, int Damage, bool IsPush = true);
    void AttackProc(Player *p_Target, st_DAMAGE *p_Damage, bool IsPush);

    // Delay ����
    bool CheckMoveDelay(void);
    bool CheckAttackDelay(void);
    void SetStunDelay(ULONGLONG Delay);
    void SetMoveDelay(ULONGLONG Delay);
    void SetAttackDelay(ULONGLONG Delay);

    // Pos �� Tile ����
    bool SetPosition(float PosX, float PosY);           // Pos ���� + Ÿ��, ���ͱ��� ����.
    bool TileUpdate(int NewTileX, int NewTileY);        // Tile ����.

    bool RemoveTileObject(Player *p_Object, int TileX, int TileY, int ObjectType);
    bool AddTileObject(Player *p_Object, int TileX, int TileY, int ObjectType);

    // Sector ����
    bool SectorUpdate(void);                            // Sector ����.
    void SendPacket_SectorUpdate(void);                 // SectorUpdate�� true��� ȣ��.
    void SendPacket_SectorUpdateNewPlayer(void);        // ���ο� ���� ���� �Լ�.
    bool LeaveSector(short SectorX, short SectorY);
    bool EnterSector(short SectorX, short SectorY);
    void GetSectorAround(short SectorX, short SectorY, st_SECTOR_AROUND *p_SectorAround);
    void GetSectorUpdateAround(short OldSectorX, short OldSectorY, short NewSectorX, short NewSectorY, st_SECTOR_AROUND *p_RemoveSectorAround, st_SECTOR_AROUND *p_AddSectorAround);
    void GetNewSectorByTheDirection(short OldSectorX, short OldSectorY, short NewSectorX, short NewSectorY, st_SECTOR_AROUND *p_NewSector);

    // Player ����
    Player *FindPlayer(OBJECT_ID ObjectID);

    // Send ����
    void SendPacket_SectorOne(Packet *p_Packet, short SectorX, short SectorY, Player *p_ExceptPlayer);
    void SendPacket_SectorAround(Packet *p_Packet, Player *p_ExceptPlayer = nullptr);
    void SendPacket_SectorAround(Packet *p_Packet, short SectorX, short SectorY, Player *p_ExceptPlayer);

    // Account DB ����
    bool DBAccountWriteStatusLogout(void);

    // Game DB ����
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