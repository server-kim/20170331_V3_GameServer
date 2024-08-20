/*

# 포인터 찌르는 것을 지역변수로 바꾸기만 해도 프레임이 올라간다. -> 섹터 등을 수정할 것. -> 완료
# 로직을 변경하면서 싱크가 안맞는다. -> 이동처리를 무조건 해야하나? -> 일단 완료.
# 타일 제한을 더 늘리고 체크 수만 줄이자. -> 일단 수용량만 두배로 늘렸음.

# 현재 공격 쿨타임 동안 상대방이 이동을 하면 따라가지 않고 멈춰있는다.
-> 부하를 적게 하면서 구현할 방법을 찾아야 할 듯.
# 부하지점을 찾아야 한다. -> 해결하는 중.

# 공격지점을 찾을 때 버그가 있다. 복잡한 로직을 간소화하고 버그잡을 것.
-> 버그는 좀더 지켜봐야 한다. 추가로 로직을 좀더 간단히 개선할 필요가 있다.
-> 완료.

# 공격지점 찾는 로직을 개선 했는데도 fps가 다시 떨어졌다. 좀더 올릴 방안을 찾을 것.
-> 이게 한계인듯.

# Config Parser에서 로그레벨 적용하기 구현할 것. -> 완료.
# 현재 g_GameServerConfig의 로그레벨은 쓰고 있지 않다. -> 이제 쓴다.
# SECTOR_MAX - define과 enum으로 이중으로 되어있다. -> enum으로 통일.

// 변경사항.
1. 섹터 개수는 100개까지만 내려보내기.(아군 적군 상관없이 플레이어가 있으면 내려주는 걸로 한다.)
2. 패스 보낼 때 시작 좌표도 보내기.(이건 자신의 좌표를 따로 패킷에 담아 보내는 걸로 한다.) -> 완료

# Rank 계산할 때 랭크사이즈를 받지 말고 로직 이터레이터만 사용하도록 수정할 것. -> 기존 로직이 더 좋은듯.
# 최초 접속시 공격불가 -> 완료.

# 게임 서버가 시작되었으므로 모든 서버의 유저들을 미사용중 유저들로 바꾼다. -> 이 부분은 적용하지 않는다. 왜냐하면 로그인 서버가 로테이션 돌 때 초기화 하기 때문이다. 하지만 실제 서비스에서는 적용해야 할 듯.
# 더미에서 온게임중으로 뜨는 문제 -> _DBWriterMessageQueue는 동적으로 가야 한다.(특정 상황에 예측하기 어려운 많은 메세지가 발생(더미가 한번에 접속을 끊는 등))

# push로 밀릴 경우 무조건 패스리셋하는 것 수정 -> 완료.
# 플레이어 위치 알림 구현할 것. -> 완료



# Lockfree Queue Peek 개선.
# Foreach
# 현재 최대 접속자에 대한 처리가 안되고 있다. Accept 스레드에서 거를 것.
# SECTOR_MAX - 추후 맵 너비와 길이를 파싱하여 들고 있는 쪽으로 수정할 것.
# MapTile - 추후 맵 너비와 길이를 파싱하여 들고 있는 쪽으로 수정할 것.

# 데미지 테이블에 랜덤값을 넣도록 할 것.
# 선생님은 큐사이즈 5만.



### 라이브 버그
1. 랭킹 이름출력 버그 -> 완료.
2. 게스트 위치알림 안나옴. -> 완료.
3. 걸어가다가 죽으면 기어감. -> Die 패킷 보내기전에 Stop 패킷을 보내는 것으로 해결.

### 라이브 점검사항
1. 파티별 생성위치 수정 -> 완료.
2. 엘프 파티 상관없이 모두 때리고 죽고 가능한지 확인. -> 완료.
3. Player 테이블에 킬/다이 저장여부 확인. -> 완료
4. 1U 서버 또는 데스크탑에 서버 및 틀라이언트 테스트 가능한 환경 구축. -> 완료.

### 패치사항
Guest Kill을 없애고 Dummy Kill 하는 용도로 쓴다.(이거는 저장해놨다가 로그인하면 불러올 것.)
새벽 4시에 스케쥴러를 돌려서 guest kill을 0으로 초기화.

DestTile 좌표가 타겟을 가리키고 있어서 문제가 된다. -> 이동 목적지를 가리키는 것으로 수정할 것.




# 현재는 불가능한 사항 목록
### SendMonitoringDataUpdate()에서 TimeStamp를 64비트 변수로 바꿀 것.


###
p_UDPModule
세션키 관련 함수



*/

