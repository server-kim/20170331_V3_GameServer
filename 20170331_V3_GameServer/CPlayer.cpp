#include "stdafx.h"
#include "TotalHeader.h"

ULONGLONG Player::UnderAttackLastTime = 0;

Player::Player(void)
{
    OnGame_ClientRelease();
}
Player::~Player(void)
{

}

//------------------------------------------
// auth packet proc
//------------------------------------------

bool Player::PacketProc_Login(Packet *p_Packet)
{
    INT64 AccountNo;
    char SessionKey[SESSION_KEY_BYTE_LEN];      // enum으로 뺄 것.
    int Version;

    //BYTE Status;
    GAME_USER_TYPE_DEFINE LoginUserType;
    stDB_ACCOUNT_READ_USER_in ReadUserIn;
    stDB_ACCOUNT_READ_USER_out ReadUserOut;
    stDB_GAME_READ_PLAYER_CHECK_in PlayerCheckIn;
    stDB_GAME_READ_PLAYER_CHECK_out PlayerCheckOut;
    Packet *p_MakePacket;
    //int PlayerIndex;

    *p_Packet >> AccountNo;
    p_Packet->GetData(SessionKey, SESSION_KEY_BYTE_LEN);
    *p_Packet >> Version;

    if (_AccountNo != ACCOUNT_NUM_DEFAULT)
    {
        //CrashDump::Crash();

        // Login Packet이 두번 오는 경우가 발생하였다. 상황을 지켜보자.
        //ReadUserOut.Status = dfGAME_LOGIN_FAIL;
        //p_MakePacket = PacketMake_Login(ReadUserOut.Status, AccountNo);
        //if (false == SendPacket(p_MakePacket))
        //{
        //    CrashDump::Crash();
        //    return false;
        //}
        //p_MakePacket->Free();
        //
        //_DisconnectFlag = true;
        SYSLOG(L"PacketProc_Login", LOG_ERROR, L"Login Packet Error # [AccountNo:%lld][PacketAccountNo:%lld]", _AccountNo, AccountNo);
        return false;
    }

    _AccountNo = AccountNo;     // 새로 추가되었음.

    //Status = dfGAME_LOGIN_FAIL;
    ReadUserOut.Status = dfGAME_LOGIN_FAIL;
    do
    {
        if (AccountNo <= DUMMY_ACCOUNTNO_LIMIT)
        {
            // 더미 - 세션키 체크 안함.
            LoginUserType = USER_TYPE_DUMMY;
        }
        else if (AccountNo >= GUEST_ACCOUNTNO_OVER)
        {
            LoginUserType = USER_TYPE_GUEST;
        }
        else
        {
            LoginUserType = USER_TYPE_GAMECODI;
        }

        // session key 및 version 체크.
        if (LoginUserType != USER_TYPE_DUMMY)
        {
            if (false == _p_GameServer->CheckLoginSessionKey(AccountNo, (BYTE *)SessionKey))
            {
                //Status = dfGAME_LOGIN_FAIL;
                SYSLOG(L"PacketProc_Login", LOG_ERROR, L"SessionKey Error # [AccountNo:%lld]", AccountNo);
                break;
            }
        }

        if (Version != GAME_VERSION)
        {
            ReadUserOut.Status = dfGAME_LOGIN_VERSION_MISS;
            SYSLOG(L"PacketProc_Login", LOG_ERROR, L"Version Miss # [AccountNo:%lld]", AccountNo);
            break;
        }

        ReadUserIn.AccountNo = AccountNo;
        ReadUserIn.ConnectInfo = _ClientInfo;
        if (false == _p_GameServer->_p_DBAccount_Auth->ReadDB(enDB_ACCOUNT_READ_USER, &ReadUserIn, &ReadUserOut))
        {
            //Status = dfGAME_LOGIN_FAIL;
            // 만약 현재 게임중이라면 접속을 끊고 현재 로그인중인 유저에게는 다시 접속하도록 요청을 보낸다.
            if (dfGAME_LOGIN_DUPLICATE == ReadUserOut.Status)
                SYSLOG(L"PacketProc_Login", LOG_ERROR, L"ReadDB Duplicate # [AccountNo:%lld]", AccountNo);
            else
                SYSLOG(L"PacketProc_Login", LOG_ERROR, L"ReadDB Failed # [AccountNo:%lld]", AccountNo);
            break;
        }

        //// 플레이어 배열을 돌면서 AccountNo 중복 체크.
        //for (PlayerIndex = 0; PlayerIndex < _MaxSession; ++PlayerIndex)
        //{
        //    if (ACCOUNT_NUM_DEFAULT == _p_PlayerArray[PlayerIndex]._AccountNo)
        //        continue;
        //    if (AccountNo == _p_PlayerArray[PlayerIndex]._AccountNo && &_p_PlayerArray[PlayerIndex] != this)
        //    {
        //        _p_PlayerArray[PlayerIndex]._LogoutFlag = true;
        //        SYSLOG(L"PacketProc_Login", LOG_ERROR, L"AccountNo Duplicate # [AccountNo:%lld]", AccountNo);
        //    }
        //}

        // 로그인 체크가 다 끝났기 때문에 GameDB의 플레이어 정보를 체크한다.
        PlayerCheckIn.AccountNo = AccountNo;
        PlayerCheckIn.ConnectInfo = _ClientInfo;
        if (false == _p_GameServer->_p_DBGame_Auth->ReadDB(enDB_GAME_READ_PLAYER_CHECK, &PlayerCheckIn, &PlayerCheckOut))
        {
            ReadUserOut.Status = dfGAME_LOGIN_FAIL;
            SYSLOG(L"PacketProc_Login", LOG_ERROR, L"ReadPlayerCheck Failed # [AccountNo:%lld]", AccountNo);
            break;
        }

    } while (0);

    if (ReadUserOut.Status == dfGAME_LOGIN_PARTY1 || ReadUserOut.Status == dfGAME_LOGIN_PARTY2)
    {
        _LoginFlag = true;
        //_Party = Status;
        _Party = ReadUserOut.Party;

        _UserType = LoginUserType;
        //_AccountNo = AccountNo;
        wcscpy_s(_ID, 256, ReadUserOut.szID);
        wcscpy_s(_Nickname, 256, ReadUserOut.szNick);

        _GuestKillCount = PlayerCheckOut.GuestKillCount;

        // Login이 성공하였으므로  Game 모드로 전환.(이건 캐릭터 선택 다음에 한다.)
        //SetMode_Game();
    }
    else
    {
        _DisconnectFlag = true;
        SYSLOG(L"PacketProc_Login", LOG_ERROR, L"Login Failed # [Status:%u]", ReadUserOut.Status);
    }

    p_MakePacket = PacketMake_Login(ReadUserOut.Status, AccountNo);
    if (false == SendPacket(p_MakePacket))
    {
        CrashDump::Crash();
        return false;
    }
    p_MakePacket->Free();

    return true;
}
bool Player::PacketProc_CharacterSelect(Packet *p_Packet)
{
    if (ACCOUNT_NUM_DEFAULT == _AccountNo || false == _LoginFlag || PARTY_DEFAULT == _Party)
    {
        // login 하지 않고 패킷을 날리는 경우.
        //CrashDump::Crash();
        SYSLOG(L"PacketProc_CharacterSelect", LOG_ERROR, L"Is Not Login Error # [AccountNo:%lld]", _AccountNo);
        return false;
    }

    BYTE Status;
    Packet *p_MakePacket;
    BYTE CharacterType;

    *p_Packet >> CharacterType;

    Status = 0;             // 일단 실패로 초기값.
    switch (CharacterType)
    {
    case dfGAME_CHARACTER_GOLEM:
    case dfGAME_CHARACTER_KNIGHT:
        if (dfGAME_LOGIN_PARTY1 == _Party)
            Status = 1;
        break;
    case dfGAME_CHARACTER_ELF:
        if (dfGAME_LOGIN_PARTY1 == _Party || dfGAME_LOGIN_PARTY2 == _Party)
        {
            if (_UserType != USER_TYPE_GUEST)
                Status = 1;
        }
        break;
    case dfGAME_CHARACTER_ORC:
    case dfGAME_CHARACTER_ARCHER:
        if (dfGAME_LOGIN_PARTY2 == _Party)
            Status = 1;
        break;
    default:
        SYSLOG(L"PacketProc", LOG_ERROR, L"PacketProc_CharacterSelect # [CharacterType:%u]", CharacterType);
        //CrashDump::Crash();
        //Status = 0;
        return false;
        //break;
    }

    if (1 == Status)
    {
        _CharacterType = CharacterType;

        // 초기값은 일단 여기서 세팅한다.
        do
        {
            if (USER_TYPE_DUMMY == _UserType)
            {
                _TileX = dfCREATE_PLAYER_X_DUMMY(_AccountNo);
                _TileY = dfCREATE_PLAYER_Y_DUMMY(_AccountNo);
            }
            else
            {
                switch (_Party)
                {
                case dfGAME_LOGIN_PARTY1:
                    _TileX = dfCREATE_PLAYER_X_PARTY1;
                    _TileY = dfCREATE_PLAYER_Y_PARTY1;
                    break;
                case dfGAME_LOGIN_PARTY2:
                    _TileX = dfCREATE_PLAYER_X_PARTY2;
                    _TileY = dfCREATE_PLAYER_Y_PARTY2;
                    break;
                default:
                    CrashDump::Crash();
                    break;
                }
            }
        } while (false == _p_PathFinder->CheckMapAttribute(_TileX, _TileY));

        // Tile에 따른 Pos 및 Sector 결정.
        _PosX = TILE_to_POS_X(_TileX);
        _PosY = TILE_to_POS_Y(_TileY);
        _SectorX = TILE_to_SECTOR_X(_TileX);
        _SectorY = TILE_to_SECTOR_Y(_TileY);
        _OldSectorX = _SectorX;
        _OldSectorY = _SectorY;

        // HP 세팅.
        _HP = dfHP_MAX;

        // 캐릭터 선택이 성공하였으므로 Game 모드로 전환.
        SetMode_Game();
    }

    p_MakePacket = PacketMake_CharacterSelect(Status);
    if (false == SendPacket(p_MakePacket))
    {
        CrashDump::Crash();
        return false;
    }
    p_MakePacket->Free();

    return true;
}

Packet *Player::PacketMake_Login(BYTE Status, INT64 AccountNo)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_LOGIN;
    *p_Packet << Status;
    *p_Packet << AccountNo;

    return p_Packet;
}
Packet *Player::PacketMake_CharacterSelect(BYTE Status)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_CHARACTER_SELECT;
    *p_Packet << Status;

    return p_Packet;
}

//------------------------------------------
// game packet proc
//------------------------------------------

