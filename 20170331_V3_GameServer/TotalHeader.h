#ifndef __TOTAL_HEADER__
#define __TOTAL_HEADER__

//#include "LibHeader.h"
//
//using namespace MonLib;
#include "GameServerDefine.h"
#include "AttackPattern.h"
struct st_PATH
{
    float X;
    float Y;
};
struct st_TILE_PATH
{
    int X;
    int Y;
};
enum GAME_PATHFINDING_RANGE_DEFINE
{
    SEARCH_RANGE_MAX_X = 80,           // 섹터 두개 크기.
    SEARCH_RANGE_MAX_Y = 80
};
#include "CGamePathFinder.h"
#include "GameDBDefine.h"
#include "CDBLog.h"
#include "CDBAccount.h"
#include "CDBGame.h"
//#include "CDBLogTLS.h"
//#include "CDBAccountTLS.h"
//#include "CDBGameTLS.h"

class CGameServer;
#include "CLanClientLogin.h"
#include "CLanClientMonitoring.h"
#include "CLanClientAgent.h"

class Player;
#include "GameContentsDefine.h"
#include "CPlayer.h"
#include "CGameServer.h"
#include "CGameServerConfig.h"

extern CGameServerConfig g_GameServerConfig;

#endif