#include "stdafx.h"
#include "TotalHeader.h"

const WCHAR *g_ConfigFileName = L"GameServer.Cnf";
const WCHAR *g_MapFileName = L"Map.txt";

CrashDump Dump;
CGameServerConfig g_GameServerConfig;

bool GameServerStart;
CGameServer *p_GameServer;
//CLanClientAgent *p_LanClientAgent;
CLanClientLogin *p_LanClientLogin;
CLanClientMonitoring *p_LanClientMonitoring;

CUDPModule *p_UDPModule;
CMAP *p_Map;
CGamePathFinder *p_PathFinder;

void GameServer_Run(void);
void MonitoringServerStatus(void);
bool KeyProcess(void);

int _tmain(int argc, _TCHAR* argv[])
{
    GameServer_Run();
    return 0;
}

void GameServer_Run(void)
{
    timeBeginPeriod(1);

    // 로그 설정
    //g_SystemLOG->SetLogOption(LOG_CONSOLE | LOG_FILE);
    //g_SystemLOG->SetLogOption(LOG_CONSOLE);
    g_SystemLOG->SetLogOption(LOG_FILE);
    g_SystemLOG->SetLogLevel(LOG_ERROR);
    //g_SystemLOG->SetLogLevel(LOG_DEBUG);
    g_SystemLOG->SetLogDirectory(L"Log_GameServer");

    // 플래그 초기화
    GameServerStart = false;
    SYSLOG(L"GameServer", LOG_SYSTEM, L"### GameServer_Run !!");

    // 설정파일 로드
    if (false == g_GameServerConfig.ParseINI(g_ConfigFileName))
    {
        SYSLOG(L"GameServer", LOG_SYSTEM, L"%s Load Failed !!", g_ConfigFileName);
        return;
    }
    SYSLOG(L"GameServer", LOG_SYSTEM, L"-----------------------------------------------------------");
    SYSLOG(L"GameServer", LOG_SYSTEM, L"%s Load Complete", g_ConfigFileName);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"-----------------------------------------------------------");
    SYSLOG(L"GameServer", LOG_SYSTEM, L"         SERVER NAME : %s", g_GameServerConfig._ServerName);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"      LISTEN BIND IP : %s:%u", g_GameServerConfig._BindIP, g_GameServerConfig._BindPort);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"     LOGIN SERVER IP : %s:%u", g_GameServerConfig._LoginServerIP, g_GameServerConfig._LoginServerPort);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"MONITORING SERVER IP : %s:%u", g_GameServerConfig._MonitoringServerIP, g_GameServerConfig._MonitoringServerPort);
    //SYSLOG(L"GameServer", LOG_SYSTEM, L"            AGENT IP : %s:%u", g_GameServerConfig._AgentIP, g_GameServerConfig._AgentPort);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"       WORKER THREAD : %d", g_GameServerConfig._WorkerThreadCnt);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"          CLIENT MAX : %d", g_GameServerConfig._ClientMax);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"             VERSION : %d", g_GameServerConfig._Version);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"       PACKET ENCODE : Code 0x%x / KEY1 : 0x%x / KEY2 0x%x", g_GameServerConfig._PacketCode, g_GameServerConfig._PacketKey1, g_GameServerConfig._PacketKey2);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"           LOG LEVEL : %s", g_GameServerConfig._LogLevelStr);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"        DB AccountDB : %s:%u [user:%s][DB:%s]", g_GameServerConfig._AccountIP, g_GameServerConfig._AccountPort, g_GameServerConfig._AccountUser, g_GameServerConfig._AccountDBName);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"           DB GameDB : %s:%u [user:%s][DB:%s]", g_GameServerConfig._GameIP, g_GameServerConfig._GamePort, g_GameServerConfig._GameUser, g_GameServerConfig._GameDBName);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"            DB LogDB : %s:%u [user:%s][DB:%s]", g_GameServerConfig._LogIP, g_GameServerConfig._LogPort, g_GameServerConfig._LogUser, g_GameServerConfig._LogDBName);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"-----------------------------------------------------------");

    // 로그 레벨 세팅
    g_SystemLOG->SetLogLevel(g_GameServerConfig._LogLevel);

    // 맵 로드
    p_Map = new CMAP;
    if (false == p_Map->LoadMap(g_MapFileName))
    {
        SYSLOG(L"GameServer", LOG_SYSTEM, L"%s Load Failed !!", g_MapFileName);
        delete p_Map;
        return;
    }
    SYSLOG(L"GameServer", LOG_SYSTEM, L"%s Load Complete", g_MapFileName);

    // 게임서버 생성
    p_GameServer = new CGameServer(g_GameServerConfig._ClientMax);

    // Map Setting
    p_GameServer->SetMap(p_Map);

    p_PathFinder = new CGamePathFinder(p_Map, dfPATH_POINT_MAX);
    p_GameServer->SetPathFinder(p_PathFinder);

    // LanClient Setting
    p_LanClientLogin = new CLanClientLogin(p_GameServer);
    p_GameServer->SetLanClientLogin(p_LanClientLogin);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"LanClientLogin # Created !!");

    //p_LanClientAgent = new CLanClientAgent(p_GameServer);
    //p_GameServer->SetLanClientAgent(p_LanClientAgent);

    p_LanClientMonitoring = new CLanClientMonitoring(p_GameServer);
    p_GameServer->SetLanClientMonitoring(p_LanClientMonitoring);
    SYSLOG(L"GameServer", LOG_SYSTEM, L"LanClientMonitoring # Created !!");

    // Server Start
    GameServerStart = p_GameServer->Start(g_GameServerConfig._BindIP, g_GameServerConfig._BindPort, g_GameServerConfig._WorkerThreadCnt, false, g_GameServerConfig._PacketCode, g_GameServerConfig._PacketKey1, g_GameServerConfig._PacketKey2);
    if (false == GameServerStart)
    {
        SYSLOG(L"GameServer", LOG_SYSTEM, L"!!! GameServer Start FAILED !!");
        return;
    }
    SYSLOG(L"GameServer", LOG_SYSTEM, L"### GameServer Start !!");

    if (false == p_LanClientLogin->Connect(g_GameServerConfig._LoginServerIP, g_GameServerConfig._LoginServerPort, 0))
        SYSLOG(L"GameServer", LOG_SYSTEM, L"LanClientLogin # Connect Failed !!");
    else
        SYSLOG(L"GameServer", LOG_SYSTEM, L"LanClientLogin # Connect Success !!");

    if (false == p_LanClientMonitoring->Connect(g_GameServerConfig._MonitoringServerIP, g_GameServerConfig._MonitoringServerPort, 0))
        SYSLOG(L"GameServer", LOG_SYSTEM, L"LanClientMonitoring # Connect Failed !!");
    else
        SYSLOG(L"GameServer", LOG_SYSTEM, L"LanClientMonitoring # Connect Success !!");

    // UDP
    p_UDPModule = new CUDPModule(L"127.0.0.1", 42001, 5);
    p_UDPModule->Run();

    //// 임시로 스케쥴을 여기에 넣는다.
    //bool IsBotKillResetSchedule = true;

    while (1)
    {
        if (true == p_UDPModule->_ShutdownFlag)
        {
            p_LanClientLogin->Disconnect();
            p_LanClientMonitoring->Disconnect();
            p_GameServer->Stop();
            break;
        }



        // 임시로 스케쥴을 여기에 넣는다.
        SYSTEMTIME LocalTime;
        GetLocalTime(&LocalTime);

        //if (LocalTime.wHour != p_GameServer->_ScheduleTime.wHour)
        if (LocalTime.wMinute != p_GameServer->_ScheduleTime.wMinute)
        {
            for (int Cnt = 0; Cnt < SCHEDULE_MAX; ++Cnt)
            {
                if (false == p_GameServer->_Schedule[Cnt].IsUse)
                    continue;
                if (LocalTime.wHour == p_GameServer->_Schedule[Cnt].Hour &&
                    LocalTime.wMinute == p_GameServer->_Schedule[Cnt].Minute)
                {
                    p_GameServer->_Schedule[Cnt].IsSchedule = true;
                }
            }
            p_GameServer->_ScheduleTime = LocalTime;
        }
        //if (true == IsBotKillResetSchedule)
        //{
        //    SYSTEMTIME LocalTime;
        //    GetLocalTime(&LocalTime);
        //
        //    //if (2017 == LocalTime.wYear && 4 == LocalTime.wMonth && 5 == LocalTime.wDay && 4 == LocalTime.wHour)
        //    if (4 == LocalTime.wHour && 25 == LocalTime.wMinute)
        //    {
        //        p_GameServer->_IsBotKillResetSchedule = true;
        //        IsBotKillResetSchedule = false;
        //    }
        //    //wprintf_s(L"%u:%u:%u %u:%u:%u", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
        //}



        //if (false == p_LanClientAgent->IsConnect())
        //    p_LanClientAgent->Connect(g_GameServerConfig._AgentIP, g_GameServerConfig._AgentPort, 0);

        if (false == p_LanClientLogin->IsConnect())
            p_LanClientLogin->Connect(g_GameServerConfig._LoginServerIP, g_GameServerConfig._LoginServerPort, 0);

        if (false == p_LanClientMonitoring->IsConnect())
            p_LanClientMonitoring->Connect(g_GameServerConfig._MonitoringServerIP, g_GameServerConfig._MonitoringServerPort, 0);
        else
            p_LanClientMonitoring->SendMonitoringDataUpdate();

        MonitoringServerStatus();
        if (false == KeyProcess())
            break;

        Sleep(999);
    }

    delete p_PathFinder;
    delete p_Map;
    //delete p_LanClientAgent;
    delete p_LanClientMonitoring;
    delete p_LanClientLogin;
    delete p_GameServer;

    timeEndPeriod(1);
}