bool Player::PacketProc_MoveCharacter(Packet *p_Packet)
{
    OBJECT_ID ObjectID;
    float StartX;
    float StartY;
    float DestX;
    float DestY;

    bool IsSync;
    float CheckPosX;
    float CheckPosY;
    Packet *p_MakePacket;

    *p_Packet >> ObjectID;
    *p_Packet >> StartX;
    *p_Packet >> StartY;
    *p_Packet >> DestX;
    *p_Packet >> DestY;

    if (_ObjectID != ObjectID)
    {
        //CrashDump::Crash();         // 일단 뻑내보자.
        SYSLOG(L"PacketProc", LOG_ERROR, L"PacketProc_MoveCharacter # Wrong ObjectID [AccountNo:%lld][ObjectID:%lld][PacketObjectID:%lld]",
            _AccountNo, _ObjectID, ObjectID);
        return false;
    }

    if (true == _IsFirstJoin)
        _IsFirstJoin = false;

    IsSync = false;
    if (std::isnan(StartX) || std::isnan(StartY))
    {
        IsSync = true;
        SYSLOG(L"PacketProc", LOG_ERROR, L"MoveCharacter # IsNAN ERROR [AccountNo:%lld][PosX:%f][PosY:%f][StartX:%s][StartY:%s]",
            _AccountNo, _PosX, _PosY, std::isnan(StartX) ? "true" : "false", std::isnan(StartY) ? "true" : "false");
    }
    else if (std::isnan(DestX) || std::isnan(DestY))
    {
        IsSync = true;
        SYSLOG(L"PacketProc", LOG_ERROR, L"MoveCharacter # IsNAN ERROR [AccountNo:%lld][PosX:%f][PosY:%f][DestX:%s][DestY:%s]",
            _AccountNo, _PosX, _PosY, std::isnan(DestX) ? "true" : "false", std::isnan(DestY) ? "true" : "false");
    }
    else
    {
        // 길찾기 전에 범위 체크.
        CheckPosX = abs(_PosX - StartX);
        CheckPosY = abs(_PosY - StartY);

        if (CheckPosX > dfPOSITON_ERROR_RANGE_X || CheckPosY > dfPOSITON_ERROR_RANGE_Y)
        {
            IsSync = true;
            SYSLOG(L"PacketProc", LOG_ERROR, L"MoveCharacter # ERROR_RANGE[AccountNo:%lld][PosX:%f][PosY:%f][StartX:%f][StartY:%f][DestX:%f][DestY:%f]", _AccountNo, _PosX, _PosY, StartX, StartY, DestX, DestY);
        }
        else
        {
            // 이 때는 클라가 보내준 값을 믿는다.
            if (false == SetPosition(StartX, StartY))
            {
                IsSync = true;
                SYSLOG(L"PacketProc", LOG_ERROR, L"MoveCharacter # SetPosition FAIL[AccountNo:%lld][PosX:%f][PosY:%f][StartX:%f][StartY:%f]", _AccountNo, _PosX, _PosY, StartX, StartY);
            }
        }
    }

    if (true == IsSync)
    {
        p_MakePacket = PacketMake_CharacterSync(_ObjectID, _PosX, _PosY);
        SendPacket(p_MakePacket);
        p_MakePacket->Free();
    }

    // 액션 초기화
    MoveReset();
    AttackReset();

    // 목적지 세팅
    _DestTileX = POS_to_TILE_X(DestX);
    _DestTileY = POS_to_TILE_Y(DestY);

    _IsActionReset = false;

    //SYSLOG(L"PacketProc", LOG_DEBUG, L"MoveCharacter # [AccountNo:%lld][TileX:%d][TileY:%d][DestX:%d][DestY:%d]",
    //    _AccountNo, _TileX, _TileY, _DestTileX, _DestTileY);

    return true;
}
bool Player::PacketProc_StopCharacter(Packet *p_Packet)
{
    OBJECT_ID ObjectID;
    float StopX;
    float StopY;
    USHORT Rotation;

    bool IsSync;
    float CheckPosX;
    float CheckPosY;
    Packet *p_MakePacket;

    *p_Packet >> ObjectID;
    *p_Packet >> StopX;
    *p_Packet >> StopY;
    *p_Packet >> Rotation;

    if (_ObjectID != ObjectID)
    {
        //CrashDump::Crash();         // 일단 뻑내보자.
        SYSLOG(L"PacketProc", LOG_ERROR, L"PacketProc_StopCharacter # Wrong ObjectID [AccountNo:%lld][ObjectID:%lld][PacketObjectID:%lld]",
            _AccountNo, _ObjectID, ObjectID);
        return false;
    }

    IsSync = false;
    if (std::isnan(StopX) || std::isnan(StopY))
    {
        IsSync = true;
        SYSLOG(L"PacketProc", LOG_ERROR, L"StopCharacter # IsNAN ERROR [AccountNo:%lld][PosX:%f][PosY:%f][StopX:%s][StopY:%s]",
            _AccountNo, _PosX, _PosY, std::isnan(StopX) ? "true" : "false", std::isnan(StopY) ? "true" : "false");
    }
    else
    {
        // 길찾기 전에 범위 체크.
        CheckPosX = abs(_PosX - StopX);
        CheckPosY = abs(_PosY - StopY);

        if (CheckPosX > dfPOSITON_ERROR_RANGE_X || CheckPosY > dfPOSITON_ERROR_RANGE_Y)
        {
            IsSync = true;
            SYSLOG(L"PacketProc", LOG_ERROR, L"StopCharacter # ERROR_RANGE[AccountNo:%lld][PosX:%f][PosY:%f][StopX:%f][StopY:%f]", _AccountNo, _PosX, _PosY, StopX, StopY);
        }
        else
        {
            // 이 때는 클라가 보내준 값을 믿는다.
            if (false == SetPosition(StopX, StopY))
            {
                IsSync = true;
                SYSLOG(L"PacketProc", LOG_ERROR, L"StopCharacter # SetPosition FAIL[AccountNo:%lld][PosX:%f][PosY:%f][StopX:%f][StopY:%f]", _AccountNo, _PosX, _PosY, StopX, StopY);
            }
        }
    }

    if (true == IsSync)
    {
        // stop은 싱크 안보내도 된다.
        // 일단 보내보자.
        p_MakePacket = PacketMake_CharacterSync(_ObjectID, _PosX, _PosY);
        SendPacket(p_MakePacket);
        p_MakePacket->Free();
    }

    // 액션 초기화
    MoveReset();
    //AttackReset();        // 클라이언트에서 정지시 보내는 거 같다. 공격시라면 빼자.
    _DestTileX = TILE_DEFAULT_X;
    _DestTileY = TILE_DEFAULT_Y;

    // Rotation 저장.
    _Rotation = Rotation;

    _IsActionReset = false;

    //SYSLOG(L"PacketProc", LOG_DEBUG, L"StopCharacter # [AccountNo:%lld][TileX:%d][TileY:%d][DestX:%d][DestY:%d]",
    //    _AccountNo, _TileX, _TileY, _DestTileX, _DestTileY);

    return true;
}
bool Player::PacketProc_Attack1Target(Packet *p_Packet)
{
    OBJECT_ID AttackID;
    OBJECT_ID TargetID;

    //Player *p_Target;

    *p_Packet >> AttackID;
    *p_Packet >> TargetID;

    if (_ObjectID != AttackID)
    {
        //CrashDump::Crash();
        SYSLOG(L"PacketProc", LOG_ERROR, L"PacketProc_Attack1Target # _ObjectID != AttackID [AccountNo:%lld][ObjectID:%lld][AttackID:%lld][TargetID:%lld]",
            _AccountNo, _ObjectID, AttackID, TargetID);
        return false;
    }
    if (TargetID == _ObjectID)
    {
        //CrashDump::Crash();
        SYSLOG(L"PacketProc", LOG_ERROR, L"PacketProc_Attack1Target # TargetID == _ObjectID [AccountNo:%lld][ObjectID:%lld][AttackID:%lld][TargetID:%lld]",
            _AccountNo, _ObjectID, AttackID, TargetID);
        return false;
    }

    //if (true == _IsFirstJoin)
    //    _IsFirstJoin = false;
    //
    //if (0 == TargetID)
    //{
    //    _IsActionReset = true;
    //    return true;
    //}
    //
    //p_Target = FindPlayer(TargetID);
    //if (nullptr == p_Target)
    //{
    //    _IsActionReset = true;
    //    return true;
    //}
    //
    //if (true == p_Target->_IsFirstJoin)
    //{
    //    return true;
    //}
    //
    //if (p_Target->_HP < 1)
    //{
    //    _IsActionReset = true;
    //    return true;
    //}
    //
    //if (_TargetID == p_Target->_ObjectID && ATTACK_TYPE_1 == _AttackType)
    //{
    //    // 기존과 타겟이 동일하다면 아무일도 하지 않는다.
    //    _IsActionReset = false;
    //    return true;
    //}
    //
    //// 1. 서버는 대상 캐릭터가 공격가능한 대상인지 ( Party 별 캐릭터 타입 확인 / 엘프는 중립으로 모두 공격가능 )
    //if (_CharacterType != dfGAME_CHARACTER_ELF)         // ELF는 모두 공격 가능.
    //{
    //    if (p_Target->_CharacterType != dfGAME_CHARACTER_ELF)
    //    {
    //        if (_Party == p_Target->_Party)
    //        {
    //            _IsActionReset = true;
    //            return true;
    //        }
    //    }
    //}
    //
    //// 새로운 타겟이므로 공격을 중단한다.
    //AttackReset();
    //MoveReset();
    //
    //_TargetID = p_Target->_ObjectID;
    //_p_Target = p_Target;
    //_DestTileX = p_Target->_TileX;
    //_DestTileY = p_Target->_TileY;
    //
    //_AttackType = ATTACK_TYPE_1;
    //_IsActionReset = false;

    if (true == CheckAttackTarget(TargetID))
        _AttackType = ATTACK_TYPE_1;

    //SYSLOG(L"PacketProc", LOG_DEBUG, L"Attack1Target # [AccountNo:%lld][TileX:%d][TileY:%d][DestX:%d][DestY:%d]",
    //    _AccountNo, _TileX, _TileY, _DestTileX, _DestTileY);

    return true;
}
bool Player::PacketProc_Attack2Target(Packet *p_Packet)
{
    OBJECT_ID AttackID;
    OBJECT_ID TargetID;

    //Player *p_Target;

    *p_Packet >> AttackID;
    *p_Packet >> TargetID;

    if (_ObjectID != AttackID)
    {
        //CrashDump::Crash();
        SYSLOG(L"PacketProc", LOG_ERROR, L"PacketProc_Attack2Target # _ObjectID != AttackID [AccountNo:%lld][ObjectID:%lld][AttackID:%lld][TargetID:%lld]",
            _AccountNo, _ObjectID, AttackID, TargetID);
        return false;
    }
    if (TargetID == _ObjectID)
    {
        //CrashDump::Crash();
        SYSLOG(L"PacketProc", LOG_ERROR, L"PacketProc_Attack2Target # TargetID == _ObjectID [AccountNo:%lld][ObjectID:%lld][AttackID:%lld][TargetID:%lld]",
            _AccountNo, _ObjectID, AttackID, TargetID);
        return false;
    }

    //if (true == _IsFirstJoin)
    //    _IsFirstJoin = false;
    //
    //if (0 == TargetID)
    //{
    //    _IsActionReset = true;
    //    return true;
    //}
    //
    //p_Target = FindPlayer(TargetID);
    //if (nullptr == p_Target)
    //{
    //    _IsActionReset = true;
    //    return true;
    //}
    //
    //if (true == p_Target->_IsFirstJoin)
    //{
    //    return true;
    //}
    //
    //if (p_Target->_HP < 1)
    //{
    //    _IsActionReset = true;
    //    return true;
    //}
    //
    //if (_TargetID == p_Target->_ObjectID && ATTACK_TYPE_2 == _AttackType)
    //{
    //    // 기존과 타겟이 동일하다면 아무일도 하지 않는다.
    //    _IsActionReset = false;
    //    return true;
    //}
    //
    //// 1. 서버는 대상 캐릭터가 공격가능한 대상인지 ( Party 별 캐릭터 타입 확인 / 엘프는 중립으로 모두 공격가능 )
    //if (_CharacterType != dfGAME_CHARACTER_ELF)         // ELF는 모두 공격 가능.
    //{
    //    if (p_Target->_CharacterType != dfGAME_CHARACTER_ELF)
    //    {
    //        if (_Party == p_Target->_Party)
    //        {
    //            _IsActionReset = true;
    //            return true;
    //        }
    //    }
    //    //if (_Party == p_Target->_Party)
    //    //{
    //    //    _IsActionReset = true;
    //    //    return true;
    //    //}
    //}
    //
    //// 새로운 타겟이므로 공격을 중단한다.
    //AttackReset();
    //MoveReset();
    //
    //_TargetID = p_Target->_ObjectID;
    //_p_Target = p_Target;
    //_DestTileX = p_Target->_TileX;
    //_DestTileY = p_Target->_TileY;
    //
    //_AttackType = ATTACK_TYPE_2;
    //_IsActionReset = false;

    if (true == CheckAttackTarget(TargetID))
        _AttackType = ATTACK_TYPE_2;

    //SYSLOG(L"PacketProc", LOG_DEBUG, L"Attack2Target # [AccountNo:%lld][TileX:%d][TileY:%d][DestX:%d][DestY:%d]",
    //    _AccountNo, _TileX, _TileY, _DestTileX, _DestTileY);

    return true;
}
bool Player::CheckAttackTarget(OBJECT_ID TargetID)
{
    Player *p_Target;

    if (true == _IsFirstJoin)
        _IsFirstJoin = false;

    if (0 == TargetID)
    {
        _IsActionReset = true;
        return false;
    }

    p_Target = FindPlayer(TargetID);
    if (nullptr == p_Target)
    {
        _IsActionReset = true;
        return false;
    }

    if (true == p_Target->_IsFirstJoin)
    {
        _IsActionReset = true;
        return false;
    }

    if (p_Target->_HP < 1)
    {
        _IsActionReset = true;
        return false;
    }

    if (_TargetID == p_Target->_ObjectID && ATTACK_TYPE_1 == _AttackType)
    {
        // 기존과 타겟이 동일하다면 아무일도 하지 않는다.
        _IsActionReset = false;
        return false;
    }

    // 1. 서버는 대상 캐릭터가 공격가능한 대상인지 ( Party 별 캐릭터 타입 확인 / 엘프는 중립으로 모두 공격가능 )
    if (_CharacterType != dfGAME_CHARACTER_ELF)         // ELF는 모두 공격 가능.
    {
        if (p_Target->_CharacterType != dfGAME_CHARACTER_ELF)
        {
            if (_Party == p_Target->_Party)
            {
                _IsActionReset = true;
                return false;
            }
        }
    }

    // 새로운 타겟이므로 액션을 중단한다.
    AttackReset();
    MoveReset();

    _TargetID = p_Target->_ObjectID;
    _p_Target = p_Target;
    _TargetTileX = p_Target->_TileX;
    _TargetTileY = p_Target->_TileY;

    //_AttackType = ATTACK_TYPE_1;
    _IsActionReset = false;

    return true;
}

Packet *Player::PacketMake_CreateMyCharacter(void)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_CREATE_MY_CHARACTER;
    *p_Packet << _ObjectID;
    *p_Packet << _CharacterType;
    p_Packet->PutData((char *)_Nickname, NICK_MAX_LEN * sizeof(WCHAR));
    *p_Packet << _PosX;
    *p_Packet << _PosY;
    *p_Packet << _Rotation;
    *p_Packet << _Cristal;
    *p_Packet << _HP;
    *p_Packet << _Exp;
    *p_Packet << _Level;
    *p_Packet << _Party;

    return p_Packet;
}
Packet *Player::PacketMake_CreateOtherCharacter(Player *p_Other, BYTE Respawn)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_CREATE_OTHER_CHARACTER;
    *p_Packet << p_Other->_ObjectID;
    *p_Packet << p_Other->_CharacterType;
    p_Packet->PutData((char *)p_Other->_Nickname, NICK_MAX_LEN * sizeof(WCHAR));
    *p_Packet << p_Other->_PosX;
    *p_Packet << p_Other->_PosY;
    *p_Packet << p_Other->_Rotation;
    *p_Packet << p_Other->_Level;
    *p_Packet << Respawn;
    *p_Packet << (BYTE)0;                   // Sit
    if (p_Other->_HP > 0)                   // Die
        *p_Packet << (BYTE)0;
    else
        *p_Packet << (BYTE)1;
    *p_Packet << p_Other->_HP;
    *p_Packet << p_Other->_Party;
    if (true == p_Other->_IsFirstJoin)               // 접속 후 최초 액션을 했는지 여부
        *p_Packet << (BYTE)0;
    else
        *p_Packet << (BYTE)1;

    return p_Packet;
}
Packet *Player::PacketMake_RemoveObject(OBJECT_ID ObjectID)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_REMOVE_OBJECT;
    *p_Packet << ObjectID;

    return p_Packet;
}

Packet *Player::PacketMake_MoveCharacter(Player *p_Player)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    int Cnt;
    char Count;

    Count = p_Player->_PathSize - p_Player->_PathIndex;

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_MOVE_CHARACTER;
    *p_Packet << p_Player->_ObjectID;
    *p_Packet << Count;
    for (Cnt = p_Player->_PathIndex; Cnt < p_Player->_PathSize; ++Cnt)
    {
        *p_Packet << (float)(TILE_to_POS_X(p_Player->_TilePath[Cnt].X));
        *p_Packet << (float)(TILE_to_POS_Y(p_Player->_TilePath[Cnt].Y));

        //wprintf_s(L"[%d][X:%d][Y:%d]\n", Cnt, p_Player->_TilePath[Cnt].X, p_Player->_TilePath[Cnt].Y);
    }
    *p_Packet << p_Player->_PosX;
    *p_Packet << p_Player->_PosY;

    //int TileX;
    //int TileY;
    //MoveTile(MoveDirection(_TileX, _TileY, p_Player->_TilePath[0].X, p_Player->_TilePath[0].Y),
    //    _TileX, _TileY, &TileX, &TileY);
    //*p_Packet << TILE_to_POS_X(TileX);
    //*p_Packet << TILE_to_POS_Y(TileY);

    return p_Packet;
}
Packet *Player::PacketMake_StopCharacter(OBJECT_ID ObjectID, float X, float Y, USHORT Rotation)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_STOP_CHARACTER;
    *p_Packet << ObjectID;
    *p_Packet << X;
    *p_Packet << Y;
    *p_Packet << Rotation;

    return p_Packet;
}
Packet *Player::PacketMake_CharacterSync(OBJECT_ID ObjectID, float X, float Y)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_CHARACTER_SYNC;
    *p_Packet << ObjectID;
    *p_Packet << X;
    *p_Packet << Y;

    return p_Packet;
}

Packet *Player::PacketMake_Attack(BYTE AttackType, OBJECT_ID AttackID, OBJECT_ID TargetID, int CoolTime, float AttackPosX, float AttackPosY)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_ATTACK;
    *p_Packet << AttackType;
    *p_Packet << AttackID;
    *p_Packet << TargetID;
    *p_Packet << CoolTime;
    *p_Packet << AttackPosX;
    *p_Packet << AttackPosY;

    return p_Packet;
}
Packet *Player::PacketMake_Damage(OBJECT_ID TargetID, int DamageValue, float PushPosX, float PushPosY)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_DAMAGE;
    *p_Packet << TargetID;
    *p_Packet << DamageValue;
    *p_Packet << PushPosX;
    *p_Packet << PushPosY;

    return p_Packet;
}
Packet *Player::PacketMake_Die(OBJECT_ID DeadID)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_PLAYER_DIE;
    *p_Packet << DeadID;

    return p_Packet;
}

