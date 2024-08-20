#include "stdafx.h"
#include "TotalHeader.h"

CLanClientLogin::CLanClientLogin(CGameServer *p_GameServer)
{
    if (nullptr == p_GameServer)
        CrashDump::Crash();
    _p_GameServer = p_GameServer;
}
CLanClientLogin::~CLanClientLogin(void)
{

}

bool CLanClientLogin::PacketProc_ReqNewClientLogin(Packet *p_Packet)
{
    __int64 AccountNo;
    BYTE SessionKey[SESSION_KEY_BYTE_LEN];
    __int64 ClientID;

    *p_Packet >> AccountNo;
    p_Packet->GetData((char *)SessionKey, SESSION_KEY_BYTE_LEN);
    *p_Packet >> ClientID;

    _p_GameServer->AddLoginSessionKey(AccountNo, SessionKey);

    SendResNewClientLogin(AccountNo, ClientID);
    return false;
}
void CLanClientLogin::SendResNewClientLogin(__int64 AccountNo, __int64 ClientID)
{
    if (false == IsConnect())
        return;

    Packet *p_Packet = Packet::Alloc();

    *p_Packet << (WORD)en_PACKET_SS_RES_NEW_CLIENT_LOGIN;
    *p_Packet << AccountNo;
    *p_Packet << ClientID;

    SendPacket(p_Packet);
    p_Packet->Free();
}

void CLanClientLogin::SendLoginServerLogin(void)
{
    if (false == IsConnect())
        return;

    Packet *p_Packet = Packet::Alloc();

    *p_Packet << (WORD)en_PACKET_SS_LOGINSERVER_LOGIN;
    *p_Packet << (BYTE)dfSERVER_TYPE_GAME;
    p_Packet->PutData((char *)g_GameServerConfig._ServerName, (SERVER_NAME_LEN + 1) * sizeof(WCHAR));

    SendPacket(p_Packet);
    p_Packet->Free();
}
void CLanClientLogin::SendHeartBeat(int ThreadType)
{
    if (false == IsConnect())
        return;

    Packet *p_Packet = Packet::Alloc();

    *p_Packet << (WORD)en_PACKET_SS_HEARTBEAT;
    *p_Packet << (BYTE)ThreadType;

    SendPacket(p_Packet);
    p_Packet->Free();
}

void CLanClientLogin::OnEnterJoinServer(void)
{
    SendLoginServerLogin();
    SYSLOG(L"LanClientLogin", LOG_ERROR, L"Connect To Server Success");
}
void CLanClientLogin::OnLeaveServer(void)
{
    SYSLOG(L"LanClientLogin", LOG_ERROR, L"Disconnect From Server");
}

void CLanClientLogin::OnRecv(Packet *p_Packet)
{
    if (nullptr == p_Packet)
        CrashDump::Crash();

    WORD Type;
    *p_Packet >> Type;

    switch (Type)
    {
    case en_PACKET_SS_REQ_NEW_CLIENT_LOGIN:
        PacketProc_ReqNewClientLogin(p_Packet);
        break;
    default:
        CrashDump::Crash();
        break;
    }
}
void CLanClientLogin::OnSend(int SendSize)
{

}

void CLanClientLogin::OnWorkerThreadBegin(void)
{

}
void CLanClientLogin::OnWorkerThreadEnd(void)
{

}

void CLanClientLogin::OnError(int ErrorCode, WCHAR *ErrorMessage)
{

}