void MonitoringServerStatus(void)
{
    long Monitor_AcceptSocket;
    long Monitor_SessionAllMode;
    long Monitor_SessionAuthMode;
    long Monitor_SessionGameMode;
    long Monitor_RealPlayer;

    //long Monitor_Counter_AuthUpdate[AuthThreadCnt];
    long Monitor_Counter_AuthUpdate;
    long Monitor_Counter_GameUpdate;
    long Monitor_Counter_SendUpdate;
    long Monitor_Counter_Accept;
    long Monitor_Counter_PackerProc;
    long Monitor_Counter_PacketSend;

    long Monitor_SessionMiss;
    long Monitor_SessionNotFound;

    long Monitor_Counter_DBWriteTPS;

    long Monitor_Sector_Count;
    long Monitor_SendQ_Size;
    long Monitor_CompletePacketQ_Size;

    long Monitor_Logout;
    long Monitor_Release;
    long Monitor_Encode;
    long Monitor_SendSessionCount;

    Monitor_AcceptSocket = p_GameServer->_Monitor_AcceptSocket;
    Monitor_SessionAllMode = p_GameServer->_Monitor_SessionAllMode;
    Monitor_SessionAuthMode = p_GameServer->_Monitor_SessionAuthMode;
    Monitor_SessionGameMode = p_GameServer->_Monitor_SessionGameMode;
    Monitor_RealPlayer = p_GameServer->_Monitor_RealPlayer;

    Monitor_Counter_Accept = p_GameServer->_Monitor_Counter_Accept;
    p_GameServer->_Monitor_Counter_Accept = 0;

    Monitor_Counter_PacketSend = p_GameServer->_Monitor_Counter_PacketSend;
    p_GameServer->_Monitor_Counter_PacketSend = 0;

    Monitor_Counter_PackerProc = p_GameServer->_Monitor_Counter_PackerProc;
    p_GameServer->_Monitor_Counter_PackerProc = 0;

    //for (Cnt = 0; Cnt < AuthThreadCnt; ++Cnt)
    //{
    //    Monitor_Counter_AuthUpdate[Cnt] = p_GameServer->_Monitor_Counter_AuthUpdate[Cnt];
    //    p_GameServer->_Monitor_Counter_AuthUpdate[Cnt] = 0;
    //}
    Monitor_Counter_AuthUpdate = p_GameServer->_Monitor_Counter_AuthUpdate;
    p_GameServer->_Monitor_Counter_AuthUpdate = 0;

    Monitor_Counter_GameUpdate = p_GameServer->_Monitor_Counter_GameUpdate;
    p_GameServer->_Monitor_Counter_GameUpdate = 0;

    Monitor_Counter_SendUpdate = p_GameServer->_Monitor_Counter_SendUpdate;
    p_GameServer->_Monitor_Counter_SendUpdate = 0;

    Monitor_SessionMiss = p_GameServer->_Monitor_SessionMiss;
    Monitor_SessionNotFound = p_GameServer->_Monitor_SessionNotFound;

    Monitor_Counter_DBWriteTPS = p_GameServer->_Monitor_Counter_DBWriteTPS;
    p_GameServer->_Monitor_Counter_DBWriteTPS = 0;

    Monitor_Sector_Count = p_GameServer->_Monitor_Sector_Count;
    Monitor_SendQ_Size = p_GameServer->_Monitor_SendQ_Size;
    Monitor_CompletePacketQ_Size = p_GameServer->_Monitor_CompletePacketQ_Size;

    Monitor_Logout = p_GameServer->_Monitor_Logout;
    p_GameServer->_Monitor_Logout = 0;
    Monitor_Release = p_GameServer->_Monitor_Release;
    p_GameServer->_Monitor_Release = 0;
    Monitor_Encode = CMMOServerV2::_Monitor_Encode;
    CMMOServerV2::_Monitor_Encode = 0;
    Monitor_SendSessionCount = p_GameServer->_Monitor_SendSessionCount;
    p_GameServer->_Monitor_SendSessionCount = 0;

    //p_GameServer->_Monitor_AcceptSocket = 0;
    //p_GameServer->_Monitor_SessionAllMode = 0;
    //p_GameServer->_Monitor_SessionAuthMode = 0;
    //p_GameServer->_Monitor_SessionGameMode = 0;

    wprintf_s(L"=== Game Server ======================================\n");
    wprintf_s(L"    AcceptSocket : %d\n", Monitor_AcceptSocket);
    wprintf_s(L"======================================================\n");
    wprintf_s(L"         Session : %d\n", Monitor_SessionAllMode);
    wprintf_s(L"      RealPlayer : %d\n", Monitor_RealPlayer);
    wprintf_s(L"\n");
    wprintf_s(L"    Session_Auth : %d\n", Monitor_SessionAuthMode);
    wprintf_s(L"    Session_Game : %d\n", Monitor_SessionGameMode);
    wprintf_s(L"\n");
    wprintf_s(L"  PacketPool Use : %d\n", Packet::GetPacketUseCount());
    //wprintf_s(L"PacketPool Alloc : %d\n", Packet::GetPacketAllocCount());
    //wprintf_s(L"  ConnInfo Alloc : %d\n", p_GameServer->GetConnectInfoAllocCount());
    //wprintf_s(L"    DB Msg Alloc : %d\n", p_GameServer->GetDBWriterMessagePoolAllocCount());
    wprintf_s(L"PacketPool Chunk : %d\n", Packet::GetPacketAllocCount());
    wprintf_s(L"  ConnInfo Chunk : %d\n", p_GameServer->GetConnectInfoAllocCount());
    wprintf_s(L"    DB Msg Chunk : %d\n", p_GameServer->GetAllocCount_DBWriterMessagePool());
    wprintf_s(L"======================================================\n");
    wprintf_s(L"       AcceptTPS : %d\n", Monitor_Counter_Accept);
    wprintf_s(L"  PacketProc TPS : %d\n", Monitor_Counter_PackerProc);
    wprintf_s(L"  PacketSend TPS : %d\n", Monitor_Counter_PacketSend);
    wprintf_s(L"\n");
    wprintf_s(L"  AuthThread FPS : %d\n", Monitor_Counter_AuthUpdate);
    //for (Cnt = 0; Cnt < AuthThreadCnt; ++Cnt)
    //{
    //    wprintf_s(L"  AuthThread %d FPS : %d\n", Cnt, Monitor_Counter_AuthUpdate[Cnt]);
    //}
    wprintf_s(L"  GameThread FPS : %d\n", Monitor_Counter_GameUpdate);
    wprintf_s(L"  SendThread FPS : %d\n", Monitor_Counter_SendUpdate);
    wprintf_s(L"     DBWrite FPS : %d\n", Monitor_Counter_DBWriteTPS);
    wprintf_s(L"     DBWrite Msg : %d\n", p_GameServer->GetUseSize_DBWriterMessageQueue());
    wprintf_s(L"\n");
    wprintf_s(L" LoginSessionKey : %d\n", p_GameServer->GetLoginSessionKeyCount());
    wprintf_s(L"     SessionMiss : %d\n", Monitor_SessionMiss);
    wprintf_s(L" SessionNotFound : %d\n", Monitor_SessionNotFound);
    wprintf_s(L"\n");
    wprintf_s(L"    Sector Count : %d\n", Monitor_Sector_Count);
    wprintf_s(L"      SendQ Size : %d\n", Monitor_SendQ_Size);
    wprintf_s(L"    PacketQ Size : %d\n", Monitor_CompletePacketQ_Size);
    wprintf_s(L"          Logout : %d\n", Monitor_Logout);
    wprintf_s(L"         Release : %d\n", Monitor_Release);
    wprintf_s(L"          Encode : %d\n", Monitor_Encode);
    wprintf_s(L"SendSessionCount : %d\n", Monitor_SendSessionCount);
    if (0 == Monitor_SendSessionCount)
        wprintf_s(L"PacketPerSession : %d\n", 0);
    else
        wprintf_s(L"PacketPerSession : %d\n", Monitor_Counter_PacketSend / Monitor_SendSessionCount);
    wprintf_s(L"\n");

    wprintf_s(L"======================================================\n");
    wprintf_s(L"\n");
}
bool KeyProcess(void)
{
    WCHAR ControlKey;
    if (_kbhit())
    {
        ControlKey = towupper(_getwch());
        if (ControlKey == L'Q')
        {
            if (true == GameServerStart)
            {
                wprintf(L"\n");
                wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                wprintf(L"!!!!!!!!!!!!!!!!!! Server Stop !!!!!!!!!!!!!!!!!!!\n");
                wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                wprintf(L"!!! Log !!!\n");
                p_GameServer->Stop();
                GameServerStart = false;
                wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                wprintf(L"\n");
            }
            else
            {
                wprintf(L"\n");
                wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                wprintf(L"!!!!!!!!!!!!!!!!!! Server Exit !!!!!!!!!!!!!!!!!!!\n");
                wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                wprintf(L"!!! Log !!!\n");
                wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                wprintf(L"\n");

                return false;
            }
        }
        //else if (ControlKey == L'S')
        //{
        //    if (true == GameServerStart)
        //    {
        //        wprintf(L"\n");
        //        wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        //        wprintf(L"!!!!!!!!!!!!!! Server Already Start !!!!!!!!!!!!!!\n");
        //        wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        //        wprintf(L"!!! Log !!!\n");
        //        wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        //        wprintf(L"\n");
        //    }
        //    else
        //    {
        //        wprintf(L"\n");
        //        wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        //        wprintf(L"!!!!!!!!!!!!!!!!!! Server Start !!!!!!!!!!!!!!!!!!\n");
        //        wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        //        wprintf(L"!!! Log !!!\n");
        //
        //        if (false == p_GameServer->Start(L"0.0.0.0", 40000, 3, false, 119, 50, 132))
        //            CrashDump::Crash();
        //        else
        //            GameServerStart = true;
        //
        //        wprintf(L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        //        wprintf(L"\n");
        //    }
        //}
        else if (ControlKey == L'P')
        {
            if (true == GameServerStart)
            {
                ProfileDataOutText();
            }
            else
            {
                wprintf(L"\n");
                wprintf(L"!!! Error : Start Server First !!!\n");
                wprintf(L"\n");
            }
        }
        else if (ControlKey == L'C')
        {
            CrashDump::Crash();
        }
    }

    return true;
}