Packet *Player::PacketMake_DamageGroup(st_DAMAGE *p_Damage, BYTE Count)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    if (Count > AREA_ATTACK_OBJECT_MAX_COUNT || Count < 1)
    {
        CrashDump::Crash();
        return nullptr;
    }

    //int Cnt;

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_DAMAGE_GROUP;
    *p_Packet << Count;
    p_Packet->PutData((char *)p_Damage, Count * sizeof(st_DAMAGE));         // 이건 바이트정렬 때문에 위험성이 있다.
    //for (Cnt = 0; Cnt < Count; ++Count)
    //{
    //    *p_Packet << p_Damage[Cnt].X;
    //    *p_Packet << p_Damage[Cnt].Y;
    //}

    return p_Packet;
}
Packet *Player::PacketMake_PlayerHP(int HP)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_PLAYER_HP;
    *p_Packet << HP;

    return p_Packet;
}
Packet *Player::PacketMake_UnderattackPos(float PosX, float PosY)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_UNDERATTACK_POS;
    *p_Packet << PosX;
    *p_Packet << PosY;

    return p_Packet;
}
Packet *Player::PacketMake_BotKill(void)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_BOT_KILL;
    *p_Packet << _GuestKillCount;

    return p_Packet;
}
//Packet *Player::PacketMake_KillRank(BYTE RankDataSize, st_RANK_DATA *p_RankData)
//{
//    Packet *p_Packet = Packet::Alloc();
//    if (nullptr == p_Packet)
//    {
//        CrashDump::Crash();
//        return nullptr;
//    }
//
//    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_KILL_RANK;
//    *p_Packet << RankDataSize;
//    p_Packet->PutData((char *)p_RankData, sizeof(st_RANK_DATA) * RankDataSize);
//
//    return p_Packet;
//}
//Packet *Player::PacketMake_PlayerPosAlert(st_SECTOR_POS *p_Sector, BYTE Count)
//{
//    Packet *p_Packet = Packet::Alloc();
//    if (nullptr == p_Packet)
//    {
//        CrashDump::Crash();
//        return nullptr;
//    }
//
//    if (Count > PLAYER_POS_ALERT_SECTOR_MAX || Count < 1)
//    {
//        CrashDump::Crash();
//        return nullptr;
//    }
//
//    int Cnt;
//
//    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_PLAYER_POS_ALERT;
//    *p_Packet << Count;
//    for (Cnt = 0; Cnt < Count; ++Count)
//    {
//        *p_Packet << p_Sector[Cnt].SectorX;
//        *p_Packet << p_Sector[Cnt].SectorY;
//    }
//
//    return p_Packet;
//}


//------------------------------------------
// Contents proc
//------------------------------------------

void Player::GameUpdate(ULONGLONG CurTime)
{
    _UpdateTime = CurTime;
    _IsMove = CheckMoveDelay();
    _IsAttack = CheckAttackDelay();
    //_IsFind = true;

    if (true == _IsMove)
    {
        if (_PathSize != 0 || _PathIndex < _PathSize)
        {
            if (_NextMoveTime <= CurTime)
            {
                ProfileBegin(L"Action_Move");
                Action_Move();
                ProfileEnd(L"Action_Move");
            }
        }
    }

    if (true == _IsActionReset)
    {
        ProfileBegin(L"ActionReset");
        ActionReset();
        _IsActionReset = false;
        ProfileEnd(L"ActionReset");
        return;
    }

    if (true == _IsAttack)
    {
        // 쿨타임 확인.
        if (_NextAttackTime < _UpdateTime)
        {
            float AttackRange;
            float TargetDistance;

            // 3. 공격가능 거리인지 확인.
            AttackRange = (float)(g_Pattern_AttackRange[_AttackType][_CharacterType]);
            TargetDistance = Distance(_TileX, _TileY, _p_Target->_TileX, _p_Target->_TileY);
            if (TargetDistance > AttackRange)
            {
                if (0 == _PathSize || _PathIndex >= _PathSize ||
                    _TargetTileX != _p_Target->_TileX ||
                    _TargetTileY != _p_Target->_TileY)
                {
                    // Attack Path Delay Check
                    if (_AttackMoveCheckTime <= _UpdateTime)
                    {
                        _AttackMoveCheckTime = _UpdateTime + dfATTACK_CHECK_MOVE;

                        ProfileBegin(L"StartRemainTime");
                        StartRemainMoveTime();
                        ProfileEnd(L"StartRemainTime");

                        ProfileBegin(L"FindAttackPath");
                        if (false == FindAttackPath())
                            ActionReset();
                        else
                            EndRemainMoveTime();
                        ProfileEnd(L"FindAttackPath");
                    }
                }
            }
            else
            {
                // 공격거리 안이다.    // 일단 정지.
                if (_PathSize != 0 && _PathIndex < _PathSize)
                {
                    // 패스를 리셋한다.
                    MoveReset();
                }

                //// 쿨타임 확인.
                //if (_NextAttackTime < _UpdateTime)
                //{
                    //ProfileBegin(L"Action_Attack");
                    Action_Attack();
                    //ProfileEnd(L"Action_Attack");    
                //}

            }
        }
        
    }

    //if (OBJECT_ID_DEFAULT == _TargetID)
    //{
    //    if (OBJECT_ID_DEFAULT == _TargetID)
    //    {
    //        _IsAttack = false;
    //    }
    //    else if (_p_Target->_ObjectID != _TargetID)
    //    {
    //        AttackReset();
    //        _IsAttack = false;
    //    }
    //    else if (_p_Target->_HP < 1)
    //    {
    //        AttackReset();
    //        _IsAttack = false;
    //    }
    //
    //    //if (true == _IsAttack && _TargetID != OBJECT_ID_DEFAULT)
    //    if (true == _IsAttack)
    //    {
    //        // 쿨타임 확인.
    //        if (_NextAttackTime < _UpdateTime)
    //        {
    //            float AttackRange;
    //            float TargetDistance;
    //
    //            // 3. 공격가능 거리인지 확인.
    //            AttackRange = (float)(g_Pattern_AttackRange[_AttackType][_CharacterType]);
    //            TargetDistance = Distance(_TileX, _TileY, _p_Target->_TileX, _p_Target->_TileY);
    //            if (TargetDistance > AttackRange)
    //            {
    //                if (0 == _PathSize || _PathIndex >= _PathSize)
    //                {
    //                    _DestTileX = _p_Target->_TileX;
    //                    _DestTileY = _p_Target->_TileY;
    //                    return true;
    //                }
    //                else
    //                {
    //                    return false;
    //                }
    //            }
    //            else
    //            {
    //                // 공격거리 안이다.    // 일단 정지.
    //                if (_PathSize != 0 && _PathIndex < _PathSize)
    //                {
    //                    // 패스를 리셋한다.
    //                    MoveReset();
    //                }
    //
    //                //ProfileBegin(L"Action_Attack");
    //                _IsFind = Action_Attack();
    //                //ProfileEnd(L"Action_Attack");    
    //            }
    //        }
    //    }
    //}
    else if (true == _IsMove)
    {
        bool IsFind = true;

        if (TILE_DEFAULT_X == _DestTileX || TILE_DEFAULT_Y == _DestTileY)
            IsFind = false;
        else if (abs(_TileX - _DestTileX) > SEARCH_RANGE_MAX_X || abs(_TileY - _DestTileY) > SEARCH_RANGE_MAX_Y)
            IsFind = false;
        else if (_PathSize != 0 && _PathIndex < _PathSize &&
            _DestTileX == _TilePath[_PathSize - 1].X && _DestTileY == _TilePath[_PathSize - 1].Y)
            IsFind = false;

        if (true == IsFind)
        {
            ProfileBegin(L"StartRemainTime");
            StartRemainMoveTime();
            ProfileEnd(L"StartRemainTime");

            ProfileBegin(L"FindMovePath");
            if (false == FindMovePath())
                ActionReset();
            else
                EndRemainMoveTime();
            ProfileEnd(L"FindMovePath");
        }
    }

    if (_SectorX != TILE_to_SECTOR_X(_TileX) || _SectorY != TILE_to_SECTOR_Y(_TileY))
    {
        ProfileBegin(L"SectorUpdate");
        if (true == SectorUpdate())
        {
            //ProfileBegin(L"SectorUpdateSend");
            SendPacket_SectorUpdate();
            //ProfileEnd(L"SectorUpdateSend");
        }
        ProfileEnd(L"SectorUpdate");
    }
}

void Player::Action_Move(void)
{
    if (_PathIndex < 0)             // 이상한 값이다.
        CrashDump::Crash();

    if (_TilePath[_PathIndex].X == _TileX && _TilePath[_PathIndex].Y == _TileY)
    {
        _PathIndex++;
        if (_PathIndex >= _PathSize)
        {
            MoveReset();
            return;
        }
    }

    en_DIRECTION Dir;
    ULONGLONG MoveTime;
    bool IsStop;
    int MoveX;
    int MoveY;
    int CurTileX;
    int CurTileY;

    Dir = MoveDirection(_TileX, _TileY, _TilePath[_PathIndex].X, _TilePath[_PathIndex].Y);
    if (eMOVE_NN == Dir)
    {
        // 이건 생각을 더 해보자.(길을 새로 찾아서 돌리는 방법도 있을 듯.)
        return;
    }

    MoveTime = _NextMoveTime;
    IsStop = false;
    MoveX = -1;
    MoveY = -1;
    CurTileX = _TileX;
    CurTileY = _TileY;
    while (MoveTime <= _UpdateTime)
    {
        // 다음 타일을 얻어온다.
        //MoveTile(Dir, _TileX, _TileY, &MoveX, &MoveY);
        //MoveTile(Dir, CurTileX, CurTileY, &MoveX, &MoveY);
        switch (Dir)
        {
        case eMOVE_NN:
            MoveX = -1;
            MoveY = -1;
            break;
        case eMOVE_UU:
            MoveX = _TileX;
            MoveY = _TileY - 1;
            break;
        case eMOVE_RU:
            MoveX = _TileX + 1;
            MoveY = _TileY - 1;
            break;
        case eMOVE_RR:
            MoveX = _TileX + 1;
            MoveY = _TileY;
            break;
        case eMOVE_RD:
            MoveX = _TileX + 1;
            MoveY = _TileY + 1;
            break;
        case eMOVE_DD:
            MoveX = _TileX;
            MoveY = _TileY + 1;
            break;
        case eMOVE_LD:
            MoveX = _TileX - 1;
            MoveY = _TileY + 1;
            break;
        case eMOVE_LL:
            MoveX = _TileX - 1;
            MoveY = _TileY;
            break;
        case eMOVE_LU:
            MoveX = _TileX - 1;
            MoveY = _TileY - 1;
            break;
        }

        if (MoveX < 0 || MoveY < 0 || MoveX >= TILE_MAX_X || MoveY >= TILE_MAX_Y)
        {
            MoveX = -1;
            MoveY = -1;
            break;
        }

        // 패스 체크
        if (_TilePath[_PathIndex].X == MoveX && _TilePath[_PathIndex].Y == MoveY)
        {
            _PathIndex++;
            if (_PathIndex >= _PathSize)
            {
                // 목적지 이므로 정지.
                IsStop = true;
                break;
            }

            // 꺾어준다.
            Dir = MoveDirection(MoveX, MoveY, _TilePath[_PathIndex].X, _TilePath[_PathIndex].Y);
        }
        CurTileX = MoveX;
        CurTileY = MoveY;
        MoveTime += NextTileTime(Dir);
    }

    // Tile Update
    // 제거가 안되는 경우는 없다.
    if (false == RemoveTileObject(this, _TileX, _TileY, OBJECT_TYPE_PLAYER))
    {
        CrashDump::Crash();
        return;
    }
    // 추가가 불가능한 경우가 발생한다.(푸시로 인한 밀림으로 패스 깨짐.)
    if (false == AddTileObject(this, CurTileX, CurTileY, OBJECT_TYPE_PLAYER))
    {
        // 추가가 불가능한 경우에는 기존 타일에 재추가하고 패스를 다시 찾는다.
        if (false == AddTileObject(this, _TileX, _TileY, OBJECT_TYPE_PLAYER))
        {
            // 밖에서 체크한다.
            CrashDump::Crash();
            return;
        }
        FindMovePath();
        SYSLOG(L"Action_Move", LOG_ERROR, L"AddTile ERROR # [AccountNo:%lld][TileX:%d][TileY:%d][NewTileX:%d][NewTileY:%d][TileCnt:%d]", _AccountNo, _TileX, _TileY, MoveX, MoveY, _pp_MapTile[MoveY][MoveX].ObjectNum);
        return;
    }
    else
    {
        _TileX = CurTileX;
        _TileY = CurTileY;

        // Sector는 여기서 업데이트하지 않는다.(SectorUpdate() 호출)
        _NextMoveTime = MoveTime;
        _PosX = TILE_to_POS_X(_TileX);
        _PosY = TILE_to_POS_Y(_TileY);

        if (true == IsStop)
        {
            MoveReset();
            _DestTileX = TILE_DEFAULT_X;
            _DestTileY = TILE_DEFAULT_Y;

            //Packet *p_MakePacket;
            //p_MakePacket = PacketMake_StopCharacter(_ObjectID, _PosX, _PosY, (USHORT)0xffff);
            //SendPacket(p_MakePacket);
            //p_MakePacket->Free();
        }
    }
}
void Player::Action_Attack(void)
{
    //float AttackRange;
    //float TargetDistance;

    Packet *p_MakePacket;
    int Damage;
    int CoolTime;
    bool IsUnderAttack;

    //// 3. 공격가능 거리인지 확인.
    //ProfileBegin(L"Action_Attack4");
    //AttackRange = (float)(g_Pattern_AttackRange[_AttackType][_CharacterType]);
    //TargetDistance = Distance(_TileX, _TileY, _p_Target->_TileX, _p_Target->_TileY);
    //if (TargetDistance > AttackRange)
    //{
    //    if (0 == _PathSize || _PathIndex >= _PathSize)
    //    {
    //        _DestTileX = _p_Target->_TileX;
    //        _DestTileY = _p_Target->_TileY;
    //        ProfileEnd(L"Action_Attack4");
    //        return true;
    //    }
    //    else
    //    {
    //        ProfileEnd(L"Action_Attack4");
    //        return false;
    //    }
    //}
    //ProfileEnd(L"Action_Attack4");
    //
    //// 공격거리 안이다.    // 일단 정지.
    //ProfileBegin(L"Action_Attack5");
    //if (_PathSize != 0 && _PathIndex < _PathSize)
    //{
    //    // 패스를 리셋한다.
    //    MoveReset();
    //}
    //ProfileEnd(L"Action_Attack5");

    // 공격시작
    ProfileBegin(L"Atk_Start1");
    Damage = g_Pattern_AttackPower[_AttackType][_CharacterType];
    CoolTime = g_Pattern_AttackTime[_AttackType][_CharacterType];
    _NextAttackTime = _UpdateTime + CoolTime;

    // 이동중 공격이라면 이동 딜레이 처리
    SetMoveDelay(((ULONGLONG)dfATTACK_STOP_TIME));
    ProfileEnd(L"Atk_Start1");

    // Attack 패킷을 뿌린다.
    ProfileBegin(L"Atk_Start2");
    p_MakePacket = PacketMake_Attack(_AttackType, _ObjectID, _p_Target->_ObjectID, CoolTime, _PosX, _PosY);
    SendPacket_SectorAround(p_MakePacket);
    p_MakePacket->Free();
    ProfileEnd(L"Atk_Start2");

    // UnderAttack 체크
    // 여기에 UnderAttack을 추가한다.(더미 간의 공격은 공격알림 제외.)
    ProfileBegin(L"Atk_Start3");
    IsUnderAttack = false;
    if (_UserType != USER_TYPE_DUMMY || _p_Target->_UserType != USER_TYPE_DUMMY)
    {
        if (Player::UnderAttackLastTime + UNDER_ATTACK_SEND_TIME < _UpdateTime)
        {
            IsUnderAttack = true;
            Player::UnderAttackLastTime = _UpdateTime;
        }
    }
    ProfileEnd(L"Atk_Start3");

    // 공격 로직 시작
    switch (_AttackType)
    {
    case ATTACK_TYPE_1:
        // 일반 공격
        ProfileBegin(L"AttackProc");
        SingleAttackProc(_p_Target, Damage);
        ProfileEnd(L"AttackProc");
        break;
    case ATTACK_TYPE_2:
        // 광역공격
        ProfileBegin(L"AreaAttackProc");
        AreaAttackProc(_p_Target->_TileX, _p_Target->_TileY, Damage);
        ProfileEnd(L"AreaAttackProc");
        break;
    default:
        // AttackType에 문제가 있거나 AttackReset이 제대로 이루어지지 않았다.
        CrashDump::Crash();
        //return false;
    }

    // UnderAttack 체크
    // 여기에 UnderAttack을 추가한다.(더미 간의 공격은 공격알림 제외.)
    ProfileBegin(L"Atk_Start4");
    if (true == IsUnderAttack)
    {
        p_MakePacket = PacketMake_UnderattackPos(_PosX, _PosY);
        _p_GameServer->SendPacket_Broadcast(p_MakePacket);
        p_MakePacket->Free();
    }
    ProfileEnd(L"Atk_Start4");

    //return false;
}

