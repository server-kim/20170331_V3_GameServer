#include "stdafx.h"
#include "TotalHeader.h"

CLanClientMonitoring::CLanClientMonitoring(CGameServer *p_GameServer)
{
    if (nullptr == p_GameServer)
        CrashDump::Crash();
    _p_GameServer = p_GameServer;
}
CLanClientMonitoring::~CLanClientMonitoring(void)
{

}

void CLanClientMonitoring::SendMonitoringServerLogin(void)
{
    if (false == IsConnect())
        return;

    Packet *p_Packet = Packet::Alloc();

    *p_Packet << (WORD)en_PACKET_SS_MONITOR_LOGIN;
    *p_Packet << (BYTE)dfMONITOR_SERVER_TYPE_GAME;
    p_Packet->PutData((char *)g_GameServerConfig._ServerName, sizeof(WCHAR) * (SERVER_NAME_LEN + 1));

    SendPacket(p_Packet);
    p_Packet->Free();
}
void CLanClientMonitoring::SendMonitoringDataUpdate(void)
{
    if (false == IsConnect())
        return;

    int TimeStamp;
    Packet *p_Packet;

    // 지금은 프로토콜이 정해져있어서 이렇게 가지만 나중에 64비트 변수로 바꿀 것.
    TimeStamp = (int)time(NULL);

    // dfMONITOR_DATA_TYPE_GAME_SESSION
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_SESSION;
    *p_Packet << (int)(_p_GameServer->_Monitor_SessionAllMode);
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_AUTH_PLAYER
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_AUTH_PLAYER;
    *p_Packet << (int)(_p_GameServer->_Monitor_SessionAuthMode);
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_GAME_PLAYER
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_GAME_PLAYER;
    *p_Packet << (int)(_p_GameServer->_Monitor_SessionGameMode);
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_ACCEPT_TPS
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_ACCEPT_TPS;
    *p_Packet << (int)(_p_GameServer->_Monitor_Counter_Accept);
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_PACKET_PROC_TPS
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_PACKET_PROC_TPS;
    *p_Packet << (int)(_p_GameServer->_Monitor_Counter_PackerProc);
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_PACKET_SEND_TPS
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_PACKET_SEND_TPS;
    *p_Packet << (int)(_p_GameServer->_Monitor_Counter_PacketSend);
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_DB_WRITE_TPS
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_DB_WRITE_TPS;
    *p_Packet << (int)(_p_GameServer->_Monitor_Counter_DBWriteTPS);
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_DB_WRITE_MSG
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_DB_WRITE_MSG;
    *p_Packet << _p_GameServer->GetUseSize_DBWriterMessageQueue();
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_AUTH_THREAD_FPS
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_AUTH_THREAD_FPS;
    //*p_Packet << (int)(_p_GameServer->_Monitor_Counter_AuthUpdate[0]);
    *p_Packet << (int)(_p_GameServer->_Monitor_Counter_AuthUpdate);
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_GAME_THREAD_FPS
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_GAME_THREAD_FPS;
    *p_Packet << (int)(_p_GameServer->_Monitor_Counter_GameUpdate);
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_PACKET_POOL
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_PACKET_POOL;
    *p_Packet << (int)(Packet::GetPacketUseCount());
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();

    // dfMONITOR_DATA_TYPE_GAME_REAL_PLAYER
    p_Packet = Packet::Alloc();
    *p_Packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
    *p_Packet << (BYTE)dfMONITOR_DATA_TYPE_GAME_REAL_PLAYER;
    *p_Packet << (int)(_p_GameServer->_Monitor_RealPlayer);
    *p_Packet << TimeStamp;

    SendPacket(p_Packet);
    p_Packet->Free();
}

void CLanClientMonitoring::OnEnterJoinServer(void)
{
    SendMonitoringServerLogin();
    SYSLOG(L"LanClientMonitoring", LOG_ERROR, L"Connect To Server Success");
}
void CLanClientMonitoring::OnLeaveServer(void)
{
    SYSLOG(L"LanClientMonitoring", LOG_ERROR, L"Disconnect From Server");
}

void CLanClientMonitoring::OnRecv(Packet *p_Packet)
{
    // 받을게 없음.
    SYSLOG(L"LanClientMonitoring", LOG_ERROR, L"Packet Received");
}
void CLanClientMonitoring::OnSend(int SendSize)
{

}

void CLanClientMonitoring::OnWorkerThreadBegin(void)
{

}
void CLanClientMonitoring::OnWorkerThreadEnd(void)
{

}

void CLanClientMonitoring::OnError(int ErrorCode, WCHAR *ErrorMessage)
{

}