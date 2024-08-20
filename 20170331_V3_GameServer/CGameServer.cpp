#include "stdafx.h"
#include "TotalHeader.h"

CGameServer::CGameServer(int MaxSession) : CMMOServerV2(MaxSession)
{
    int Cnt;
    unsigned int ThreadId;
    MMOSession *p_Session;

    _Monitor_SessionMiss = 0;
    _Monitor_SessionNotFound = 0;

    _PlayerArray = new Player[_MaxSession];
    for (Cnt = 0; Cnt < _MaxSession; ++Cnt)
    {
        _PlayerArray[Cnt].SetGameServer(this);
        p_Session = (MMOSession *)(&_PlayerArray[Cnt]);
        SetSessionArray(Cnt, p_Session);
    }

    InitializeSRWLock(&_SessionKeyMap_srwlock);

    _p_LanClient_Agent = nullptr;
    _p_LanClient_Login = nullptr;
    _p_LanClient_Monitoring = nullptr;

    // DB 접속.
    if (false == ConnectDB())
    {
        SYSLOG(L"GameServer", LOG_SYSTEM, L"ConnectDB Failed !!");
        exit(0);
    }

    // DBWriter Message Queue & MemoryPool 생성
    _DBWriterMessagePool = new MemoryPoolTLS<_st_DBWRITER_MSG>(DB_WRITER_MESSAGE_POOL_CHUNK_SIZE, false);
    _DBWriterMessageQueue = new LockfreeQueue<_st_DBWRITER_MSG *>(0);

    // DBWriter Event 생성
    _DBWriterEvent = CreateEvent(NULL, FALSE, FALSE, L"DBWriterEvent");

    // DBWriter Thread 생성
    _DBWriterThread = (HANDLE)_beginthreadex(NULL, 0, DBWriterThread, this, NULL, &ThreadId);
    if (INVALID_HANDLE_VALUE == _DBWriterThread)
    {
        SYSLOG(L"GameServer", LOG_SYSTEM, L"DBWriterThread Creation Failed !!");
        exit(0);
    }

    // Map과 PathFinder 초기화
    _p_Map = nullptr;
    _p_PathFinder = nullptr;

    // MapTile
    int CntX;
    int ObjectIndex;
    _pp_MapTile = new st_MAP_TILE *[TILE_MAX_Y];
    for (Cnt = 0; Cnt < TILE_MAX_Y; ++Cnt)
    {
        _pp_MapTile[Cnt] = new st_MAP_TILE[TILE_MAX_X];
        for (CntX = 0; CntX < TILE_MAX_X; ++CntX)
        {
            for (ObjectIndex = 0; ObjectIndex < MAP_TILE_OBJECT_MAX; ++ObjectIndex)
            {
                _pp_MapTile[Cnt][CntX].Object[ObjectIndex].ObjectType = -1;
                _pp_MapTile[Cnt][CntX].Object[ObjectIndex].p_Object = nullptr;
            }
            _pp_MapTile[Cnt][CntX].ObjectNum = 0;
        }
    }

    // Sector
    _pp_Sector = new st_SECTOR *[SECTOR_MAX_Y];
    for (Cnt = 0; Cnt < SECTOR_MAX_Y; ++Cnt)
    {
        _pp_Sector[Cnt] = new st_SECTOR[SECTOR_MAX_X];
    }

    // Rank 초기화
    _RankTime = 0;
    _RankDataSize = 0;
    for (Cnt = 0; Cnt < KILL_RANK_MAX; ++Cnt)
    {
        _RankData[Cnt].Nickname[0] = L'\0';
        _RankData[Cnt].KillCount = 0;
    }

    // PlayerPosAlert 초기화
    _PlayerPosAlertTime = 0;
    _PlayerPosAlertSectorCount = 0;

    // Schedule 초기화
    GetLocalTime(&_ScheduleTime);
    for (Cnt = 0; Cnt < SCHEDULE_MAX; ++Cnt)
    {
        _Schedule[Cnt].IsUse = false;
        _Schedule[Cnt].IsSchedule = false;
    }

    _Schedule[0].IsUse = true;
    _Schedule[0].Year = 0;
    _Schedule[0].Month = 0;
    _Schedule[0].Day = 0;
    _Schedule[0].Hour = 4;
    _Schedule[0].Minute = 0;

    //// 게임 서버가 시작되었으므로 모든 서버의 유저들을 미사용중 유저들로 바꾼다.(이 부분은 적용하지 않는다. 왜냐하면 로그인 서버가 로테이션 돌 때 초기화 하기 때문이다. 하지만 실제 서비스에서는 적용해야 할 듯.)
    ////if (false == _p_DBAccount_Auth->ReadDB(enDB_ACCOUNT_READ_RESET_STATUS_ALL, nullptr, nullptr))
    //if (false == _p_DBAccount_Writer->ReadDB(enDB_ACCOUNT_READ_RESET_STATUS_ALL, nullptr, nullptr))
    //    CrashDump::Crash();

    // 프로파일링
    ProfileInitial();
}
CGameServer::~CGameServer(void)
{
    int Cnt;
    delete[] _PlayerArray;

    for (Cnt = 0; Cnt < TILE_MAX_Y; ++Cnt)
    {
        delete[] _pp_MapTile[Cnt];
    }
    delete[] _pp_MapTile;

    for (Cnt = 0; Cnt < SECTOR_MAX_Y; ++Cnt)
    {
        delete[] _pp_Sector[Cnt];
    }
    delete[] _pp_Sector;
}

