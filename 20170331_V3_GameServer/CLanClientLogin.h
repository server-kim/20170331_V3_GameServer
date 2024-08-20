#ifndef __LAN_CLINET_LOGIN_HEADER__
#define __LAN_CLINET_LOGIN_HEADER__

class CLanClientLogin : public CLanClient
{
public:
    CLanClientLogin(CGameServer *p_GameServer);
    virtual ~CLanClientLogin(void);

    bool PacketProc_ReqNewClientLogin(Packet *p_Packet);
    //void SendResNewClientLogin(__int64 AccountNo);
    void SendResNewClientLogin(__int64 AccountNo, __int64 ClientID);

    void SendLoginServerLogin(void);
    void SendHeartBeat(int ThreadType);

private:
    virtual void OnEnterJoinServer(void);
    virtual void OnLeaveServer(void);

    virtual void OnRecv(Packet *p_Packet);
    virtual void OnSend(int SendSize);

    virtual void OnWorkerThreadBegin(void);
    virtual void OnWorkerThreadEnd(void);

    virtual void OnError(int ErrorCode, WCHAR *ErrorMessage);

private:
    CGameServer     *_p_GameServer;
};

#endif