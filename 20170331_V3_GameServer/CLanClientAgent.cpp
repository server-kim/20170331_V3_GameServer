#include "stdafx.h"
#include "TotalHeader.h"

CLanClientAgent::CLanClientAgent(CGameServer *p_GameServer)
{
    if (nullptr == p_GameServer)
        CrashDump::Crash();
    _p_GameServer = p_GameServer;
}
CLanClientAgent::~CLanClientAgent(void)
{

}

void CLanClientAgent::OnEnterJoinServer(void)
{
    SYSLOG(L"LanClientAgent", LOG_ERROR, L"Connect To Server Success");
}
void CLanClientAgent::OnLeaveServer(void)
{
    SYSLOG(L"LanClientAgent", LOG_ERROR, L"Disconnect From Server");
}

void CLanClientAgent::OnRecv(Packet *p_Packet)
{
    // Shutdown Packet 처리할 것.
    SYSLOG(L"LanClientAgent", LOG_ERROR, L"Packet Received");
}
void CLanClientAgent::OnSend(int SendSize)
{

}

void CLanClientAgent::OnWorkerThreadBegin(void)
{

}
void CLanClientAgent::OnWorkerThreadEnd(void)
{

}

void CLanClientAgent::OnError(int ErrorCode, WCHAR *ErrorMessage)
{

}