void Player::ActionReset(void)
{
    // 모든 액션을 리셋.
    MoveReset();
    AttackReset();
    _DestTileX = TILE_DEFAULT_X;
    _DestTileY = TILE_DEFAULT_Y;
}
void Player::MoveReset(void)
{
    _PathIndex = 0;
    _PathSize = 0;
    _RemainMoveTime = 0;
}
void Player::AttackReset(void)
{
    _p_Target = nullptr;
    _TargetID = OBJECT_ID_DEFAULT;
    _TargetTileX = TILE_DEFAULT_X;
    _TargetTileY = TILE_DEFAULT_Y;
    _AttackType = ATTACK_TYPE_DEFAULT;
}

void Player::ResetRemainMoveTime(void)
{
    _RemainMoveTime = 0;
}
void Player::StartRemainMoveTime(void)
{
    en_DIRECTION Dir;
    ULONGLONG OldTileTime;

    _RemainMoveTime = 0;
    if (_PathSize != 0 && _PathIndex < _PathSize)
    {
        // 이동중이었다면 보정해준다.
        Dir = MoveDirection(_TileX, _TileY, _TilePath[_PathIndex].X, _TilePath[_PathIndex].Y);
        if (eMOVE_NN == Dir)
        {
            //SYSLOG(L"MoveAction", LOG_ERROR, L"RemainMoveTime Dir ERROR # [AccountNo:%lld][TileX:%d][TileY:%d][PathX:%d][PathY:%d]",
            //    _AccountNo, _TileX, _TileY, POS_to_TILE_X(_Path[_PathIndex].X), POS_to_TILE_Y(_Path[_PathIndex].Y));
        }
        else
        {
            OldTileTime = NextTileTime(Dir);
            if (_NextMoveTime > _UpdateTime)
            {
                _RemainMoveTime = _NextMoveTime - _UpdateTime;
                if (_RemainMoveTime > OldTileTime)
                {
                    //wprintf_s(L"RemainMoveTime Count ERROR # [AccountNo:%lld][RemainMoveTime:%llu][OldTime:%llu]",
                    //    _AccountNo, _RemainMoveTime, OldTileTime);
                    SYSLOG(L"MoveAction", LOG_ERROR, L"RemainMoveTime Count ERROR # [AccountNo:%lld][RemainMoveTime:%llu][OldTime:%llu]",
                        _AccountNo, _RemainMoveTime, OldTileTime);
                    _RemainMoveTime = OldTileTime;
                }
            }
        }
    }
}
void Player::EndRemainMoveTime(void)
{
    en_DIRECTION Dir;
    ULONGLONG OldTileTime;
    ULONGLONG NewTileTime;

    //if (_RemainMoveTime != 0)
    //{
    //    OldTileTime = _RemainMoveTime;
    //
    //    Dir = MoveDirection(_TileX, _TileY, _TilePath[_PathIndex].X, _TilePath[_PathIndex].Y);
    //    NewTileTime = NextTileTime(Dir);
    //
    //    //New * _remain / old
    //    _RemainMoveTime = NewTileTime * _RemainMoveTime / OldTileTime;
    //}
}

bool Player::FindMovePath(void)
{
    // 목적지가 잘못되면 패스를 설정하지 않는다.
    if (false == _p_PathFinder->CheckMapAttribute(_DestTileX, _DestTileY))
    {
        return false;
    }
    if (_DestTileX == _TileX && _DestTileY == _TileY)
    {
        // 이 경우는 패스파인더 안에서도 체크하지만 패스파인더의 호출을 줄이기 위함.
        return false;
    }

    int SearchSize;
    en_DIRECTION Dir;
    Packet *p_MakePacket;

    //ProfileBegin(L"FindMovePath1");
    SearchSize = _p_PathFinder->Search(_TileX, _TileY, _DestTileX, _DestTileY, _TilePath);
    if (SearchSize < 1)
    {
        SYSLOG(L"FindMovePath", LOG_ERROR, L"JPS Error # [AccountNo:%lld][TileX:%d][TileY:%d][DestX:%d][DestY:%d]",
            _AccountNo, _TileX, _TileY, _DestTileX, _DestTileY);
        return false;
    }
    _PathIndex = 0;
    _PathSize = SearchSize;
    //ProfileEnd(L"FindMovePath1");

    // 이동경로를 재설정한다.
    //ProfileBegin(L"FindMovePath2");
    Dir = MoveDirection(_TileX, _TileY, _TilePath[0].X, _TilePath[0].Y);
    _NextMoveTime = _UpdateTime + NextTileTime(Dir);
    //ProfileEnd(L"FindMovePath2");

    //ProfileBegin(L"FindMovePath3");
    p_MakePacket = PacketMake_MoveCharacter(this);
    SendPacket_SectorAround(p_MakePacket);
    p_MakePacket->Free();
    //ProfileEnd(L"FindMovePath3");

    return true;
}
bool Player::FindAttackPath(void)
{
    //ProfileBegin(L"FindAttackPath1");

    int OldDestX;
    int OldDestY;

    int SearchSize;
    en_DIRECTION Dir;
    int AttackRange;
    float TargetDistance;

    int CurTileX;
    int CurTileY;
    int NextTileX;
    int NextTileY;
    int MoveX;
    int MoveY;
    Packet *p_MakePacket;

    if (true == IsAttackRange(_p_Target->_TileX, _p_Target->_TileY))
    {
        // 이미 공격거리 안이다.(이 때는 예외적으로 패스를 삭제한다.)
        MoveReset();
        //ProfileEnd(L"FindAttackPath1");
        return true;
    }
    //ProfileEnd(L"FindAttackPath1");

    OldDestX = TILE_DEFAULT_X;
    OldDestY = TILE_DEFAULT_Y;
    if (_PathSize != 0 && _PathIndex < _PathSize)
    {
        OldDestX = _TilePath[_PathSize - 1].X;
        OldDestY = _TilePath[_PathSize - 1].Y;
    }

    //ProfileBegin(L"FindAttackPath2");
    SearchSize = _p_PathFinder->Search(_TileX, _TileY, _p_Target->_TileX, _p_Target->_TileY, _TilePath);
    if (SearchSize < 1)
    {
        SYSLOG(L"FindAttackPath", LOG_ERROR, L"JPS Error1 # [AccountNo:%lld][TileX:%d][TileY:%d][DestX:%d][DestY:%d]", _AccountNo, _TileX, _TileY, _p_Target->_TileX, _p_Target->_TileY);
        //ProfileEnd(L"FindAttackPath2");
        return false;
    }
    //ProfileEnd(L"FindAttackPath2");

    // 도착지점을 설정.
    //ProfileBegin(L"FindAttackPath3");
    int ReverseIndex;
    int DistX;
    int DistY;
    int DirX;
    int DirY;
    int Cnt;

    ReverseIndex = SearchSize - 1;
    CurTileX = _TilePath[ReverseIndex].X;
    CurTileY = _TilePath[ReverseIndex].Y;

    AttackRange = g_Pattern_AttackRange[_AttackType][_CharacterType];
    if (1 == SearchSize)
    {
        NextTileX = _TileX;
        NextTileY = _TileY;

        if (CurTileX == NextTileX)
        {
            DistY = abs(CurTileY - NextTileY);
            if (DistY <= AttackRange)
                CrashDump::Crash();
            DirY = (CurTileY - NextTileY) / DistY;
            if (DirY != 1 && DirY != -1)
                CrashDump::Crash();
            CurTileY = NextTileY + DirY * AttackRange;
        }
        else if (CurTileY == NextTileY)
        {
            DistX = abs(CurTileX - NextTileX);
            if (DistX <= AttackRange)
                CrashDump::Crash();
            DirX = (CurTileX - NextTileX) / DistX;
            if (DirX != 1 && DirX != -1)
                CrashDump::Crash();
            CurTileX = NextTileX + DirX * AttackRange;
        }
        else
        {
            DistX = abs(CurTileX - NextTileX);
            DistY = abs(CurTileY - NextTileY);
            DirX = (NextTileX - CurTileX) / DistX;        // 이건 특수한 경우다.
            if (DirX != 1 && DirX != -1)
                CrashDump::Crash();
            DirY = (NextTileY - CurTileY) / DistY;
            if (DirY != 1 && DirY != -1)
                CrashDump::Crash();

            for (Cnt = 0; Cnt < DistX; ++Cnt)
            {
                TargetDistance = Distance(CurTileX, CurTileY, NextTileX, NextTileY);
                if (TargetDistance <= (float)AttackRange)
                    break;
                CurTileX += DirX;
                CurTileY += DirY;
            }
        }
    }
    else
    {
        while (ReverseIndex != 0)
        {
            ReverseIndex--;
            NextTileX = _TilePath[ReverseIndex].X;
            NextTileY = _TilePath[ReverseIndex].Y;

            // 거리 체크
            TargetDistance = Distance(NextTileX, NextTileY, _p_Target->_TileX, _p_Target->_TileY);
            if (TargetDistance > (float)AttackRange)
            {
                // 이전 타일이 목적지이다.
                ReverseIndex++;
                break;
            }

            CurTileX = NextTileX;
            CurTileY = NextTileY;

            if (0 == ReverseIndex)
            {
                NextTileX = _TileX;
                NextTileY = _TileY;
                //ReverseIndex--;
                break;
            }
        }

        Dir = MoveDirection(CurTileX, CurTileY, NextTileX, NextTileY);

        MoveX = -1;
        MoveY = -1;
        while (1)
        {
            // 다음 타일을 얻어온다.
            MoveTile(Dir, CurTileX, CurTileY, &MoveX, &MoveY);

            // 거리 체크
            TargetDistance = Distance(MoveX, MoveY, _p_Target->_TileX, _p_Target->_TileY);
            if (TargetDistance > (float)AttackRange)
            {
                // 이전 타일이 목적지이다.
                break;
            }

            // 패스 체크
            if (NextTileX == MoveX && NextTileY == MoveY)
            {
                break;
            }

            //OldTileX = CurTileX;
            //OldTileY = CurTileY;
            CurTileX = MoveX;
            CurTileY = MoveY;
        }
    }
    //ProfileEnd(L"FindAttackPath3");

    //ProfileBegin(L"FindAttackPath4");
    //ReverseIndex++;
    _TilePath[ReverseIndex].X = CurTileX;
    _TilePath[ReverseIndex].Y = CurTileY;

    if (OldDestX != TILE_DEFAULT_X && OldDestY != TILE_DEFAULT_Y)
    {
        if (OldDestX == _TilePath[ReverseIndex].X && OldDestY == _TilePath[ReverseIndex].Y)
            return true;
    }

    _PathIndex = 0;
    _PathSize = ReverseIndex + 1;
    //ProfileEnd(L"FindAttackPath4");

    // 이동경로를 재설정한다.
    //ProfileBegin(L"FindAttackPath5");
    Dir = MoveDirection(_TileX, _TileY, _TilePath[0].X, _TilePath[0].Y);
    _NextMoveTime = _UpdateTime + NextTileTime(Dir);
    //_DestTileX = _TilePath[_PathSize - 1].X;
    //_DestTileY = _TilePath[_PathSize - 1].Y;
    //ProfileEnd(L"FindAttackPath5");

    // 길을 찾았으므로 타겟 좌표를 갱신한다.
    _TargetTileX = _p_Target->_TileX;
    _TargetTileY = _p_Target->_TileY;

    //wprintf_s(L"[TileX:%d][TileX:%d][TargetX:%d][TargetX:%d][PathSize:%d]\n", _TileX, _TileY, _p_Target->_TileX, _p_Target->_TileY, _PathSize);
    //for (int Cnt = 0; Cnt < _PathSize; ++Cnt)
    //{
    //    wprintf_s(L"--[PathX:%d][PathY:%d]\n\n", _TilePath[Cnt].X, _TilePath[Cnt].Y);
    //}

    // 이동 시작
    //ProfileBegin(L"FindAttackPath6");
    p_MakePacket = PacketMake_MoveCharacter(this);
    SendPacket_SectorAround(p_MakePacket);
    p_MakePacket->Free();
    //ProfileEnd(L"FindAttackPath6");

    return true;
}

