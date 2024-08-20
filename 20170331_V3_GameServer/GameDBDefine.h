#ifndef __GAME_DB_DEFINE_HEADER__
#define __GAME_DB_DEFINE_HEADER__

/*

# ��� �α״� DBWriteThread �� ���ؼ� �����ؾ� �ϸ�,
# ���� ������ ó�� ���� �Ǵ� ���� �����Ͱ� ���ٸ� �α����常�� ���� DBWriteThread �� �̿��Ѵ�


type	code	AccountNo	ServerName	param1		param2		param3		param4		message

���Ӽ��� �α���	1	11	AccountNo	�����̸�								port		ip

// ��¥ ���Ӽ����� �α��� �� ����. (ĳ���� ���� ������ �ƴ�)
// Auth �ܰ迡�� ����Ű Ȯ�� �� Player ������ ���翩�� Ȯ�� �� ����


���Ӽ��� ����	1	12	AccountNo	�����̸�		TileX		TileY		CharacterType

// ĳ���� ���� �� ���Ӹ�� ���Խ�
// gameDB �� ������ �����ʹ� ������ �α������� ���ؼ� DBWriterThread �� Game �� �������� ���ֵ��� ��


���Ӽ��� ����	1	13	AccountNo	�����̸�		TileX		TileY		killcount	guestkillcount

// �װų� ����ڰ� ���� �� ���Ӽ������� ���� �� ���� (ĳ���� ���� ������ �����ٸ� ���� ����)
// ���⼭ KillCount / GuestKillCount �� �� ������ �ƴ� �̹� �÷����� ī��Ʈ.

// gameDB �� ������ �����ʹ� ������ �α������� ���ؼ� DBWriterThread �� Game �� �������� ���ֵ��� ��
// �Ǵ� Status = 0 ���� ��ȯ�ϴ� �������� �ص� �ǳ�, �÷��̸� �� ���� ���� ��츦 �����Ͽ� �����÷��� �� ��츸 �����ϵ��� �ؾ� ��

ĳ���� ����	2	21	AccountNo	�����̸�		TileX		TileY		AttackerAccountNo

// �״� ���� die ī��Ʈ �ø��� ����


ĳ���� 	ų	2	23	AccountNo	�����̸�		TileX		TileY		TargetAccountNo

// ������ �׾��� �� Kill ī��Ʈ �ø��� ����


# �α��μ��� (�α��� ���� ����)

�α���		100	101	AccountNo	"Login"		status								ip:port



*/

//--------------------------------------------------------
// DB �� ������ ���,�����ϱ� Ÿ��.
//
// Read, Write �������� ��.  
// �̸� ������ �� ��� �Ǽ��� ���� ���� ����ص� Ȯ���ϱ� ������� ���� �� ����.
//--------------------------------------------------------
enum en_DB_ACTION_TYPE
{
    //enDB_ACCOUNT_READ_LOGIN_SESSION = 0,			// accountdb ���� �α��� ����Ű �� ���� Ȯ�� (�α��μ������� ���)

    enDB_ACCOUNT_READ_RESET_STATUS_ALL,		        // ���� ������ ó�� ������ ��� ����ڰ� �α׾ƿ� �Ȱɷ� �����.

    enDB_ACCOUNT_READ_USER,							// accountdb ���� ȸ�� ���� ��� & status login ���·�

    enDB_GAME_READ_PLAYER_CHECK,					// gamedb ���� �÷��̾� ������ �ִ��� Ȯ��, ���ٸ� ����. 
    // ���� ������ ����. 

    enDB_GAME_WRITE_LOG_JOIN,						// ĳ���� ���� �� �������Խ�
    // gameDB �� ������ ������ ������ �α׸� ����� ����

    enDB_GAME_WRITE_LOG_LEAVE,						// ���Ӹ�忡�� ���Ӽ����� ���� �� ����.
    // gameDB �� ������ ������ ������ �α׸� ����� ����

    enDB_GAME_WRITE_PLAYER_DIE,						// �÷��̾� ���ư��� / die ī��Ʈ + 1
    enDB_GAME_WRITE_PLAYER_KILL,					// �÷��̾� ���� ų / kill ī��Ʈ + 1
    // ����� ���̶�� �α׸� ����� Kill ī��Ʈ�� �ø��� ����

    enDB_ACCOUNT_WRITE_STATUS_LOGOUT,				// �÷��̾� �α׾ƿ��� status �� �α׾ƿ� ���·� ����

    //enDB_LOG_WRITE_GAME_LOG                         // ���ӷα�(������ �߰�.)
    //enDB_GAME_WRITE_LOG_LOGIN                       // ���� �α��� �α�(������ �߰�.)

    enDB_GAME_WRITE_LOG_PLAYER_DIE,						// �÷��̾� ���ư��� / �α׸� �����.
    enDB_GAME_WRITE_LOG_PLAYER_KILL,					// �÷��̾� ���� ų / �α׸� �����.
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
    BYTE		Party;					// ��Ƽ 1 / 2
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

    __int64		AttackerAccountNo;		// gameDB �� �������� ������ Log �� ����� ����

    int			DiePosX;				// gameDB ���� �ʿ䰡 ������ Log �� ������ġ ǥ�ø� ����.
    int			DiePosY;
};



// enDB_GAME_WRITE_PLAYER_KILL ----------------------------------------------------------

struct stDB_GAME_WRITE_PLAYER_KILL_in
{
    __int64		AccountNo;

    __int64		TargetAccountNo;		// gameDB �� �������� ������ Log �� ����� ����

    int			KillPosX;				// gameDB ���� �ʿ䰡 ������ Log �� ������ġ ǥ�ø� ����.
    int			KillPosY;

    bool        IsGuestTarget;
};


// enDB_ACCOUNT_WRITE_STATUS_LOGOUT ----------------------------------------------------------

struct stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in
{
    __int64		AccountNo;
};






//--------------------------------------------------------
// DB ���� ������� �޽���
//
//--------------------------------------------------------
#define dfDBWRITER_MSG_MAX	200

#define dfDBWRITER_TYPE_ACCOUNT		1		// AccountDB 
#define dfDBWRITER_TYPE_GAME		2		// GameDB
#define dfDBWRITER_TYPE_HEARTBEAT	3		// DBThread Heartbeat

//--------------------------------------------------------
// DB ���� �޽��� ���պ�.
//--------------------------------------------------------
typedef struct _st_DBWRITER_MSG
{
    int					Type_DB;			// AccountDB, GameDB ����	
    en_DB_ACTION_TYPE	Type_Message;		// Message ����

    BYTE	Message[dfDBWRITER_MSG_MAX];	// �޽��� ������ �� ������ ����
    // �Ʒ��� ����ü���� ��� ���� �� �� �ִ� ������.
    // �������� ����Ѵ�.
} st_DBWRITER_MSG;






// ��� DB ���� ���� ������ DBWrite ���� �����ؾ� �Ѵ�.
//
// DB ���� �޽����� ���̺� ������ �ƴ� ������ ������ ���°��� �´�.
// �ϳ��� �޽��� ó���� ���� ������ Ʈ������ �ɾ ó�� �ؾ� ��.
// �׷��Ƿ� �ϳ��� st_DBWRITER_MSG �޽����� �ϳ��� ������ �ؾ� �ϴ°��� �ƴ�.

#endif