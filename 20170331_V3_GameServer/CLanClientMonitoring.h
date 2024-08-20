#ifndef __LAN_CLIENT_MONITORING_HEADER__
#define __LAN_CLIENT_MONITORING_HEADER__

class CLanClientMonitoring : public CLanClient
{
public:
    CLanClientMonitoring(CGameServer *p_GameServer);
    virtual ~CLanClientMonitoring(void);

    void SendMonitoringServerLogin(void);
    void SendMonitoringDataUpdate(void);

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