bool Player::IsAttackRange(int TargetTileX, int TargetTileY)
{
    // 공격범위를 체크한다.
    if (_AttackType < ATTACK_TYPE_1 || _AttackType > ATTACK_TYPE_2)
    {
        CrashDump::Crash();
        return false;
    }

    float AttackRange;
    float TargetDistance;

    // 3. 공격가능 거리인지 확인.
    AttackRange = (float)(g_Pattern_AttackRange[_AttackType][_CharacterType]);
    TargetDistance = Distance(_TileX, _TileY, TargetTileX, TargetTileY);
    if (TargetDistance <= AttackRange)
        return true;
    return false;
}
bool Player::IsAttackMove(int TargetTileX, int TargetTileY)
{
    // 대상 좌표가 달라졌는지 여부를 확인한다.
    if (TargetTileX != _DestTileX || TargetTileY != _DestTileY)
        return true;

    // 대상을 향해 이동중인지 파악한다.
    if (0 == _PathSize)
        return true;        // 이동중이 아니라면 패스를 찾는다.(이건 시간 체크안함.)

    return false;
}
bool Player::GetTileTarget(int TargetTileX, int TargetTileY, st_SECTOR_TARGET *SectorTarget)
{
    if (SectorTarget->Count == AREA_ATTACK_OBJECT_MAX_COUNT)
        return false;
    else if (SectorTarget->Count > AREA_ATTACK_OBJECT_MAX_COUNT)
        CrashDump::Crash();

    st_MAP_TILE *p_MapTile;
    int ObjectCnt;
    int TargetCnt;
    int TargetMax;

    if (false == _p_PathFinder->CheckMapAttribute(TargetTileX, TargetTileY))
        return false;
    p_MapTile = &_pp_MapTile[TargetTileY][TargetTileX];
    if (p_MapTile->ObjectNum < 1)
        return false;
    
    TargetMax = p_MapTile->ObjectNum;
    if (p_MapTile->ObjectNum > MAP_TILE_OBJECT_CHECK)
        TargetMax = MAP_TILE_OBJECT_CHECK;

    TargetCnt = 0;
    for (ObjectCnt = 0; ObjectCnt < MAP_TILE_OBJECT_MAX; ++ObjectCnt)
    {
        if (nullptr == p_MapTile->Object[ObjectCnt].p_Object)
            continue;
        if (true == p_MapTile->Object[ObjectCnt].p_Object->_IsFirstJoin)
            continue;
        if (p_MapTile->Object[ObjectCnt].p_Object == this)
            continue;
        if (_CharacterType != dfGAME_CHARACTER_ELF)         // ELF는 모두 공격 가능.
        {
            if (p_MapTile->Object[ObjectCnt].p_Object->_CharacterType != dfGAME_CHARACTER_ELF)
            {
                if (p_MapTile->Object[ObjectCnt].p_Object->_Party == _Party)
                    continue;
            }
        }
        if (p_MapTile->Object[ObjectCnt].ObjectType != OBJECT_TYPE_PLAYER)
            CrashDump::Crash();

        SectorTarget->TargetArr[SectorTarget->Count] = p_MapTile->Object[ObjectCnt].p_Object;
        SectorTarget->Count++;
        TargetCnt++;

        if (TargetCnt == TargetMax)
            break;
        else if (TargetCnt > TargetMax)
            CrashDump::Crash();
    }
    return true;
}
void Player::SectorAttackProc(st_SECTOR_TARGET *SectorTarget, short SectorX, short SectorY, int Damage, bool IsPush)
{
    int TargetCnt;
    int DmgCnt;
    st_DAMAGE DmgArr[AREA_ATTACK_OBJECT_MAX_COUNT];

    Player *p_Target;
    Packet *p_MakePacket;

    DmgCnt = 0;
    for (TargetCnt = 0; TargetCnt < SectorTarget->Count; ++TargetCnt)
    {
        p_Target = SectorTarget->TargetArr[TargetCnt];

        DmgArr[DmgCnt].DamageObjectID = p_Target->_ObjectID;
        DmgArr[DmgCnt].DamageValue = Damage;
        DmgArr[DmgCnt].PushPosX = 0.0f;
        DmgArr[DmgCnt].PushPosY = 0.0f;
        DmgArr[DmgCnt].Die = DIE_FLAG_OFF;

        AttackProc(p_Target, &DmgArr[DmgCnt], IsPush);
        DmgCnt++;
    }

    // 결과 패킷을 보낸다.
    p_MakePacket = PacketMake_DamageGroup(DmgArr, DmgCnt);
    SendPacket_SectorAround(p_MakePacket, SectorX, SectorY, nullptr);
    p_MakePacket->Free();
}
void Player::AreaAttackProc(int CenterTileX, int CenterTileY, int Damage, bool IsPush)
{
    st_SECTOR_AROUND_TARGET SectorAroundTarget;
    int SectorCnt;
    short CenterSectorX;
    short CenterSectorY;
    st_SECTOR_TARGET *p_SectorTarget;

    int AreaCnt;
    int DistX;
    int DistY;
    int CheckTileX;
    int CheckTileY;
    short CheckSectorX;
    short CheckSectorY;

    int AreaTargetCnt;

    // 초기화
    SectorAroundTarget.Count = 0;
    for (SectorCnt = 0; SectorCnt < 9; ++SectorCnt)
        SectorAroundTarget.SectorTarget[SectorCnt].Count = 0;

    // 섹터 정보를 불러온다.
    CenterSectorX = TILE_to_SECTOR_X(CenterTileX);
    CenterSectorY = TILE_to_SECTOR_Y(CenterTileY);
    GetSectorAround(CenterSectorX, CenterSectorY, (st_SECTOR_AROUND *)&SectorAroundTarget);

    // 타겟 선정.
    AreaTargetCnt = 0;
    for (AreaCnt = 0; AreaCnt < dfPATTERN_ATTACK_AREA_MAX; ++AreaCnt)
    {
        DistX = g_Pattern_AttackArea[_CharacterType][AreaCnt][0];
        DistY = g_Pattern_AttackArea[_CharacterType][AreaCnt][1];

        if (255 == DistX || 255 == DistY)
            break;

        //p_Target = nullptr;
        CheckTileX = CenterTileX + DistX;
        CheckTileY = CenterTileY + DistY;

        CheckSectorX = TILE_to_SECTOR_X(CheckTileX);
        CheckSectorY = TILE_to_SECTOR_Y(CheckTileY);

        p_SectorTarget = nullptr;
        for (SectorCnt = 0; SectorCnt < SectorAroundTarget.Count; ++SectorCnt)
        {
            if (CheckSectorX == SectorAroundTarget.Around[SectorCnt].SectorX &&
                CheckSectorY == SectorAroundTarget.Around[SectorCnt].SectorY)
            {
                p_SectorTarget = &SectorAroundTarget.SectorTarget[SectorCnt];
                break;
            }
        }
        if (nullptr == p_SectorTarget)
        {
            CrashDump::Crash();
        }

        // 타일의 오브젝트 검사.
        GetTileTarget(CheckTileX, CheckTileY, p_SectorTarget);

        AreaTargetCnt += p_SectorTarget->Count;
        if (AreaTargetCnt >= AREA_ATTACK_OBJECT_MAX_COUNT)
            break;
    }

    // 공격 처리.
    for (SectorCnt = 0; SectorCnt < SectorAroundTarget.Count; ++SectorCnt)
    {
        if (SectorAroundTarget.SectorTarget[SectorCnt].Count > 0)
        {
            SectorAttackProc(&SectorAroundTarget.SectorTarget[SectorCnt],
                SectorAroundTarget.Around[SectorCnt].SectorX, SectorAroundTarget.Around[SectorCnt].SectorY, Damage, IsPush);
        }
    }
}
void Player::SingleAttackProc(Player *p_Target, int Damage, bool IsPush)
{
    if (true == p_Target->_IsFirstJoin)
        return;

    st_DAMAGE DamageObj;
    Packet *p_MakePacket;

    DamageObj.DamageObjectID = p_Target->_ObjectID;
    DamageObj.DamageValue = Damage;
    DamageObj.PushPosX = 0.0f;
    DamageObj.PushPosY = 0.0f;
    DamageObj.Die = DIE_FLAG_OFF;

    AttackProc(p_Target, &DamageObj, IsPush);

    // Damage 패킷을 뿌린다.(이건 무조건 뿌린다.)
    //p_MakePacket = PacketMake_Damage(p_Target->_ObjectID, Damage, DamageObj.PushPosX, DamageObj.PushPosY);
    p_MakePacket = PacketMake_Damage(p_Target->_ObjectID, DamageObj.DamageValue, DamageObj.PushPosX, DamageObj.PushPosY);
    p_Target->SendPacket_SectorAround(p_MakePacket);
    p_MakePacket->Free();

    if (DIE_FLAG_ON == DamageObj.Die)
    {
        // Die 패킷 보낸다.
        p_MakePacket = PacketMake_Die(p_Target->_ObjectID);
        p_Target->SendPacket_SectorAround(p_MakePacket);
        p_MakePacket->Free();
    }
    //else
    //{
    //    // Damage 패킷을 뿌린다.
    //    p_MakePacket = PacketMake_Damage(p_Target->_ObjectID, Damage, DamageObj.PushPosX, DamageObj.PushPosY);
    //    p_Target->SendPacket_SectorAround(p_MakePacket);
    //    p_MakePacket->Free();
    //}
}
void Player::AttackProc(Player *p_Target, st_DAMAGE *p_Damage, bool IsPush)
{
    if (nullptr == p_Target)
    {
        CrashDump::Crash();
        return;
    }

    bool IsGuestTarget;
    float PushPosX;
    float PushPosY;
    int TargetOldTileX;
    int TargetOldTileY;
    Packet *p_MakePacket;

    IsGuestTarget = false;

    if (_UserType != USER_TYPE_DUMMY && USER_TYPE_DUMMY == p_Target->_UserType)
    {
        p_Damage->DamageValue = (short)(p_Damage->DamageValue * 2.5);
    }

    p_Target->_HP -= p_Damage->DamageValue;
    if (p_Target->_HP < 1)
    {
        //// Die 처리.
        //if (_UserType != USER_TYPE_DUMMY && p_Target->_UserType != USER_TYPE_DUMMY)
        //{
        //    // 토탈 킬수 증가
        //    _KillCount++;
        //
        //    // 일단 DB에 데이터를 저장한다.
        //    if (USER_TYPE_GUEST == p_Target->_UserType)
        //    {
        //        // 게스트 킬수 증가
        //        _GuestKillCount++;
        //
        //        IsGuestTarget = true;
        //    }
        //
        //    DBGameWritePlayerKill(p_Target->_AccountNo, _TileX, _TileY, IsGuestTarget);
        //    p_Target->DBGameWritePlayerDie(_AccountNo);
        //}
        //else
        //{
        //    // 공격자와 타겟 둘 중하나가 더미인 경우는 로그만 남긴다.
        //    DBGameWriteKillLog(p_Target->_AccountNo, _TileX, _TileY, IsGuestTarget);
        //    p_Target->DBGameWriteDieLog(_AccountNo);
        //}

        //if (_UserType != USER_TYPE_DUMMY && p_Target->_UserType != USER_TYPE_DUMMY)
        if (_UserType != USER_TYPE_DUMMY)
        {
            // 일단 DB에 데이터를 저장한다.
            if (USER_TYPE_DUMMY == p_Target->_UserType)
            {
                // 게스트 킬수 증가
                _GuestKillCount++;

                IsGuestTarget = true;

                //p_MakePacket = PacketMake_BotKill();
                //SendPacket(p_MakePacket);
                //p_MakePacket->Free();
            }
            else
            {
                // 토탈 킬수 증가
                _KillCount++;
            }

            DBGameWritePlayerKill(p_Target->_AccountNo, _TileX, _TileY, IsGuestTarget);
            p_Target->DBGameWritePlayerDie(_AccountNo);
        }
        else
        {
            // 공격자가 더미인 경우는 로그만 남긴다.
            DBGameWriteKillLog(p_Target->_AccountNo, _TileX, _TileY, IsGuestTarget);
            p_Target->DBGameWriteDieLog(_AccountNo);
        }

        // 타겟의 데이터 수정
        p_Target->_HP = 0;
        p_Target->ActionReset();
        if (false == RemoveTileObject(p_Target, p_Target->_TileX, p_Target->_TileY, OBJECT_TYPE_PLAYER))
        {
            CrashDump::Crash();
            return;
        }

        //// Stop을 보낸다.
        //p_MakePacket = PacketMake_StopCharacter(p_Target->_ObjectID, p_Target->_PosX, p_Target->_PosY, (USHORT)(0xffff));
        //p_Target->SendPacket_SectorAround(p_MakePacket);
        //p_MakePacket->Free();

        p_Damage->Die = DIE_FLAG_ON;

        // 액션 초기화(TargetID 초기화 포함) -> 내 타겟일 때만 초기화.
        if (_TargetID == p_Target->_ObjectID)
        {
            ActionReset();
        }
    }
    else
    {
        // 오버힐 처리.
        if (p_Target->_HP > dfHP_MAX)
            p_Target->_HP = dfHP_MAX;

        // 기본은 밀리지 않는다.
        PushPosX = 0.0f;
        PushPosY = 0.0f;

        // 밀려나는 것 계산.(데미지가 음수이면 힐이다.)
        if (p_Damage->DamageValue >= 0 && true == IsPush)
        {
            AttackPushPos(_PosX, _PosY, p_Target->_PosX, p_Target->_PosY, &PushPosX, &PushPosY);

            // 만약 밀려나면 Stun 시간 동안 움직임과 공격 모두 중단.(스턴 중에도 공격받는다.)
            if (PushPosX != 0.0f && PushPosY != 0.0f)
            {
                do
                {
                    if (std::isnan(PushPosX) || std::isnan(PushPosY))
                    {
                        SYSLOG(L"AttackProc", LOG_ERROR, L"Push NAN Error # [PosX:%f][PosY:%f][TargetX:%f][TargetY:%f]", _PosX, _PosY, p_Target->_PosX, p_Target->_PosY);
                        PushPosX = 0.0f;
                        PushPosY = 0.0f;
                        break;
                    }

                    if (abs(p_Target->_PosX - PushPosX) > 1.0f || abs(p_Target->_PosY - PushPosY) > 1.0f)
                    {
                        SYSLOG(L"AttackProc", LOG_ERROR, L"Wrong Push Pos # [PosX:%f][PosY:%f][TargetX:%f][TargetY:%f][PushX:%f][PushY:%f]", _PosX, _PosY, p_Target->_PosX, p_Target->_PosY, PushPosX, PushPosY);
                        PushPosX = 0.0f;
                        PushPosY = 0.0f;
                        break;
                    }

                    // 기존 타일 좌표를 저장한다.
                    TargetOldTileX = p_Target->_TileX;
                    TargetOldTileY = p_Target->_TileY;

                    // 피공격자 좌표 갱신.
                    if (false == p_Target->SetPosition(PushPosX, PushPosY))
                    {
                        PushPosX = 0.0f;
                        PushPosY = 0.0f;
                        break;
                    }

                    // 기존 타일 좌표와 다르다면 패스 리셋
                    if (TargetOldTileX != p_Target->_TileX || TargetOldTileY != p_Target->_TileY)
                        p_Target->MoveReset();
                } while (0);

                //if (PushPosX != 0.0f && PushPosY != 0.0f)
                //{
                //    p_Target->MoveReset();
                //}
            }

            // 이건 push 여부에 관계없이 데미지를 받았다면 적용한다.
            p_Target->SetStunDelay(((ULONGLONG)dfDAMAGE_STUN_TIME));

        }// end push

        p_Damage->PushPosX = PushPosX;
        p_Damage->PushPosY = PushPosY;
    }
}