bool CGameServer::Start(WCHAR *p_ListenIP, USHORT Port, int WorkerThread, bool EnableNagle, BYTE PacketCode, BYTE PacketKey1, BYTE PacketKey2)
{
    if (nullptr == _p_Map)
        return false;
    if (nullptr == _p_PathFinder)
        return false;
    int Cnt;
    for (Cnt = 0; Cnt < _MaxSession; ++Cnt)
        _PlayerArray[Cnt].SetGameData(_MaxSession, _PlayerArray, _p_Map, _p_PathFinder, _pp_MapTile, _pp_Sector, _TimeManager);
    return ServerStart(p_ListenIP, Port, WorkerThread, EnableNagle, PacketCode, PacketKey1, PacketKey2);
}
bool CGameServer::Stop(void)
{
    return ServerStop();
}

int CGameServer::GetUseSize_DBWriterMessageQueue(void)
{
    return _DBWriterMessageQueue->GetUseSize();
}
int CGameServer::GetAllocCount_DBWriterMessagePool(void)
{
    return _DBWriterMessagePool->GetChunkPoolBlockCount();
}

int CGameServer::GetLoginSessionKeyCount(void)
{
    return _SessionKeyMap.size();
}
void CGameServer::AddLoginSessionKey(__int64 AccountNo, BYTE *p_SessionKey)
{
    std::map<__int64, st_LOGIN_SESSION_KEY *>::iterator SessionKeyMapItr;
    st_LOGIN_SESSION_KEY *NewLoginSessionKey;
    
    NewLoginSessionKey = new st_LOGIN_SESSION_KEY;
    NewLoginSessionKey->AccountNo = AccountNo;
    memcpy_s(NewLoginSessionKey->SessionKey, SESSION_KEY_BYTE_LEN, p_SessionKey, SESSION_KEY_BYTE_LEN);
    NewLoginSessionKey->AuthTime = _TimeManager->GetTickTime64();
    
    AcquireSRWLockExclusive(&_SessionKeyMap_srwlock);
    
    SessionKeyMapItr = _SessionKeyMap.find(AccountNo);
    if (SessionKeyMapItr != _SessionKeyMap.end())
    {
        delete SessionKeyMapItr->second;
        _SessionKeyMap.erase(SessionKeyMapItr);
    }
    
    _SessionKeyMap.insert(std::map<__int64, st_LOGIN_SESSION_KEY *>::value_type(NewLoginSessionKey->AccountNo, NewLoginSessionKey));
    
    ReleaseSRWLockExclusive(&_SessionKeyMap_srwlock);
}
bool CGameServer::CheckLoginSessionKey(__int64 AccountNo, BYTE *p_SessionKey)
{
    std::map<__int64, st_LOGIN_SESSION_KEY *>::iterator SessionKeyMapItr;
    st_LOGIN_SESSION_KEY *LoginSessionKey;
    bool SessionKeyCmp;
    
    AcquireSRWLockExclusive(&_SessionKeyMap_srwlock);
    
    SessionKeyMapItr = _SessionKeyMap.find(AccountNo);
    if (SessionKeyMapItr != _SessionKeyMap.end())
    {
        LoginSessionKey = SessionKeyMapItr->second;
        _SessionKeyMap.erase(SessionKeyMapItr);
    }
    else
    {
        LoginSessionKey = nullptr;
    }
    
    ReleaseSRWLockExclusive(&_SessionKeyMap_srwlock);
    
    if (nullptr == LoginSessionKey)
    {
        InterlockedIncrement(&_Monitor_SessionNotFound);
        return false;
    }
    
    if (0 == memcmp(LoginSessionKey->SessionKey, p_SessionKey, SESSION_KEY_BYTE_LEN))
    {
        SessionKeyCmp = true;
    }
    else
    {
        InterlockedIncrement(&_Monitor_SessionMiss);
        SessionKeyCmp = false;
    }
    
    delete LoginSessionKey;
    return SessionKeyCmp;

    return true;
}
void CGameServer::TimeoutLoginSessionKey(void)
{
    std::map<__int64, st_LOGIN_SESSION_KEY *>::iterator SessionKeyMapItr;
    ULONGLONG CurTickTime;
    
    CurTickTime = _TimeManager->GetTickTime64();
    
    AcquireSRWLockExclusive(&_SessionKeyMap_srwlock);
    
    for (SessionKeyMapItr = _SessionKeyMap.begin(); SessionKeyMapItr != _SessionKeyMap.end(); ++SessionKeyMapItr)
    {
        //if (SessionKeyMapItr->second->AuthTime + TIMEOUT_LOGIN_SESSION_KEY < _TimeManager->GetTickTime64())
        if (SessionKeyMapItr->second->AuthTime + TIMEOUT_LOGIN_SESSION_KEY < CurTickTime)
        {
            delete SessionKeyMapItr->second;
            SessionKeyMapItr = _SessionKeyMap.erase(SessionKeyMapItr);
        }
        else
        {
            break;
        }
    }
    
    ReleaseSRWLockExclusive(&_SessionKeyMap_srwlock);
}

