/*

# ������ ��� ���� ���������� �ٲٱ⸸ �ص� �������� �ö󰣴�. -> ���� ���� ������ ��. -> �Ϸ�
# ������ �����ϸ鼭 ��ũ�� �ȸ´´�. -> �̵�ó���� ������ �ؾ��ϳ�? -> �ϴ� �Ϸ�.
# Ÿ�� ������ �� �ø��� üũ ���� ������. -> �ϴ� ���뷮�� �ι�� �÷���.

# ���� ���� ��Ÿ�� ���� ������ �̵��� �ϸ� ������ �ʰ� �����ִ´�.
-> ���ϸ� ���� �ϸ鼭 ������ ����� ã�ƾ� �� ��.
# ���������� ã�ƾ� �Ѵ�. -> �ذ��ϴ� ��.

# ���������� ã�� �� ���װ� �ִ�. ������ ������ ����ȭ�ϰ� �������� ��.
-> ���״� ���� ���Ѻ��� �Ѵ�. �߰��� ������ ���� ������ ������ �ʿ䰡 �ִ�.
-> �Ϸ�.

# �������� ã�� ������ ���� �ߴµ��� fps�� �ٽ� ��������. ���� �ø� ����� ã�� ��.
-> �̰� �Ѱ��ε�.

# Config Parser���� �α׷��� �����ϱ� ������ ��. -> �Ϸ�.
# ���� g_GameServerConfig�� �α׷����� ���� ���� �ʴ�. -> ���� ����.
# SECTOR_MAX - define�� enum���� �������� �Ǿ��ִ�. -> enum���� ����.

// �������.
1. ���� ������ 100�������� ����������.(�Ʊ� ���� ������� �÷��̾ ������ �����ִ� �ɷ� �Ѵ�.)
2. �н� ���� �� ���� ��ǥ�� ������.(�̰� �ڽ��� ��ǥ�� ���� ��Ŷ�� ��� ������ �ɷ� �Ѵ�.) -> �Ϸ�

# Rank ����� �� ��ũ����� ���� ���� ���� ���ͷ����͸� ����ϵ��� ������ ��. -> ���� ������ �� ������.
# ���� ���ӽ� ���ݺҰ� -> �Ϸ�.

# ���� ������ ���۵Ǿ����Ƿ� ��� ������ �������� �̻���� ������� �ٲ۴�. -> �� �κ��� �������� �ʴ´�. �ֳ��ϸ� �α��� ������ �����̼� �� �� �ʱ�ȭ �ϱ� �����̴�. ������ ���� ���񽺿����� �����ؾ� �� ��.
# ���̿��� �°��������� �ߴ� ���� -> _DBWriterMessageQueue�� �������� ���� �Ѵ�.(Ư�� ��Ȳ�� �����ϱ� ����� ���� �޼����� �߻�(���̰� �ѹ��� ������ ���� ��))

# push�� �и� ��� ������ �н������ϴ� �� ���� -> �Ϸ�.
# �÷��̾� ��ġ �˸� ������ ��. -> �Ϸ�



# Lockfree Queue Peek ����.
# Foreach
# ���� �ִ� �����ڿ� ���� ó���� �ȵǰ� �ִ�. Accept �����忡�� �Ÿ� ��.
# SECTOR_MAX - ���� �� �ʺ�� ���̸� �Ľ��Ͽ� ��� �ִ� ������ ������ ��.
# MapTile - ���� �� �ʺ�� ���̸� �Ľ��Ͽ� ��� �ִ� ������ ������ ��.

# ������ ���̺� �������� �ֵ��� �� ��.
# �������� ť������ 5��.



### ���̺� ����
1. ��ŷ �̸���� ���� -> �Ϸ�.
2. �Խ�Ʈ ��ġ�˸� �ȳ���. -> �Ϸ�.
3. �ɾ�ٰ� ������ ��. -> Die ��Ŷ ���������� Stop ��Ŷ�� ������ ������ �ذ�.

### ���̺� ���˻���
1. ��Ƽ�� ������ġ ���� -> �Ϸ�.
2. ���� ��Ƽ ������� ��� ������ �װ� �������� Ȯ��. -> �Ϸ�.
3. Player ���̺� ų/���� ���忩�� Ȯ��. -> �Ϸ�
4. 1U ���� �Ǵ� ����ũž�� ���� �� Ʋ���̾�Ʈ �׽�Ʈ ������ ȯ�� ����. -> �Ϸ�.

### ��ġ����
Guest Kill�� ���ְ� Dummy Kill �ϴ� �뵵�� ����.(�̰Ŵ� �����س��ٰ� �α����ϸ� �ҷ��� ��.)
���� 4�ÿ� �����췯�� ������ guest kill�� 0���� �ʱ�ȭ.

DestTile ��ǥ�� Ÿ���� ����Ű�� �־ ������ �ȴ�. -> �̵� �������� ����Ű�� ������ ������ ��.




# ����� �Ұ����� ���� ���
### SendMonitoringDataUpdate()���� TimeStamp�� 64��Ʈ ������ �ٲ� ��.


###
p_UDPModule
����Ű ���� �Լ�



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

    // �α� ����
    //g_SystemLOG->SetLogOption(LOG_CONSOLE | LOG_FILE);
    //g_SystemLOG->SetLogOption(LOG_CONSOLE);
    g_SystemLOG->SetLogOption(LOG_FILE);
    g_SystemLOG->SetLogLevel(LOG_ERROR);
    //g_SystemLOG->SetLogLevel(LOG_DEBUG);
    g_SystemLOG->SetLogDirectory(L"Log_GameServer");

    // �÷��� �ʱ�ȭ
    GameServerStart = false;
    SYSLOG(L"GameServer", LOG_SYSTEM, L"### GameServer_Run !!");

    // �������� �ε�
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

    // �α� ���� ����
    g_SystemLOG->SetLogLevel(g_GameServerConfig._LogLevel);

    // �� �ε�
    p_Map = new CMAP;
    if (false == p_Map->LoadMap(g_MapFileName))
    {
        SYSLOG(L"GameServer", LOG_SYSTEM, L"%s Load Failed !!", g_MapFileName);
        delete p_Map;
        return;
    }
    SYSLOG(L"GameServer", LOG_SYSTEM, L"%s Load Complete", g_MapFileName);

    // ���Ӽ��� ����
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

    //// �ӽ÷� �������� ���⿡ �ִ´�.
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



        // �ӽ÷� �������� ���⿡ �ִ´�.
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