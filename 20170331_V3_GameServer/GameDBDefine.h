#ifndef __GAME_DB_DEFINE_HEADER__
#define __GAME_DB_DEFINE_HEADER__

/*

# 모든 로그는 DBWriteThread 를 통해서 저장해야 하며,
# 관련 컨텐츠 처리 직후 또는 저장 데이터가 없다면 로그저장만을 위해 DBWriteThread 를 이용한다


type	code	AccountNo	ServerName	param1		param2		param3		param4		message

게임서버 로그인	1	11	AccountNo	서버이름								port		ip

// 진짜 게임서버에 로그인 된 순간. (캐릭터 생성 시점이 아님)
// Auth 단계에서 세션키 확인 후 Player 데이터 존재여부 확인 후 저장


게임서버 조인	1	12	AccountNo	서버이름		TileX		TileY		CharacterType

// 캐릭터 선택 후 게임모드 진입시
// gameDB 에 저장할 데이터는 없으나 로그저장을 위해서 DBWriterThread 에 Game 용 저장으로 쏴주도록 함


게임서버 리브	1	13	AccountNo	서버이름		TileX		TileY		killcount	guestkillcount

// 죽거나 사용자가 게임 후 게임서버에서 나갈 때 저장 (캐릭터 선택 이전에 나간다면 저장 안함)
// 여기서 KillCount / GuestKillCount 는 총 누적이 아닌 이번 플레이의 카운트.

// gameDB 에 저장할 데이터는 없으나 로그저장을 위해서 DBWriterThread 에 Game 용 저장으로 쏴주도록 함
// 또는 Status = 0 으로 전환하는 과정에서 해도 되나, 플레이를 한 경우와 안한 경우를 구분하여 게임플레이 한 경우만 저장하도록 해야 함

캐릭터 죽음	2	21	AccountNo	서버이름		TileX		TileY		AttackerAccountNo

// 죽는 순간 die 카운트 올릴시 저장


캐릭터 	킬	2	23	AccountNo	서버이름		TileX		TileY		TargetAccountNo

// 상대방이 죽었을 때 Kill 카운트 올릴시 저장


# 로그인서버 (로그인 서버 전용)

로그인		100	101	AccountNo	"Login"		status								ip:port



*/

//--------------------------------------------------------
// DB 의 정보를 얻기,저장하기 타입.
//
// Read, Write 통합으로 감.  
// 이를 별도로 할 경우 실수로 같은 값을 사용해도 확인하기 어려움이 있을 수 있음.
//--------------------------------------------------------
enum en_DB_ACTION_TYPE
{
    //enDB_ACCOUNT_READ_LOGIN_SESSION = 0,			// accountdb 에서 로그인 세션키 및 정보 확인 (로그인서버에서 사용)

    enDB_ACCOUNT_READ_RESET_STATUS_ALL,		        // 게임 서버가 처음 켜질때 모든 사용자가 로그아웃 된걸로 만든다.

    enDB_ACCOUNT_READ_USER,							// accountdb 에서 회원 정보 얻기 & status login 상태로

    enDB_GAME_READ_PLAYER_CHECK,					// gamedb 에서 플레이어 정보가 있는지 확인, 없다면 생성. 
    // 얻을 내용은 없음. 

    enDB_GAME_WRITE_LOG_JOIN,						// 캐릭터 선택 후 게임진입시
    // gameDB 에 저장할 내용은 없으나 로그를 남기기 위함

    enDB_GAME_WRITE_LOG_LEAVE,						// 게임모드에서 게임서버를 나갈 때 저장.
    // gameDB 에 저장할 내용은 없으나 로그를 남기기 위함

    enDB_GAME_WRITE_PLAYER_DIE,						// 플레이어 돌아가심 / die 카운트 + 1
    enDB_GAME_WRITE_PLAYER_KILL,					// 플레이어 적군 킬 / kill 카운트 + 1
    // 대상이 더미라면 로그만 남기고 Kill 카운트는 올리지 않음

    enDB_ACCOUNT_WRITE_STATUS_LOGOUT,				// 플레이어 로그아웃시 status 를 로그아웃 상태로 변경

    //enDB_LOG_WRITE_GAME_LOG                         // 게임로그(본인이 추가.)
    //enDB_GAME_WRITE_LOG_LOGIN                       // 게임 로그인 로그(본인이 추가.)

    enDB_GAME_WRITE_LOG_PLAYER_DIE,						// 플레이어 돌아가심 / 로그만 남긴다.
    enDB_GAME_WRITE_LOG_PLAYER_KILL,					// 플레이어 적군 킬 / 로그만 남긴다.
};