OBJECT_ID CGameServer::GetObjectID(__int64 PlayerIndex)
{
    OBJECT_ID ObjectID;
    ObjectID = NEW_OBJECT_ID(PlayerIndex);
    return ObjectID;
}

void CGameServer::SendPacket_Broadcast(Packet *p_Packet)
{
    // Game중인 유저들에게만 보낸다. // Die 포함 // Dummy 제외
    Player *p_Player;
    int PlayerIndex;
    for (PlayerIndex = 0; PlayerIndex < _MaxSession; ++PlayerIndex)
    {
        //p_Player = _PlayerArray[PlayerIndex];
        p_Player = &_PlayerArray[PlayerIndex];
        if (p_Player->_Mode != MODE_GAME || ACCOUNT_NUM_DEFAULT == p_Player->_AccountNo)
            continue;
        if (USER_TYPE_DUMMY == p_Player->_UserType)
            continue;
        p_Player->SendPacket(p_Packet);
    }
}
void CGameServer::SendPacket_Ranking(void)
{
    Packet *p_RankingPacket;

    // PacketMake_KillRank를 game server로 빼던지 player 내부에서 처리하도록 할 것.
    p_RankingPacket = PacketMake_KillRank(_RankDataSize, _RankData);
    SendPacket_Broadcast(p_RankingPacket);
    p_RankingPacket->Free();
}
void CGameServer::SendPacket_PlayerPosAlert(void)
{
    Packet *p_PlayerPosAlertPacket;

    p_PlayerPosAlertPacket = PacketMake_PlayerPosAlert(_PlayerPosAlertSectorPos, _PlayerPosAlertSectorCount);
    SendPacket_Broadcast(p_PlayerPosAlertPacket);
    p_PlayerPosAlertPacket->Free();
}
Packet *CGameServer::PacketMake_KillRank(BYTE RankDataSize, st_KILL_RANK_DATA *p_RankData)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    int Cnt;

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_KILL_RANK;
    *p_Packet << RankDataSize;
    //p_Packet->PutData((char *)p_RankData, sizeof(st_KILL_RANK_DATA) * RankDataSize);
    for (Cnt = 0; Cnt < RankDataSize; ++Cnt)
    {
        p_Packet->PutData((char *)p_RankData[Cnt].Nickname, NICK_MAX_LEN * sizeof(WCHAR));
        *p_Packet << p_RankData[Cnt].KillCount;
    }
    return p_Packet;
}
Packet *CGameServer::PacketMake_PlayerPosAlert(st_SECTOR_POS *p_Sector, BYTE Count)
{
    Packet *p_Packet = Packet::Alloc();
    if (nullptr == p_Packet)
    {
        CrashDump::Crash();
        return nullptr;
    }

    if (Count > PLAYER_POS_ALERT_SECTOR_MAX || Count < 1)
    {
        CrashDump::Crash();
        return nullptr;
    }

    int Cnt;

    *p_Packet << (WORD)en_PACKET_CS_GAME_RES_PLAYER_POS_ALERT;
    *p_Packet << Count;
    for (Cnt = 0; Cnt < Count; ++Cnt)
    {
        *p_Packet << p_Sector[Cnt].SectorX;
        *p_Packet << p_Sector[Cnt].SectorY;
    }
    return p_Packet;
}

