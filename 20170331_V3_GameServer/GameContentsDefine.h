#ifndef __GAME_CONTENTS_DEFINE_HEADER__
#define __GAME_CONTENTS_DEFINE_HEADER__

typedef __int64 OBJECT_ID;

#define NEW_OBJECT_ID(Index)                    ((InterlockedIncrement64(&_ObjectIDIndex) & 0x00ffffffffffffff) | (Index << 48))       // 새로운 오브젝트 아이디 만드는 매크로

#define OBJECT_ID_INDEX(ObjectID)               ((ObjectID >> 48) & 0xffff)               // 오브젝트 아이디 인덱스 뽑는 매크로
#define OBJECT_ID_UNIQUE(ObjectID)              (ObjectID & 0xffffffffffff)               // 오브젝트 아이디 고유번호 뽑는 매크로

enum GAME_ACCOUNTNO_DEFINE
{
    DUMMY_ACCOUNTNO_LIMIT = 999999,
    GUEST_ACCOUNTNO_OVER = 2000000
};

enum GAME_OBJECT_DEFINE
{
    OBJECT_ID_DEFAULT = -101,

    OBJECT_TYPE_PLAYER = 1
};

enum PARTY_TYPE_DEFINE
{
    PARTY_DEFAULT = 0,
    //PARTY_FIRST = dfGAME_LOGIN_PARTY1,
    //PARTY_SECOND = dfGAME_LOGIN_PARTY2
};

enum GAME_USER_TYPE_DEFINE
{
    USER_TYPE_DUMMY = 0,
    USER_TYPE_GAMECODI,
    USER_TYPE_GUEST
};

enum GAME_CHARACTER_TYPE_DEFINE
{
    CHARACTER_TYPE_DEFAULT = 0,
    //CHARACTER_TYPE_GOLEM,
    //CHARACTER_TYPE_KNIGHT,
    //CHARACTER_TYPE_ELF,
    //CHARACTER_TYPE_ORC,
    //CHARACTER_TYPE_ARCHER
};

enum GAME_ATTACK_TYPE : BYTE
{
    ATTACK_TYPE_DEFAULT = 0,
    ATTACK_TYPE_1,
    ATTACK_TYPE_2
};

enum GAME_RANK_DEFINE
{
    //RANK_DELAY = 300,
    //RANK_MAX = 10
    KILL_RANK_DELAY = 1000,
    KILL_RANK_MAX = 5
};

enum GAME_DAMAGE_DEFINE
{
    AREA_ATTACK_OBJECT_MAX_COUNT = 50
};

enum GAME_DIE_DEFINE
{
    DIE_FLAG_OFF = 0,
    DIE_FLAG_ON = 1
};

enum GAME_BROADCASTING_DEFINE
{
    UNDER_ATTACK_SEND_TIME = 1000,

    PLAYER_POS_ALERT_SEND_TIME = 10000,
    PLAYER_POS_ALERT_SECTOR_MAX = 100
};

enum GAME_SCHEDULE_DEFINE
{
    SCHEDULE_MAX = 5
};

//enum GAME_SECTOR_DEFINE
//{
//    SECTOR_TILE_WIDTH = 25,
//    SECTOR_TILE_HEIGHT = 25
//};

enum GAME_MAP_TILE_DEFINE
{
    TILE_DEFAULT_X = -1,
    TILE_DEFAULT_Y = -1,

    TILE_MAX_X = 600,
    TILE_MAX_Y = 200,

    MAP_TILE_OBJECT_MAX = 40,
    MAP_TILE_OBJECT_CHECK = 20
};

// Field 관련
struct st_TILE_OBJECT
{
    int             ObjectType;
    //__int64         ObjectID;
    Player          *p_Object;
};

struct st_MAP_TILE
{
    //int                 TileX;
    //int                 TileY;

    st_TILE_OBJECT      Object[MAP_TILE_OBJECT_MAX];
    int                 ObjectNum;

    //SRWLOCK             MapTile_srwlock;
};

// Sector 관련
struct st_SECTOR
{
    //std::list<__int64>  PlayerList;
    //std::list<OBJECT_ID>    PlayerList;
    std::list<Player *>    PlayerList;
    //SRWLOCK                 Sector_srwlock;
};

// Sector 검색 관련
struct st_SECTOR_POS
{
    short                   SectorX;
    short                   SectorY;
};

struct st_SECTOR_AROUND
{
    int                     Count;
    st_SECTOR_POS           Around[9];          // 자신의 섹터를 포함한 주변 섹터 좌표
};

// Rank 관련
#pragma pack(push, 1)
struct st_KILL_RANK_DATA
{
    WCHAR                   Nickname[NICK_MAX_LEN];
    //WCHAR                   *Nickname;
    int                     KillCount;
};
#pragma pack(pop)

// Damage 그룹 관련
#pragma pack(push, 1)
struct st_DAMAGE
{
    OBJECT_ID               DamageObjectID;
    short                   DamageValue;

    float                   PushPosX;
    float                   PushPosY;

    BYTE                    Die;
};
#pragma pack(pop)

struct st_SECTOR_TARGET
{
    int                     Count;
    //OBJECT_ID               TargetIDArr[AREA_ATTACK_OBJECT_MAX_COUNT];
    Player                  *TargetArr[AREA_ATTACK_OBJECT_MAX_COUNT];
};

struct st_SECTOR_AROUND_TARGET : st_SECTOR_AROUND
{
    //int                     Count;
    //st_SECTOR_POS           Around[9];          // 자신의 섹터를 포함한 주변 섹터 좌표

    st_SECTOR_TARGET        SectorTarget[9];

    //int                     TargetIDCount[9];
    //OBJECT_ID               TargetIDArr[9][AREA_ATTACK_OBJECT_MAX_COUNT];
};

// 스케쥴 관련
struct st_SCHEDULE
{
    bool                    IsUse;
    bool                    IsSchedule;
    WORD                    Year;               // 0이면 매년.
    WORD                    Month;              // 0이면 매달.
    //WORD wDayOfWeek;
    WORD                    Day;                // 0이면 매일.
    WORD                    Hour;
    WORD                    Minute;
    //WORD wSecond;
    //WORD wMilliseconds;
    //SYSTEMTIME              ScheduleTime;
};

#endif