bool Player::CheckMoveDelay(void)
{
    // 이동 딜레이 처리.
    if (_MoveDelay != 0)
    {
        if (_MoveDelay > _UpdateTime)
            return false;
        if (_NextMoveTime < _UpdateTime)
            _NextMoveTime = _UpdateTime;
        _MoveDelay = 0;
    }
    return true;
}
bool Player::CheckAttackDelay(void)
{
    // 공격 딜레이 처리.
    if (_AttackDelay != 0)
    {
        if (_AttackDelay > _UpdateTime)
            return false;
        if (_NextAttackTime < _UpdateTime)
            _NextAttackTime = _UpdateTime;
        _AttackDelay = 0;
    }

    // 타겟이 적절한지 체크.
    if (OBJECT_ID_DEFAULT == _TargetID)
    {
        return false;
    }
    if (nullptr == _p_Target)
    {
        CrashDump::Crash();
        return false;
    }
    if (_p_Target->_ObjectID != _TargetID)
    {
        AttackReset();
        _DestTileX = TILE_DEFAULT_X;
        _DestTileY = TILE_DEFAULT_Y;
        return false;
    }
    if (true == _p_Target->_IsFirstJoin)
    {
        CrashDump::Crash();
        AttackReset();
        _DestTileX = TILE_DEFAULT_X;
        _DestTileY = TILE_DEFAULT_Y;
        return false;
    }
    if (_p_Target->_HP < 1)
    {
        AttackReset();
        _DestTileX = TILE_DEFAULT_X;
        _DestTileY = TILE_DEFAULT_Y;
        return false;
    }
    return true;
}
void Player::SetStunDelay(ULONGLONG Delay)
{
    SetMoveDelay(Delay);
    SetAttackDelay(Delay);
}
void Player::SetMoveDelay(ULONGLONG Delay)
{
    _MoveDelay = _TimeManager->GetTickTime64() + Delay;
    MoveReset();
}
void Player::SetAttackDelay(ULONGLONG Delay)
{
    _AttackDelay = _TimeManager->GetTickTime64() + Delay;
}

bool Player::SetPosition(float PosX, float PosY)
{
    int TileX;
    int TileY;

    TileX = POS_to_TILE_X(PosX);
    TileY = POS_to_TILE_Y(PosY);

    if (false == TileUpdate(TileX, TileY))
        return false;

    _PosX = PosX;
    _PosY = PosY;
    return true;
}
bool Player::TileUpdate(int NewTileX, int NewTileY)
{
    if (false == _p_PathFinder->CheckMapAttribute(NewTileX, NewTileY))
        return false;

    if (true == AddTileObject(this, NewTileX, NewTileY, OBJECT_TYPE_PLAYER))
    {
        if (false == RemoveTileObject(this, _TileX, _TileY, OBJECT_TYPE_PLAYER))
        {
            CrashDump::Crash();
            return false;
        }
        _TileX = NewTileX;
        _TileY = NewTileY;
        return true;
    }
    SYSLOG(L"TileUpdate", LOG_ERROR, L"AddTile ERROR # [AccountNo:%lld][TileX:%d][TileY:%d][NewTileX:%d][NewTileY:%d][TileCnt:%d]", _AccountNo, _TileX, _TileY, NewTileX, NewTileY, _pp_MapTile[NewTileY][NewTileX].ObjectNum);
    return false;
}

bool Player::RemoveTileObject(Player *p_Object, int TileX, int TileY, int ObjectType)
{
    if (TileX < 0 || TileX >= TILE_MAX_X || TileY < 0 || TileY >= TILE_MAX_Y)
        return false;

    if (false == _p_PathFinder->CheckMapAttribute(TileX, TileY))
        return false;

    st_MAP_TILE *p_MapTile;
    int Cnt;

    p_MapTile = &_pp_MapTile[TileY][TileX];
    for (Cnt = 0; Cnt < MAP_TILE_OBJECT_MAX; ++Cnt)
    {
        if (p_MapTile->Object[Cnt].p_Object != nullptr)
        {
            if (p_Object->_ObjectID == p_MapTile->Object[Cnt].p_Object->_ObjectID)
            {
                if (p_MapTile->Object[Cnt].ObjectType != ObjectType)
                {
                    CrashDump::Crash();
                }
                p_MapTile->Object[Cnt].ObjectType = -1;                 // define으로 뺄 것.
                p_MapTile->Object[Cnt].p_Object = nullptr;
                p_MapTile->ObjectNum--;
                return true;
            }
        }
    }
    return false;
}
bool Player::AddTileObject(Player *p_Object, int TileX, int TileY, int ObjectType)
{
    if (TileX < 0 || TileX >= TILE_MAX_X || TileY < 0 || TileY >= TILE_MAX_Y)
        return false;

    if (false == _p_PathFinder->CheckMapAttribute(TileX, TileY))
        return false;

    st_MAP_TILE *p_MapTile;
    int Cnt;

    p_MapTile = &_pp_MapTile[TileY][TileX];
    for (Cnt = 0; Cnt < MAP_TILE_OBJECT_MAX; ++Cnt)
    {
        if (nullptr == p_MapTile->Object[Cnt].p_Object)
        {
            p_MapTile->Object[Cnt].ObjectType = ObjectType;
            p_MapTile->Object[Cnt].p_Object = p_Object;
            p_MapTile->ObjectNum++;
            return true;
        }
    }
    return false;
}

bool Player::SectorUpdate(void)
{
    short OldSectorX;
    short OldSectorY;

    OldSectorX = _SectorX;
    OldSectorY = _SectorY;

    _SectorX = TILE_to_SECTOR_X(_TileX);
    _SectorY = TILE_to_SECTOR_Y(_TileY);

    if (OldSectorX == _SectorX && OldSectorY == _SectorY)
        return false;

    LeaveSector(OldSectorX, OldSectorY);
    EnterSector(_SectorX, _SectorY);

    _OldSectorX = OldSectorX;
    _OldSectorY = OldSectorY;
    return true;
}
void Player::SendPacket_SectorUpdate(void)
{
    int Cnt;
    st_SECTOR_AROUND RemoveSectorAround;
    st_SECTOR_AROUND AddSectorAround;

    std::list<Player *> *p_SectorPlayerList;
    std::list<Player *>::iterator SectorPlayerListItr;

    Packet *p_MakePacket;
    Player *p_Other;

    //ProfileBegin(L"SectorUpdateSnd1");
    GetSectorUpdateAround(_OldSectorX, _OldSectorY, _SectorX, _SectorY, &RemoveSectorAround, &AddSectorAround);
    //ProfileEnd(L"SectorUpdateSnd1");

    // 떠나온 섹터의 다른 플레이어에게 삭제 패킷 전송.
    //ProfileBegin(L"SectorUpdateSnd2");
    p_MakePacket = PacketMake_RemoveObject(_ObjectID);
    for (Cnt = 0; Cnt < RemoveSectorAround.Count; ++Cnt)
    {
        SendPacket_SectorOne(p_MakePacket, RemoveSectorAround.Around[Cnt].SectorX, RemoveSectorAround.Around[Cnt].SectorY, this);
    }
    p_MakePacket->Free();
    //ProfileEnd(L"SectorUpdateSnd2");

    // 떠나온 섹터의 다른 플레이어를 삭제하도록 전송.
    //ProfileBegin(L"SectorUpdateSnd3");
    for (Cnt = 0; Cnt < RemoveSectorAround.Count; ++Cnt)
    {
        p_SectorPlayerList = &(_pp_Sector[RemoveSectorAround.Around[Cnt].SectorY][RemoveSectorAround.Around[Cnt].SectorX].PlayerList);
        for (SectorPlayerListItr = p_SectorPlayerList->begin(); SectorPlayerListItr != p_SectorPlayerList->end(); ++SectorPlayerListItr)
        {
            p_MakePacket = PacketMake_RemoveObject((*SectorPlayerListItr)->_ObjectID);
            SendPacket(p_MakePacket);
            p_MakePacket->Free();
        }
    }
    //ProfileEnd(L"SectorUpdateSnd3");

    // 새로운 섹터에 생성 패킷 뿌리기.
    //ProfileBegin(L"SectorUpdateSnd4");
    p_MakePacket = PacketMake_CreateOtherCharacter(this, 0);
    for (Cnt = 0; Cnt < AddSectorAround.Count; ++Cnt)
    {
        SendPacket_SectorOne(p_MakePacket, AddSectorAround.Around[Cnt].SectorX, AddSectorAround.Around[Cnt].SectorY, this);
    }
    p_MakePacket->Free();
    //ProfileEnd(L"SectorUpdateSnd4");

    // 이동중이라는 패킷 보내기
    //ProfileBegin(L"SectorUpdateSnd5");
    if (0 == _MoveDelay && _PathSize != 0 && _PathIndex < _PathSize)
    {
        p_MakePacket = PacketMake_MoveCharacter(this);
        for (Cnt = 0; Cnt < AddSectorAround.Count; ++Cnt)
        {
            SendPacket_SectorOne(p_MakePacket, AddSectorAround.Around[Cnt].SectorX, AddSectorAround.Around[Cnt].SectorY, this);
        }
        p_MakePacket->Free();
    }
    //ProfileEnd(L"SectorUpdateSnd5");

    // 새로운 섹터의 다른 플레이어 정보 주기.
    //ProfileBegin(L"SectorUpdateSnd6");
    for (Cnt = 0; Cnt < AddSectorAround.Count; ++Cnt)
    {
        p_SectorPlayerList = &(_pp_Sector[AddSectorAround.Around[Cnt].SectorY][AddSectorAround.Around[Cnt].SectorX].PlayerList);
        for (SectorPlayerListItr = p_SectorPlayerList->begin(); SectorPlayerListItr != p_SectorPlayerList->end(); ++SectorPlayerListItr)
        {
            p_Other = (*SectorPlayerListItr);
            if (_ObjectID == p_Other->_ObjectID)
                continue;

            p_MakePacket = PacketMake_CreateOtherCharacter(p_Other, 0);
            SendPacket(p_MakePacket);
            p_MakePacket->Free();

            // 만약 이동중이라면 이동패킷을 만들어서 보낸다.
            if (p_Other->_HP > 0 && 0 == p_Other->_MoveDelay && p_Other->_PathSize != 0 && p_Other->_PathIndex < p_Other->_PathSize)
            {
                p_MakePacket = PacketMake_MoveCharacter(p_Other);
                SendPacket(p_MakePacket);
                p_MakePacket->Free();
            }
        }
    }
    //ProfileEnd(L"SectorUpdateSnd6");
}

void Player::SendPacket_SectorUpdateNewPlayer(void)
{
    st_SECTOR_AROUND AddSectorAround;
    Player *p_Other;

    std::list<Player *> *p_SectorPlayerList;
    std::list<Player *>::iterator SectorPlayerListItr;

    Packet *p_MakePacket;
    int Cnt;

    GetSectorAround(_SectorX, _SectorY, &AddSectorAround);

    // 새로운 섹터에 생성 패킷 뿌리기.
    p_MakePacket = PacketMake_CreateOtherCharacter(this, 1);
    for (Cnt = 0; Cnt < AddSectorAround.Count; ++Cnt)
    {
        SendPacket_SectorOne(p_MakePacket, AddSectorAround.Around[Cnt].SectorX, AddSectorAround.Around[Cnt].SectorY, this);
    }
    p_MakePacket->Free();

    // 새로운 섹터의 다른 플레이어 정보 주기.
    for (Cnt = 0; Cnt < AddSectorAround.Count; ++Cnt)
    {
        p_SectorPlayerList = &(_pp_Sector[AddSectorAround.Around[Cnt].SectorY][AddSectorAround.Around[Cnt].SectorX].PlayerList);
        for (SectorPlayerListItr = p_SectorPlayerList->begin(); SectorPlayerListItr != p_SectorPlayerList->end(); ++SectorPlayerListItr)
        {
            p_Other = (*SectorPlayerListItr);
            if (_ObjectID == p_Other->_ObjectID)
                continue;

            p_MakePacket = PacketMake_CreateOtherCharacter(p_Other, 0);
            SendPacket(p_MakePacket);
            p_MakePacket->Free();

            // 만약 이동중이라면 이동패킷을 만들어서 보낸다.
            if (p_Other->_HP > 0 && 0 == p_Other->_MoveDelay && p_Other->_PathSize != 0 && p_Other->_PathIndex < p_Other->_PathSize)
            {
                p_MakePacket = PacketMake_MoveCharacter(p_Other);
                SendPacket(p_MakePacket);
                p_MakePacket->Free();
            }
        }
    }
}

bool Player::LeaveSector(short SectorX, short SectorY)
{
    if (SECTOR_DEFAULT_X == SectorX || SECTOR_DEFAULT_Y == SectorY)
        return false;
    if (OBJECT_ID_DEFAULT == _ObjectID)
    {
        CrashDump::Crash();
        return false;
    }

    if (SectorX < 0 || SectorX > SECTOR_MAX_X || SectorY < 0 || SectorY > SECTOR_MAX_Y)
    {
        // 바깥에서 체크하기는 하지만 일단 여기서 걸리면 뻑낸다.
        CrashDump::Crash();
        return false;
    }

    std::list<Player *> *p_SectorPlayerList;
    std::list<Player *>::iterator SectorPlayerListItr;
    p_SectorPlayerList = &(_pp_Sector[SectorY][SectorX].PlayerList);
    for (SectorPlayerListItr = p_SectorPlayerList->begin();
        SectorPlayerListItr != p_SectorPlayerList->end(); ++SectorPlayerListItr)
    {
        if (_ObjectID == (*SectorPlayerListItr)->_ObjectID)
        {
            p_SectorPlayerList->erase(SectorPlayerListItr);

            // debug
            _p_GameServer->_Monitor_Sector_Count--;

            return true;
        }
    }
    return false;
}
bool Player::EnterSector(short SectorX, short SectorY)
{
    if (SectorX < 0 || SectorX > SECTOR_MAX_X || SectorY < 0 || SectorY > SECTOR_MAX_Y)
    {
        // 바깥에서 체크하기는 하지만 일단 여기서 걸리면 뻑낸다.
        CrashDump::Crash();
        return false;
    }
    if (OBJECT_ID_DEFAULT == _ObjectID)
    {
        CrashDump::Crash();
        return false;
    }

    _pp_Sector[SectorY][SectorX].PlayerList.push_back(this);

    // debug
    _p_GameServer->_Monitor_Sector_Count++;

    return true;
}