//void CGameServer::OnAuth_Update(__int64 ThreadNum)
void CGameServer::OnAuth_Update(void)
{
    TimeoutLoginSessionKey();
}
void CGameServer::OnGame_Update(void)
{
    __int64 PlayerIndex;
    //Player *p_Player;
    ULONGLONG CurTime;

    bool IsRank;
    std::list<Player *>::iterator RankPlayerListItr;
    int RankIndex;
    size_t RankSize;
    bool IsSetRank;

    bool IsPlayerPosAlert;
    int PlayerPosAlertCnt;
    bool AddPlayerPosAlertFlag;

    //CurTime = GetTickCount64();
    CurTime = _TimeManager->GetTickTime64();

    IsRank = false;
    IsPlayerPosAlert = false;

    ////if (true == _IsBotKillResetSchedule)
    //if (true == _Schedule[0].IsSchedule)
    //{
    //    Packet *p_MakePacket;
    //    for (PlayerIndex = 0; PlayerIndex < _MaxSession; ++PlayerIndex)
    //    {
    //        if (_PlayerArray[PlayerIndex]._Mode != MODE_GAME || ACCOUNT_NUM_DEFAULT == _PlayerArray[PlayerIndex]._AccountNo)
    //            continue;
    //        if (USER_TYPE_DUMMY == _PlayerArray[PlayerIndex]._UserType)
    //            continue;
    //
    //        _PlayerArray[PlayerIndex]._GuestKillCount = 0;
    //
    //        p_MakePacket = _PlayerArray[PlayerIndex].PacketMake_BotKill();
    //        _PlayerArray[PlayerIndex].SendPacket(p_MakePacket);
    //        p_MakePacket->Free();
    //    }
    //    _Schedule[0].IsSchedule = false;
    //
    //    SYSLOG(L"Schedule", LOG_SYSTEM, L"BotKill Reset !!!");
    //}

    // 이번 프레임에 랭크를 계산할 것인지 체크.
    if (_RankTime + KILL_RANK_DELAY < CurTime)
    {
        IsRank = true;
        _RankTime = CurTime;
        _RankPlayerList.clear();
    }
    else
    {
        // 이번 프레임에 PlayerPosAlert을 보낼 것인지 체크.
        if (_PlayerPosAlertTime + PLAYER_POS_ALERT_SEND_TIME < CurTime)
        {
            IsPlayerPosAlert = true;
            _PlayerPosAlertTime = CurTime;
            _PlayerPosAlertSectorCount = 0;
        }
    }

    for (PlayerIndex = 0; PlayerIndex < _MaxSession; ++PlayerIndex)
    {
        if (_PlayerArray[PlayerIndex]._Mode != MODE_GAME || ACCOUNT_NUM_DEFAULT == _PlayerArray[PlayerIndex]._AccountNo)
            continue;
        if (_PlayerArray[PlayerIndex]._HP < 1)      // Die라면 랭킹산정 제외.
            continue;
        
        _PlayerArray[PlayerIndex].GameUpdate(CurTime);

        if (_PlayerArray[PlayerIndex]._UserType != USER_TYPE_DUMMY)
        {
            // Rank 계산
            if (true == IsRank)
            {
                if (true == _RankPlayerList.empty())
                {
                    _RankPlayerList.push_front(&_PlayerArray[PlayerIndex]);
                }
                else
                {
                    RankSize = _RankPlayerList.size();
                    if (RankSize > KILL_RANK_MAX)
                        RankSize = KILL_RANK_MAX;

                    IsSetRank = false;
                    RankPlayerListItr = _RankPlayerList.begin();
                    for (RankIndex = 0; RankIndex < RankSize; ++RankIndex)
                    {
                        if ((*RankPlayerListItr)->_KillCount < _PlayerArray[PlayerIndex]._KillCount)
                        {
                            _RankPlayerList.insert(RankPlayerListItr, &_PlayerArray[PlayerIndex]);
                            IsSetRank = true;
                            break;
                        }
                        RankPlayerListItr++;
                    }

                    if (false == IsSetRank && _RankPlayerList.size() < KILL_RANK_MAX)
                    {
                        _RankPlayerList.push_back(&_PlayerArray[PlayerIndex]);
                    }

                    //if (RankSize == RankIndex)
                    //{
                    //    _RankPlayerList.push_back(&_PlayerArray[PlayerIndex]);
                    //}
                }
            }
            else
            {
                if (true == IsPlayerPosAlert)
                {
                    AddPlayerPosAlertFlag = true;
                    for (PlayerPosAlertCnt = 0; PlayerPosAlertCnt < _PlayerPosAlertSectorCount; ++PlayerPosAlertCnt)
                    {
                        if (_PlayerPosAlertSectorPos[PlayerPosAlertCnt].SectorX == _PlayerArray[PlayerIndex]._SectorX &&
                            _PlayerPosAlertSectorPos[PlayerPosAlertCnt].SectorY == _PlayerArray[PlayerIndex]._SectorY)
                        {
                            AddPlayerPosAlertFlag = false;
                            break;
                        }
                    }
                    if (true == AddPlayerPosAlertFlag)
                    {
                        _PlayerPosAlertSectorPos[_PlayerPosAlertSectorCount].SectorX = _PlayerArray[PlayerIndex]._SectorX;
                        _PlayerPosAlertSectorPos[_PlayerPosAlertSectorCount].SectorY = _PlayerArray[PlayerIndex]._SectorY;
                        _PlayerPosAlertSectorCount++;
                    }
                }
            }
        }

        
    }

    // Rank 정산
    if (true == IsRank)
    {
        _RankDataSize = _RankPlayerList.size();
        if (_RankDataSize != 0)
        {
            if (_RankDataSize > KILL_RANK_MAX)
                _RankDataSize = KILL_RANK_MAX;

            RankPlayerListItr = _RankPlayerList.begin();
            for (RankIndex = 0; RankIndex < _RankDataSize; ++RankIndex)
            {
                //_RankData[RankIndex].Nickname = (*RankPlayerListItr)->_Nickname;
                //wcscpy_s(_RankData[RankIndex].Nickname, NICK_MAX_LEN, (*RankPlayerListItr)->_Nickname);
                wmemcpy_s(_RankData[RankIndex].Nickname, NICK_MAX_LEN, (*RankPlayerListItr)->_Nickname, NICK_MAX_LEN);
                _RankData[RankIndex].KillCount = (*RankPlayerListItr)->_KillCount;
                RankPlayerListItr++;
            }
        }

        // 랭킹을 쏜다.  // 일단 무조건 쏴보자.
        SendPacket_Ranking();
    }
    else
    {
        if (true == IsPlayerPosAlert)
        {
            ////PlayerPosAlertCnt = 0;
            //_PlayerPosAlertSectorCount = 0;
            //for (SectorY = 0; SectorY < SECTOR_MAX_Y; ++SectorY)
            //{
            //    for (SectorX = 0; SectorX < SECTOR_MAX_X; ++SectorX)
            //    {
            //        if (_pp_Sector[SectorY][SectorX].PlayerList.size() > 0)
            //        {
            //            //_PlayerPosAlertSectorPos[PlayerPosAlertCnt].SectorX = SectorX;
            //            //_PlayerPosAlertSectorPos[PlayerPosAlertCnt].SectorY = SectorY;
            //            //PlayerPosAlertCnt++;
            //            //if (PlayerPosAlertCnt >= PLAYER_POS_ALERT_SECTOR_MAX)
            //            //    break;
            //
            //            _PlayerPosAlertSectorPos[_PlayerPosAlertSectorCount].SectorX = SectorX;
            //            _PlayerPosAlertSectorPos[_PlayerPosAlertSectorCount].SectorY = SectorY;
            //            _PlayerPosAlertSectorCount++;
            //            if (_PlayerPosAlertSectorCount >= PLAYER_POS_ALERT_SECTOR_MAX)
            //                break;
            //        }
            //    }
            //}

            if (_PlayerPosAlertSectorCount != 0)
                SendPacket_PlayerPosAlert();
        }
    }
}

