#ifndef __LAN_CLIENT_AGENT_HEADER__
#define __LAN_CLIENT_AGENT_HEADER__

class CLanClientAgent : public CLanClient
{
public:
    CLanClientAgent(CGameServer *p_GameServer);
    virtual ~CLanClientAgent(void);

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