void Player::GetSectorAround(short SectorX, short SectorY, st_SECTOR_AROUND *p_SectorAround)
{
    int CntX;
    int CntY;

    SectorX--;
    SectorY--;

    p_SectorAround->Count = 0;
    for (CntY = 0; CntY < 3; ++CntY)
    {
        if (SectorY + CntY < 0 || SectorY + CntY >= SECTOR_MAX_Y)
            continue;

        for (CntX = 0; CntX < 3; ++CntX)
        {
            if (SectorX + CntX < 0 || SectorX + CntX >= SECTOR_MAX_X)
                continue;

            p_SectorAround->Around[p_SectorAround->Count].SectorX = SectorX + CntX;
            p_SectorAround->Around[p_SectorAround->Count].SectorY = SectorY + CntY;
            p_SectorAround->Count++;
        }
    }
}
void Player::GetSectorUpdateAround(short OldSectorX, short OldSectorY, short NewSectorX, short NewSectorY, st_SECTOR_AROUND *p_RemoveSectorAround, st_SECTOR_AROUND *p_AddSectorAround)
{
    //// 일단 아래 로직은 보류한다.
    //GetNewSectorByTheDirection(NewSectorX, NewSectorY, OldSectorX, OldSectorY, p_RemoveSectorAround);
    //GetNewSectorByTheDirection(OldSectorX, OldSectorY, NewSectorX, NewSectorY, p_AddSectorAround);

    bool FindSector;
    int CntOld;
    int CntCur;
    st_SECTOR_AROUND OldSectorAround;
    st_SECTOR_AROUND NewSectorAround;

    OldSectorAround.Count = 0;
    NewSectorAround.Count = 0;

    GetSectorAround(OldSectorX, OldSectorY, &OldSectorAround);
    GetSectorAround(NewSectorX, NewSectorY, &NewSectorAround);

    p_RemoveSectorAround->Count = 0;
    p_AddSectorAround->Count = 0;
    for (CntOld = 0; CntOld < OldSectorAround.Count; ++CntOld)
    {
        FindSector = false;
        for (CntCur = 0; CntCur < NewSectorAround.Count; ++CntCur)
        {
            if (OldSectorAround.Around[CntOld].SectorX == NewSectorAround.Around[CntCur].SectorX &&
                OldSectorAround.Around[CntOld].SectorY == NewSectorAround.Around[CntCur].SectorY)
            {
                FindSector = true;
                break;
            }
        }
        if (false == FindSector)
        {
            p_RemoveSectorAround->Around[p_RemoveSectorAround->Count] = OldSectorAround.Around[CntOld];
            p_RemoveSectorAround->Count++;
        }
    }

    for (CntCur = 0; CntCur < NewSectorAround.Count; ++CntCur)
    {
        FindSector = false;
        for (CntOld = 0; CntOld < OldSectorAround.Count; ++CntOld)
        {
            if (OldSectorAround.Around[CntOld].SectorX == NewSectorAround.Around[CntCur].SectorX &&
                OldSectorAround.Around[CntOld].SectorY == NewSectorAround.Around[CntCur].SectorY)
            {
                FindSector = true;
                break;
            }
        }
        if (false == FindSector)
        {
            p_AddSectorAround->Around[p_AddSectorAround->Count] = NewSectorAround.Around[CntCur];
            p_AddSectorAround->Count++;
        }
    }
}
// Old -> New로 이동할 때 Old과 겹치는 부분을 제외한 New의 섹터를 구하는 함수.
void Player::GetNewSectorByTheDirection(short OldSectorX, short OldSectorY, short NewSectorX, short NewSectorY, st_SECTOR_AROUND *p_NewSector)
{
    short NewSectorDirX;
    short NewSectorDirY;
    st_SECTOR_POS SectorHorz;
    st_SECTOR_POS SectorVert;
    st_SECTOR_POS NewSectorStart;

    p_NewSector->Count = 0;

    // SectorAround의 경계선을 잡는다.
    NewSectorDirX = NewSectorX - OldSectorX;
    NewSectorDirY = NewSectorY - OldSectorY;

    if (0 == NewSectorDirX && 0 == NewSectorDirY)
    {
        return;
    }

    NewSectorStart.SectorX = -1;
    NewSectorStart.SectorY = -1;

    if (NewSectorDirY != 0)
    {
        // Y축으로만 움직였음.
        SectorHorz.SectorX = NewSectorX;

        SectorHorz.SectorY = NewSectorY + NewSectorDirY;
        if (SectorHorz.SectorY < 0)
        {
            // 위쪽
            SectorHorz.SectorY = 0;
            if (SectorHorz.SectorY == NewSectorY)
                NewSectorDirY = 0;
            //return;
        }
        else if (SectorHorz.SectorY >= SECTOR_MAX_Y)
        {
            // 아래쪽
            SectorHorz.SectorY = SECTOR_MAX_Y - 1;
            if (SectorHorz.SectorY == NewSectorY)
                NewSectorDirY = 0;
            //return;
        }
    }

    if (NewSectorDirX != 0)
    {
        // X축으로만 움직였음.
        SectorVert.SectorY = NewSectorY;

        SectorVert.SectorX = NewSectorX + NewSectorDirX;
        if (SectorVert.SectorX < 0)
        {
            // 왼쪽
            SectorVert.SectorX = 0;
            if (SectorVert.SectorX == NewSectorX)
                NewSectorDirX = 0;
            //return;
        }
        else if (SectorVert.SectorX >= SECTOR_MAX_X)
        {
            // 오른쪽
            SectorVert.SectorX = SECTOR_MAX_X - 1;
            if (SectorVert.SectorX == NewSectorX)
                NewSectorDirX = 0;
            //return;
        }
    }

    if (NewSectorDirX != 0 && NewSectorDirY != 0)
    {
        NewSectorStart.SectorY = NewSectorY + NewSectorDirY;
        if (NewSectorStart.SectorY < 0)
        {
            // 위쪽
            NewSectorStart.SectorY = 0;
            if (NewSectorStart.SectorY == NewSectorY)
                NewSectorDirY = 0;
            //return;
        }
        else if (NewSectorStart.SectorY >= SECTOR_MAX_Y)
        {
            // 아래쪽
            NewSectorStart.SectorY = SECTOR_MAX_Y - 1;
            if (NewSectorStart.SectorY == NewSectorY)
                NewSectorDirY = 0;
            //return;
        }

        NewSectorStart.SectorX = NewSectorX + NewSectorDirX;
        if (NewSectorStart.SectorX < 0)
        {
            // 왼쪽
            NewSectorStart.SectorX = 0;
            if (NewSectorStart.SectorX == NewSectorX)
                NewSectorDirX = 0;
            //return;
        }
        else if (NewSectorStart.SectorX >= SECTOR_MAX_X)
        {
            // 오른쪽
            NewSectorStart.SectorX = SECTOR_MAX_X - 1;
            if (NewSectorStart.SectorX == NewSectorX)
                NewSectorDirX = 0;
            //return;
        }
    }

    if (NewSectorDirX != 0)
    {
        p_NewSector->Around[p_NewSector->Count].SectorY = SectorVert.SectorY - 1;
        if (p_NewSector->Around[p_NewSector->Count].SectorY >= 0)
        {
            p_NewSector->Around[p_NewSector->Count].SectorX = SectorVert.SectorX;
            if (p_NewSector->Around[p_NewSector->Count].SectorX != NewSectorStart.SectorX ||
                p_NewSector->Around[p_NewSector->Count].SectorY != NewSectorStart.SectorY)
                p_NewSector->Count++;
        }

        p_NewSector->Around[p_NewSector->Count].SectorX = SectorVert.SectorX;
        p_NewSector->Around[p_NewSector->Count].SectorY = SectorVert.SectorY;
        p_NewSector->Count++;

        p_NewSector->Around[p_NewSector->Count].SectorY = SectorVert.SectorY + 1;
        if (p_NewSector->Around[p_NewSector->Count].SectorY < SECTOR_MAX_Y)
        {
            p_NewSector->Around[p_NewSector->Count].SectorX = SectorVert.SectorX;
            if (p_NewSector->Around[p_NewSector->Count].SectorX != NewSectorStart.SectorX ||
                p_NewSector->Around[p_NewSector->Count].SectorY != NewSectorStart.SectorY)
                p_NewSector->Count++;
        }
        if (0 == NewSectorDirY)
            return;
    }

    if (NewSectorDirY != 0)
    {
        p_NewSector->Around[p_NewSector->Count].SectorX = SectorHorz.SectorX - 1;
        if (p_NewSector->Around[p_NewSector->Count].SectorX >= 0)
        {
            p_NewSector->Around[p_NewSector->Count].SectorY = SectorHorz.SectorY;
            if (p_NewSector->Around[p_NewSector->Count].SectorX != NewSectorStart.SectorX ||
                p_NewSector->Around[p_NewSector->Count].SectorY != NewSectorStart.SectorY)
                p_NewSector->Count++;
        }

        p_NewSector->Around[p_NewSector->Count].SectorX = SectorHorz.SectorX;
        p_NewSector->Around[p_NewSector->Count].SectorY = SectorHorz.SectorY;
        p_NewSector->Count++;

        p_NewSector->Around[p_NewSector->Count].SectorX = SectorHorz.SectorX + 1;
        if (p_NewSector->Around[p_NewSector->Count].SectorX < SECTOR_MAX_X)
        {
            p_NewSector->Around[p_NewSector->Count].SectorY = SectorHorz.SectorY;
            if (p_NewSector->Around[p_NewSector->Count].SectorX != NewSectorStart.SectorX ||
                p_NewSector->Around[p_NewSector->Count].SectorY != NewSectorStart.SectorY)
                p_NewSector->Count++;
        }
        if (0 == NewSectorDirX)
            return;
    }

    if (NewSectorDirX != 0 && NewSectorDirY != 0)
    {
        p_NewSector->Around[p_NewSector->Count].SectorX = NewSectorStart.SectorX;
        p_NewSector->Around[p_NewSector->Count].SectorY = NewSectorStart.SectorY;
        p_NewSector->Count++;
    }
}

Player *Player::FindPlayer(OBJECT_ID ObjectID)
{
    __int64 PlayerIndex = OBJECT_ID_INDEX(ObjectID);
    if (0 > PlayerIndex || PlayerIndex >= _MaxSession)
    {
        // Session Index가 잘못되었으므로 정지한다.             // 이건 바깥에서 체크한다.
        CrashDump::Crash();
        return nullptr;
    }
    
    if (_p_PlayerArray[PlayerIndex]._ObjectID != ObjectID)        // 이건 바깥에서 체크한다.
        return nullptr;
    return &_p_PlayerArray[PlayerIndex];
}

void Player::SendPacket_SectorOne(Packet *p_Packet, short SectorX, short SectorY, Player *p_ExceptPlayer)
{
    if (SectorX < 0 || SectorX > SECTOR_MAX_X || SectorY < 0 || SectorY > SECTOR_MAX_Y)
    {
        // 바깥에서 체크하기는 하지만 일단 여기서 걸리면 뻑낸다.
        CrashDump::Crash();
        return;
    }

    std::list<Player *> *p_SectorPlayerList;
    std::list<Player *>::iterator SectorPlayerListItr;

    if (0 == _pp_Sector[SectorY][SectorX].PlayerList.size())
        return;
    
    p_SectorPlayerList = &(_pp_Sector[SectorY][SectorX].PlayerList);

    SectorPlayerListItr = p_SectorPlayerList->begin();
    while (SectorPlayerListItr != p_SectorPlayerList->end())
    {
        //if (nullptr == p_ExceptPlayer || (*SectorPlayerListItr)->_ObjectID != p_ExceptPlayer->_ObjectID)
        if ((*SectorPlayerListItr) != p_ExceptPlayer)
        {
            if (OBJECT_ID_DEFAULT == (*SectorPlayerListItr)->_ObjectID)
            {
                CrashDump::Crash();
                return;
            }
            (*SectorPlayerListItr)->SendPacket(p_Packet);
        }
        SectorPlayerListItr++;
    }
}
void Player::SendPacket_SectorAround(Packet *p_Packet, Player *p_ExceptPlayer)
{
    int Cnt;
    st_SECTOR_AROUND SectorAround;

    SectorAround.Count = 0;
    GetSectorAround(_SectorX, _SectorY, &SectorAround);

    for (Cnt = 0; Cnt < SectorAround.Count; ++Cnt)
        SendPacket_SectorOne(p_Packet, SectorAround.Around[Cnt].SectorX, SectorAround.Around[Cnt].SectorY, p_ExceptPlayer);
}
void Player::SendPacket_SectorAround(Packet *p_Packet, short SectorX, short SectorY, Player *p_ExceptPlayer)
{
    int Cnt;
    st_SECTOR_AROUND SectorAround;

    SectorAround.Count = 0;
    GetSectorAround(SectorX, SectorY, &SectorAround);

    for (Cnt = 0; Cnt < SectorAround.Count; ++Cnt)
        SendPacket_SectorOne(p_Packet, SectorAround.Around[Cnt].SectorX, SectorAround.Around[Cnt].SectorY, p_ExceptPlayer);
}

bool Player::DBAccountWriteStatusLogout(void)
{
    st_DBWRITER_MSG *p_Msg;
    stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *p_WriteStatusLogout;

    p_Msg = _p_GameServer->_DBWriterMessagePool->Alloc();
    if (nullptr == p_Msg)
    {
        CrashDump::Crash();
        return false;
    }
    p_Msg->Type_DB = dfDBWRITER_TYPE_ACCOUNT;
    p_Msg->Type_Message = enDB_ACCOUNT_WRITE_STATUS_LOGOUT;

    //p_WriteStatusLogout = new(p_Msg->Message) stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in;
    p_WriteStatusLogout = (stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *)(p_Msg->Message);
    p_WriteStatusLogout->AccountNo = _AccountNo;

    if (false == _p_GameServer->_DBWriterMessageQueue->Enqueue(p_Msg))
    {
        CrashDump::Crash();
    }
    SetEvent(_p_GameServer->_DBWriterEvent);
    return true;
}