void CGameServer::OnWorker_HeartBeat(void)
{
    _p_LanClient_Login->SendHeartBeat(dfTHREAD_TYPE_WORKER);
    SetEvent(_DBWriterEvent);       // DB 하트비트를 보내기 위해 깨운다.
}
void CGameServer::OnSend_HeartBeat(void)
{
    // 아무것도 없음.
}
void CGameServer::OnAuth_HeartBeat(void)
{

}
void CGameServer::OnGame_HeartBeat(void)
{
    _p_LanClient_Login->SendHeartBeat(dfTHREAD_TYPE_GAME);
}

void CGameServer::OnError(int ErrorCode, WCHAR *p_Error)
{

}

bool CGameServer::ConnectDB(void)
{
    bool IsConnect = false;

    _p_DBLog_Auth = new CDBLog(g_GameServerConfig._LogIP, g_GameServerConfig._LogUser, g_GameServerConfig._LogPassword, g_GameServerConfig._LogDBName, g_GameServerConfig._LogPort);
    IsConnect = _p_DBLog_Auth->Connect();
    if (false == IsConnect)
    {
        return false;
    }
    //_p_DBLog_Auth = new CDBLogTLS(g_GameServerConfig._LogIP, g_GameServerConfig._LogUser, g_GameServerConfig._LogPassword, g_GameServerConfig._LogDBName, g_GameServerConfig._LogPort);
    //IsConnect = _p_DBLog_Auth->Connect();
    //if (false == IsConnect)
    //{
    //    return false;
    //}
    _p_DBLog_Writer = new CDBLog(g_GameServerConfig._LogIP, g_GameServerConfig._LogUser, g_GameServerConfig._LogPassword, g_GameServerConfig._LogDBName, g_GameServerConfig._LogPort);
    IsConnect = _p_DBLog_Writer->Connect();
    if (false == IsConnect)
    {
        return false;
    }

    _p_DBAccount_Auth = new CDBAccount(g_GameServerConfig._AccountIP, g_GameServerConfig._AccountUser, g_GameServerConfig._AccountPassword, g_GameServerConfig._AccountDBName, g_GameServerConfig._AccountPort);
    _p_DBAccount_Auth->SetDBLog(_p_DBLog_Auth);
    IsConnect = _p_DBAccount_Auth->Connect();
    if (false == IsConnect)
    {
        return false;
    }
    //_p_DBAccount_Auth = new CDBAccountTLS(g_GameServerConfig._AccountIP, g_GameServerConfig._AccountUser, g_GameServerConfig._AccountPassword, g_GameServerConfig._AccountDBName, g_GameServerConfig._AccountPort);
    //_p_DBAccount_Auth->SetDBLog(_p_DBLog_Auth);
    //IsConnect = _p_DBAccount_Auth->Connect();
    //if (false == IsConnect)
    //{
    //    return false;
    //}
    _p_DBAccount_Game = new CDBAccount(g_GameServerConfig._AccountIP, g_GameServerConfig._AccountUser, g_GameServerConfig._AccountPassword, g_GameServerConfig._AccountDBName, g_GameServerConfig._AccountPort);
    IsConnect = _p_DBAccount_Game->Connect();
    if (false == IsConnect)
    {
        return false;
    }
    _p_DBAccount_Writer = new CDBAccount(g_GameServerConfig._AccountIP, g_GameServerConfig._AccountUser, g_GameServerConfig._AccountPassword, g_GameServerConfig._AccountDBName, g_GameServerConfig._AccountPort);
    _p_DBAccount_Writer->SetDBLog(_p_DBLog_Writer);
    IsConnect = _p_DBAccount_Writer->Connect();
    if (false == IsConnect)
    {
        return false;
    }

    _p_DBGame_Auth = new CDBGame(g_GameServerConfig._GameIP, g_GameServerConfig._GameUser, g_GameServerConfig._GamePassword, g_GameServerConfig._GameDBName, g_GameServerConfig._GamePort);
    _p_DBGame_Auth->SetDBLog(_p_DBLog_Auth);
    IsConnect = _p_DBGame_Auth->Connect();
    if (false == IsConnect)
    {
        return false;
    }
    //_p_DBGame_Auth = new CDBGameTLS(g_GameServerConfig._GameIP, g_GameServerConfig._GameUser, g_GameServerConfig._GamePassword, g_GameServerConfig._GameDBName, g_GameServerConfig._GamePort);
    //_p_DBGame_Auth->SetDBLog(_p_DBLog_Auth);
    //IsConnect = _p_DBGame_Auth->Connect();
    //if (false == IsConnect)
    //{
    //    return false;
    //}
    _p_DBGame_Game = new CDBGame(g_GameServerConfig._GameIP, g_GameServerConfig._GameUser, g_GameServerConfig._GamePassword, g_GameServerConfig._GameDBName, g_GameServerConfig._GamePort);
    IsConnect = _p_DBGame_Game->Connect();
    if (false == IsConnect)
    {
        return false;
    }
    _p_DBGame_Writer = new CDBGame(g_GameServerConfig._GameIP, g_GameServerConfig._GameUser, g_GameServerConfig._GamePassword, g_GameServerConfig._GameDBName, g_GameServerConfig._GamePort);
    _p_DBGame_Writer->SetDBLog(_p_DBLog_Writer);
    IsConnect = _p_DBGame_Writer->Connect();
    if (false == IsConnect)
    {
        return false;
    }

    return true;
}
unsigned __stdcall CGameServer::DBWriterThread(void *p_Param)
{
    return ((CGameServer *)p_Param)->DBWriterThread_Update();
}
bool CGameServer::DBWriterThread_Update(void)
{
    _st_DBWRITER_MSG *p_Msg;
    bool MsgResult;

    ULONGLONG HeartbeatTick;
    ULONGLONG CurTick;

    HeartbeatTick = 0;
    while (1)
    {
        //CurTick = GetTickCount64();
        CurTick = _TimeManager->GetTickTime64();
        if (HeartbeatTick + DB_HEARTBEAT_TICK < CurTick)
        {
            if (_p_LanClient_Login != nullptr)
                _p_LanClient_Login->SendHeartBeat(dfTHREAD_TYPE_DB);
            HeartbeatTick = CurTick;
        }

        // 이벤트 방식
        WaitForSingleObject(_DBWriterEvent, INFINITE);

        while (_DBWriterMessageQueue->Dequeue(&p_Msg))
        {
            //ProfileBegin(L"DBWriterThread");

            switch (p_Msg->Type_DB)
            {
            case dfDBWRITER_TYPE_ACCOUNT:
                MsgResult = _p_DBAccount_Writer->WriteDB(p_Msg->Type_Message, p_Msg->Message);
                break;
            case dfDBWRITER_TYPE_GAME:
                MsgResult = _p_DBGame_Writer->WriteDB(p_Msg->Type_Message, p_Msg->Message);
                break;
            case dfDBWRITER_TYPE_HEARTBEAT:
                break;
            default:
                CrashDump::Crash();
                break;
            }

            if (false == MsgResult)
            {
                SYSLOG(L"DBWriterThread", LOG_ERROR, L"MsgResult FALSE # [DBType:%d][MsgType:%d]",
                    p_Msg->Type_DB, p_Msg->Type_Message);
            }

            _Monitor_Counter_DBWriteTPS++;

            _DBWriterMessagePool->Free(p_Msg);

            //ProfileEnd(L"DBWriterThread");
        }
    }

    return true;
}