//// enDB_ACCOUNT_READ_LOGIN_SESSION ----------------------------------------------------------
//
//struct stDB_ACCOUNT_READ_LOGIN_SESSION_in
//{
//    __int64		AccountNo;
//    char		SessionKey[64];
//};
//
//
//struct stDB_ACCOUNT_READ_LOGIN_SESSION_out
//{
//    WCHAR		szID[ID_MAX_LEN];
//    WCHAR		szNick[NICK_MAX_LEN];
//    int			Status;
//};



// enDB_ACCOUNT_READ_USER ----------------------------------------------------------

struct stDB_ACCOUNT_READ_USER_in
{
    __int64				    AccountNo;

    st_CLIENT_CONNECT_INFO	ConnectInfo;
};


struct stDB_ACCOUNT_READ_USER_out
{
    WCHAR		szID[ID_MAX_LEN];
    WCHAR		szNick[NICK_MAX_LEN];
    BYTE		Status;
    BYTE		Party;					// 파티 1 / 2
};






// enDB_GAME_READ_PLAYER_CHECK ----------------------------------------------------------

struct stDB_GAME_READ_PLAYER_CHECK_in
{
    __int64				    AccountNo;
    st_CLIENT_CONNECT_INFO	ConnectInfo;
};

struct stDB_GAME_READ_PLAYER_CHECK_out
{
    int                     GuestKillCount;
};



// enDB_GAME_WRITE_LOG_JOIN --------------------------------------------------------------

struct stDB_GAME_WRITE_LOG_JOIN_in
{
    __int64		AccountNo;

    int			TileX;
    int			TileY;

    BYTE		CharacterType;
};

// enDB_GAME_WRITE_LOG_LEAVE --------------------------------------------------------------

struct stDB_GAME_WRITE_LOG_LEAVE_in
{
    __int64		AccountNo;

    int			TileX;
    int			TileY;

    int			KillCount;
    int			GuestKillCount;
};


// enDB_GAME_WRITE_PLAYER_DIE ----------------------------------------------------------

struct stDB_GAME_WRITE_PLAYER_DIE_in
{
    __int64		AccountNo;

    __int64		AttackerAccountNo;		// gameDB 에 저장하진 않으나 Log 에 남기기 위함

    int			DiePosX;				// gameDB 에는 필요가 없으나 Log 에 죽은위치 표시를 위함.
    int			DiePosY;
};



// enDB_GAME_WRITE_PLAYER_KILL ----------------------------------------------------------

struct stDB_GAME_WRITE_PLAYER_KILL_in
{
    __int64		AccountNo;

    __int64		TargetAccountNo;		// gameDB 에 저장하진 않으나 Log 에 남기기 위함

    int			KillPosX;				// gameDB 에는 필요가 없으나 Log 에 죽은위치 표시를 위함.
    int			KillPosY;

    bool        IsGuestTarget;
};


// enDB_ACCOUNT_WRITE_STATUS_LOGOUT ----------------------------------------------------------

struct stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in
{
    __int64		AccountNo;
};






//--------------------------------------------------------
// DB 저장 스레드용 메시지
//
//--------------------------------------------------------
#define dfDBWRITER_MSG_MAX	200

#define dfDBWRITER_TYPE_ACCOUNT		1		// AccountDB 
#define dfDBWRITER_TYPE_GAME		2		// GameDB
#define dfDBWRITER_TYPE_HEARTBEAT	3		// DBThread Heartbeat

//--------------------------------------------------------
// DB 저장 메시지 통합본.
//--------------------------------------------------------
typedef struct _st_DBWRITER_MSG
{
    int					Type_DB;			// AccountDB, GameDB 구분	
    en_DB_ACTION_TYPE	Type_Message;		// Message 구분

    BYTE	Message[dfDBWRITER_MSG_MAX];	// 메시지 역할을 할 데이터 영역
    // 아래의 구조체들을 모두 포함 할 수 있는 사이즈.
    // 공용으로 사용한다.
} st_DBWRITER_MSG;






// 모든 DB 저장 관련 쿼리는 DBWrite 에서 전담해야 한다.
//
// DB 저장 메시지는 테이블 단위가 아닌 컨텐츠 단위로 가는것이 맞다.
// 하나의 메시지 처리를 위한 내용을 트랜젝션 걸어서 처리 해야 함.
// 그러므로 하나의 st_DBWRITER_MSG 메시지에 하나의 쿼리만 해야 하는것이 아님.

#endif