bool Player::DBGameWriteLogJoin(void)
{
    st_DBWRITER_MSG *p_Msg;
    stDB_GAME_WRITE_LOG_JOIN_in *p_WriteLogJoin;

    p_Msg = _p_GameServer->_DBWriterMessagePool->Alloc();
    if (nullptr == p_Msg)
    {
        CrashDump::Crash();
        return false;
    }
    p_Msg->Type_DB = dfDBWRITER_TYPE_GAME;
    p_Msg->Type_Message = enDB_GAME_WRITE_LOG_JOIN;

    //p_WriteLogJoin = new(p_Msg->Message) stDB_GAME_WRITE_LOG_JOIN_in;
    p_WriteLogJoin = (stDB_GAME_WRITE_LOG_JOIN_in *)(p_Msg->Message);
    p_WriteLogJoin->AccountNo = _AccountNo;
    p_WriteLogJoin->TileX = _TileX;
    p_WriteLogJoin->TileY = _TileY;
    p_WriteLogJoin->CharacterType = _CharacterType;

    if (false == _p_GameServer->_DBWriterMessageQueue->Enqueue(p_Msg))
    {
        CrashDump::Crash();
    }
    SetEvent(_p_GameServer->_DBWriterEvent);
    return true;
}
bool Player::DBGameWriteLogLeave(void)
{
    st_DBWRITER_MSG *p_Msg;
    stDB_GAME_WRITE_LOG_LEAVE_in *p_WriteLogLeave;

    p_Msg = _p_GameServer->_DBWriterMessagePool->Alloc();
    if (nullptr == p_Msg)
    {
        CrashDump::Crash();
        return false;
    }
    p_Msg->Type_DB = dfDBWRITER_TYPE_GAME;
    p_Msg->Type_Message = enDB_GAME_WRITE_LOG_LEAVE;

    //p_WriteLogLeave = new(p_Msg->Message) stDB_GAME_WRITE_LOG_LEAVE_in;
    p_WriteLogLeave = (stDB_GAME_WRITE_LOG_LEAVE_in *)(p_Msg->Message);
    p_WriteLogLeave->AccountNo = _AccountNo;
    p_WriteLogLeave->TileX = _TileX;
    p_WriteLogLeave->TileY = _TileY;
    p_WriteLogLeave->KillCount = _KillCount;
    p_WriteLogLeave->GuestKillCount = _GuestKillCount;

    if (false == _p_GameServer->_DBWriterMessageQueue->Enqueue(p_Msg))
    {
        CrashDump::Crash();
    }
    SetEvent(_p_GameServer->_DBWriterEvent);
    return true;
}
bool Player::DBGameWritePlayerDie(__int64 AttackerAccountNo)
{
    st_DBWRITER_MSG *p_Msg;
    stDB_GAME_WRITE_PLAYER_DIE_in *p_WritePlayerDie;

    p_Msg = _p_GameServer->_DBWriterMessagePool->Alloc();
    if (nullptr == p_Msg)
    {
        CrashDump::Crash();
        return false;
    }
    p_Msg->Type_DB = dfDBWRITER_TYPE_GAME;
    p_Msg->Type_Message = enDB_GAME_WRITE_PLAYER_DIE;

    //p_WritePlayerDie = new(p_Msg->Message) stDB_GAME_WRITE_PLAYER_DIE_in;
    p_WritePlayerDie = (stDB_GAME_WRITE_PLAYER_DIE_in *)(p_Msg->Message);
    p_WritePlayerDie->AccountNo = _AccountNo;
    p_WritePlayerDie->AttackerAccountNo = AttackerAccountNo;
    p_WritePlayerDie->DiePosX = _TileX;
    p_WritePlayerDie->DiePosY = _TileY;

    if (false == _p_GameServer->_DBWriterMessageQueue->Enqueue(p_Msg))
    {
        CrashDump::Crash();
    }
    SetEvent(_p_GameServer->_DBWriterEvent);
    return true;
}
bool Player::DBGameWritePlayerKill(__int64 TargetAccountNo, int TargetTileX, int TargetTileY, bool IsGuestTarget)
{
    st_DBWRITER_MSG *p_Msg;
    stDB_GAME_WRITE_PLAYER_KILL_in *p_WritePlayerKill;

    p_Msg = _p_GameServer->_DBWriterMessagePool->Alloc();
    if (nullptr == p_Msg)
    {
        CrashDump::Crash();
        return false;
    }
    p_Msg->Type_DB = dfDBWRITER_TYPE_GAME;
    p_Msg->Type_Message = enDB_GAME_WRITE_PLAYER_KILL;

    //p_WritePlayerKill = new(p_Msg->Message) stDB_GAME_WRITE_PLAYER_KILL_in;
    p_WritePlayerKill = (stDB_GAME_WRITE_PLAYER_KILL_in *)(p_Msg->Message);
    p_WritePlayerKill->AccountNo = _AccountNo;
    p_WritePlayerKill->TargetAccountNo = TargetAccountNo;
    p_WritePlayerKill->KillPosX = TargetTileX;
    p_WritePlayerKill->KillPosY = TargetTileY;
    p_WritePlayerKill->IsGuestTarget = IsGuestTarget;

    if (false == _p_GameServer->_DBWriterMessageQueue->Enqueue(p_Msg))
    {
        CrashDump::Crash();
    }
    SetEvent(_p_GameServer->_DBWriterEvent);
    return true;
}

bool Player::DBGameWriteDieLog(__int64 AttackerAccountNo)
{
    st_DBWRITER_MSG *p_Msg;
    stDB_GAME_WRITE_PLAYER_DIE_in *p_WritePlayerDie;

    p_Msg = _p_GameServer->_DBWriterMessagePool->Alloc();
    if (nullptr == p_Msg)
    {
        CrashDump::Crash();
        return false;
    }
    p_Msg->Type_DB = dfDBWRITER_TYPE_GAME;
    p_Msg->Type_Message = enDB_GAME_WRITE_LOG_PLAYER_DIE;

    //p_WritePlayerDie = new(p_Msg->Message) stDB_GAME_WRITE_PLAYER_DIE_in;
    p_WritePlayerDie = (stDB_GAME_WRITE_PLAYER_DIE_in *)(p_Msg->Message);
    p_WritePlayerDie->AccountNo = _AccountNo;
    p_WritePlayerDie->AttackerAccountNo = AttackerAccountNo;
    p_WritePlayerDie->DiePosX = _TileX;
    p_WritePlayerDie->DiePosY = _TileY;

    if (false == _p_GameServer->_DBWriterMessageQueue->Enqueue(p_Msg))
    {
        CrashDump::Crash();
    }
    SetEvent(_p_GameServer->_DBWriterEvent);
    return true;
}
bool Player::DBGameWriteKillLog(__int64 TargetAccountNo, int TargetTileX, int TargetTileY, bool IsGuestTarget)
{
    st_DBWRITER_MSG *p_Msg;
    stDB_GAME_WRITE_PLAYER_KILL_in *p_WritePlayerKill;

    p_Msg = _p_GameServer->_DBWriterMessagePool->Alloc();
    if (nullptr == p_Msg)
    {
        CrashDump::Crash();
        return false;
    }
    p_Msg->Type_DB = dfDBWRITER_TYPE_GAME;
    p_Msg->Type_Message = enDB_GAME_WRITE_LOG_PLAYER_KILL;

    //p_WritePlayerKill = new(p_Msg->Message) stDB_GAME_WRITE_PLAYER_KILL_in;
    p_WritePlayerKill = (stDB_GAME_WRITE_PLAYER_KILL_in *)(p_Msg->Message);
    p_WritePlayerKill->AccountNo = _AccountNo;
    p_WritePlayerKill->TargetAccountNo = TargetAccountNo;
    p_WritePlayerKill->KillPosX = TargetTileX;
    p_WritePlayerKill->KillPosY = TargetTileY;
    p_WritePlayerKill->IsGuestTarget = IsGuestTarget;

    if (false == _p_GameServer->_DBWriterMessageQueue->Enqueue(p_Msg))
    {
        CrashDump::Crash();
    }
    SetEvent(_p_GameServer->_DBWriterEvent);
    return true;
}

void Player::OnAuth_ClientJoin(void)
{

}
void Player::OnAuth_ClientLeave(bool ToGame)
{
    stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in WriteStatusLogoutIn;

    // accountno가 0이 아니고 toGame이 아닐 때 writer에게 logout 메세지를 던진다.
    if (_AccountNo != ACCOUNT_NUM_DEFAULT && false == ToGame)
    {
        WriteStatusLogoutIn.AccountNo = _AccountNo;
        _p_GameServer->_p_DBAccount_Auth->WriteDB(enDB_ACCOUNT_WRITE_STATUS_LOGOUT, &WriteStatusLogoutIn);
    }
}
void Player::OnAuth_Packet(Packet *p_Packet)
{
    bool PacketResult;
    WORD PacketType;

    PacketResult = false;
    *p_Packet >> PacketType;
    switch (PacketType)
    {
    case en_PACKET_CS_GAME_REQ_LOGIN:
        PacketResult = PacketProc_Login(p_Packet);
        break;
    case en_PACKET_CS_GAME_REQ_CHARACTER_SELECT:
        PacketResult = PacketProc_CharacterSelect(p_Packet);
        break;
    case en_PACKET_CS_GAME_REQ_HEARTBEAT:
        PacketResult = true;
        break;
    default:
        //CrashDump::Crash();
        SYSLOG(L"OnAuth_Packet", LOG_ERROR, L"Packet Type Error # [Type:%u]", PacketType);
        return;
    }

    if (false == PacketResult)
    {
        Disconnect(true);
        return;
    }

    _LastRecvPacketTime = TimeManager::GetInstance()->GetTickTime();
    InterlockedIncrement(&_p_GameServer->_Monitor_Counter_PackerProc);
}
void Player::OnAuth_Timeout(void)
{
    SYSLOG(L"OnAuth_Timeout", LOG_ERROR, L"Auth Timeout # [SessionID:%lld][AccountNo:%lld]", _SessionID, _AccountNo);
}

void Player::OnGame_ClientJoin(void)
{
    if (ACCOUNT_NUM_DEFAULT == _AccountNo)
        CrashDump::Crash();

    //SYSLOG(L"JoinPlayer", LOG_DEBUG, L"[AccountNo:%lld][ID:%s][Nick:%s]", _AccountNo, _ID, _Nickname);

    __int64 PlayerIndex;
    Packet *p_MakePacket;

    // ObjectID 생성 및 플레이어 맵 추가.
    PlayerIndex = ClientID_Index(_SessionID);
    _ObjectID = _p_GameServer->GetObjectID(PlayerIndex);

    // DB에 로그를 남긴다.
    DBGameWriteLogJoin();

    // 최초 접속 공격불가 처리(더미 제외)
    _IsFirstJoin = false;
    if (_UserType != USER_TYPE_DUMMY)
    {
        _IsFirstJoin = true;
        _p_GameServer->_Monitor_RealPlayer++;
    }

    // 자신의 캐릭터 생성
    p_MakePacket = PacketMake_CreateMyCharacter();
    if (false == SendPacket(p_MakePacket))
    {
        CrashDump::Crash();
        return;
    }
    p_MakePacket->Free();

    //// BotKill 정보 보내기
    //if (_UserType != USER_TYPE_DUMMY)
    //{
    //    p_MakePacket = PacketMake_BotKill();
    //    SendPacket(p_MakePacket);
    //    p_MakePacket->Free();
    //}

    // 섹터처리
    if (false == EnterSector(_SectorX, _SectorY))
        CrashDump::Crash();
    SendPacket_SectorUpdateNewPlayer();

    // 타일처리
    if (false == AddTileObject(this, _TileX, _TileY, OBJECT_TYPE_PLAYER))
    {
        SYSLOG(L"JoinPlayer", LOG_ERROR, L"Add Tile Error # [AccountNo:%lld][ID:%s][Nick:%s]", _AccountNo, _ID, _Nickname);
        CrashDump::Crash();
    }
}
void Player::OnGame_ClientLeave(void)
{
    //SYSLOG(L"LeavePlayer", LOG_DEBUG, L"[AccountNo:%lld][ID:%s][Nick:%s]", _AccountNo, _ID, _Nickname);

    Packet *p_SectorPacket;
    short LeaveSectorX;
    short LeaveSectorY;

    if (_HP > 0)
    {
        if (false == RemoveTileObject(this, _TileX, _TileY, OBJECT_TYPE_PLAYER))
            CrashDump::Crash();
    }
    else
    {
        if (true == RemoveTileObject(this, _TileX, _TileY, OBJECT_TYPE_PLAYER))
            CrashDump::Crash();
    }

    LeaveSectorX = _SectorX;
    LeaveSectorY = _SectorY;

    // 섹터에서 제거.
    if (false == LeaveSector(_SectorX, _SectorY))
        CrashDump::Crash();

    // 섹터에 알린다.
    p_SectorPacket = PacketMake_RemoveObject(_ObjectID);
    SendPacket_SectorAround(p_SectorPacket, LeaveSectorX, LeaveSectorY, this);
    p_SectorPacket->Free();

    // DB 로그아웃 처리.
    DBAccountWriteStatusLogout();
    DBGameWriteLogLeave();

    if (_UserType != USER_TYPE_DUMMY)
        _p_GameServer->_Monitor_RealPlayer--;

    _ObjectID = OBJECT_ID_DEFAULT;
}
void Player::OnGame_Packet(Packet *p_Packet)
{
    bool PacketResult;
    WORD PacketType;

    PacketResult = false;
    *p_Packet >> PacketType;
    switch (PacketType)
    {
    case en_PACKET_CS_GAME_REQ_MOVE_CHARACTER:
        if (_HP > 0)
            PacketResult = PacketProc_MoveCharacter(p_Packet);
        else
            PacketResult = true;
        break;
    case en_PACKET_CS_GAME_REQ_STOP_CHARACTER:
        if (_HP > 0)
            PacketResult = PacketProc_StopCharacter(p_Packet);
        else
            PacketResult = true;
        break;

    case en_PACKET_CS_GAME_REQ_ATTACK1_TARGET:
        if (_HP > 0)
            PacketResult = PacketProc_Attack1Target(p_Packet);
        else
            PacketResult = true;
        break;
    case en_PACKET_CS_GAME_REQ_ATTACK2_TARGET:
        if (_HP > 0)
            PacketResult = PacketProc_Attack2Target(p_Packet);
        else
            PacketResult = true;
        break;
    case en_PACKET_CS_GAME_REQ_HEARTBEAT:
        PacketResult = true;
        break;
    default:
        //CrashDump::Crash();
        SYSLOG(L"OnGame_Packet", LOG_ERROR, L"Packet Type Error # [Type:%u]", PacketType);
        return;
    }

    if (false == PacketResult)
    {
        Disconnect(true);
        return;
    }

    _LastRecvPacketTime = TimeManager::GetInstance()->GetTickTime();
    InterlockedIncrement(&_p_GameServer->_Monitor_Counter_PackerProc);
}
void Player::OnGame_Timeout(void)
{
    SYSLOG(L"OnGame_Timeout", LOG_ERROR, L"Game Timeout # [SessionID:%lld][AccountNo:%lld]", _SessionID, _AccountNo);
}

void Player::OnGame_ClientRelease(void)
{
    _ObjectID = OBJECT_ID_DEFAULT;

    _LoginFlag = false;
    _Party = PARTY_DEFAULT;

    _UserType = USER_TYPE_DUMMY;
    _AccountNo = ACCOUNT_NUM_DEFAULT;
    wmemset(_ID, L'\0', 256);
    wmemset(_Nickname, L'\0', 256);

    _CharacterType = CHARACTER_TYPE_DEFAULT;

    _UpdateTime = 0;

    _IsFirstJoin = false;

    _IsMove = false;
    _IsAttack = false;
    //_IsFind = false;

    _DestTileX = TILE_DEFAULT_X;
    _DestTileY = TILE_DEFAULT_Y;

    _MoveDelay = 0;
    _AttackDelay = 0;

    _IsActionReset = false;

    _PathSize = 0;
    _PathIndex = 0;

    _RemainMoveTime = 0;
    _NextMoveTime = 0;

    _TileX = TILE_DEFAULT_X;
    _TileY = TILE_DEFAULT_Y;
    _SectorX = SECTOR_DEFAULT_X;
    _SectorY = SECTOR_DEFAULT_Y;
    _OldSectorX = SECTOR_DEFAULT_X;
    _OldSectorY = SECTOR_DEFAULT_Y;

    _p_Target = nullptr;
    _TargetID = OBJECT_ID_DEFAULT;
    _TargetTileX = TILE_DEFAULT_X;
    _TargetTileY = TILE_DEFAULT_Y;
    _AttackType = ATTACK_TYPE_DEFAULT;

    _NextAttackTime = 0;
    _AttackMoveCheckTime = 0;

    _PosX = 0.0f;
    _PosY = 0.0f;
    _Rotation = 0;
    _Cristal = 0;
    _HP = 0;
    _Exp = 0;
    _Level = 0;

    _KillCount = 0;
    _GuestKillCount = 0;
}