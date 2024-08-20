#ifndef __GAME_PATH_FINDER_HEADER__
#define __GAME_PATH_FINDER_HEADER__

class CGamePathFinder : public CJPS
{
private:
    CMAP *_Map;
    int _Width;
    int _Height;

    int _BufMaxSize;
    int *_XBuf;
    int *_YBuf;

    int _MapStartX;
    int _MapStartY;

public:
    CGamePathFinder(CMAP *p_Map, int BufMaxSize)
    {
        _Map = p_Map;
        _Width = _Map->GetWidth();
        _Height = _Map->GetHeight();

        _BufMaxSize = BufMaxSize;
        _XBuf = new int[BufMaxSize];
        _YBuf = new int[BufMaxSize];
    }
    virtual ~CGamePathFinder(void)
    {
        // Map은 밖에서 준 것이므로 건드리지 않는다.
        delete[] _XBuf;
        delete[] _YBuf;
    }

    //int Search(int StartTileX, int StartTileY, int EndTileX, int EndTileY, st_PATH *PathBuf)
    int Search(int StartTileX, int StartTileY, int EndTileX, int EndTileY, st_TILE_PATH *PathBuf)
    {
        if (false == CheckMapAttribute(StartTileX, StartTileY))
            return 0;       // 에러가 났다면 못찾은 걸로 한다.
        if (false == CheckMapAttribute(EndTileX, EndTileY))
            return 0;       // 에러가 났다면 못찾은 걸로 한다.
        if (StartTileX == EndTileX && StartTileY == EndTileY)
            return 0;
        //if (abs(StartTileX - EndTileX) > SEARCH_RANGE_MAX_X || abs(StartTileY - EndTileY) > SEARCH_RANGE_MAX_Y)
        //    return 0;

        int SearchSize;
        //SearchSize = SimpleSearch(StartTileX, StartTileY, EndTileX, EndTileY, PathBuf);
        if (StartTileX == EndTileX)
            SearchSize = SearchY(StartTileX, StartTileY, EndTileX, EndTileY, &PathBuf[0]);
        else if (StartTileY == EndTileY)
            SearchSize = SearchX(StartTileX, StartTileY, EndTileX, EndTileY, &PathBuf[0]);
        else
            SearchSize = SearchMixed(StartTileX, StartTileY, EndTileX, EndTileY, PathBuf);
        if (SearchSize > 0)
            return SearchSize;

        // StartTile과 EndTile간의 거리에 따라 시작점을 정한다.
        int DistX;
        int DistY;
        int MapWidth;
        int MapHeight;
        int StartX;
        int StartY;
        int EndX;
        int EndY;
        int Cnt;

        DistX = abs(StartTileX - EndTileX) + 10;
        DistY = abs(StartTileY - EndTileY) + 10;
        _MapStartX = StartTileX - DistX;
        _MapStartY = StartTileY - DistY;
        MapWidth = DistX * 2 + 1;
        MapHeight = DistY * 2 + 1;
        //StartX = StartTileX - _MapStartX;
        //StartY = StartTileY - _MapStartY;
        StartX = DistX;
        StartY = DistY;
        EndX = EndTileX - _MapStartX;
        EndY = EndTileY - _MapStartY;

        int PathSize;
        //int Cnt;

        //PathSize = FindPath(_Width, _Height, StartTileX, StartTileY, EndTileX, EndTileY, _XBuf, _YBuf, _BufMaxSize);
        PathSize = FindPath(MapWidth, MapHeight, StartX, StartY, EndX, EndY, _XBuf, _YBuf, _BufMaxSize);

        if (PathSize < 0)
        {
            return 0;       // 에러가 났다면 못찾은 걸로 한다.
        }

        for (Cnt = 0; Cnt < PathSize; ++Cnt)
        {
            //PathBuf[Cnt].X = TILE_to_POS_X(_XBuf[Cnt] + _MapStartX);
            //PathBuf[Cnt].Y = TILE_to_POS_Y(_YBuf[Cnt] + _MapStartY);

            PathBuf[Cnt].X = _XBuf[Cnt] + _MapStartX;
            PathBuf[Cnt].Y = _YBuf[Cnt] + _MapStartY;
        }

        return PathSize;
    }

    ////int SimpleSearch(int StartTileX, int StartTileY, int EndTileX, int EndTileY, st_PATH *PathBuf)
    //int SimpleSearch(int StartTileX, int StartTileY, int EndTileX, int EndTileY, st_TILE_PATH *PathBuf)
    //{
    //    if (false == CheckMapAttribute(StartTileX, StartTileY))
    //        return 0;       // 에러가 났다면 못찾은 걸로 한다.
    //    if (false == CheckMapAttribute(EndTileX, EndTileY))
    //        return 0;       // 에러가 났다면 못찾은 걸로 한다.
    //    if (StartTileX == EndTileX && StartTileY == EndTileY)
    //        return 0;
    //
    //    if (StartTileX == EndTileX)
    //        return SearchY(StartTileX, StartTileY, EndTileX, EndTileY, &PathBuf[0]);
    //    else if (StartTileY == EndTileY)
    //        return SearchX(StartTileX, StartTileY, EndTileX, EndTileY, &PathBuf[0]);
    //    else
    //        return SearchMixed(StartTileX, StartTileY, EndTileX, EndTileY, PathBuf);
    //}

    bool CheckMapAttribute(int X, int Y)
    {
        if (X < 0 || X >= _Map->GetWidth() || Y < 0 || Y >= _Map->GetHeight())      // 안에서도 체크하는데 일단 넣는다.
            return false;

        MAP_ATTRIBUTE MapAttribute;
        MapAttribute = _Map->GetAttribute(X, Y);
        if (MAP_ATTRIBUTE_DEFAULT == MapAttribute || MAP_ATTRIBUTE_WALL == MapAttribute)
            return false;
        return true;
    }

protected:
    virtual bool CheckMapAttributeJPS(int X, int Y)
    {
        int MapTileX = X + _MapStartX;
        int MapTileY = Y + _MapStartY;

        if (MapTileX < 0 || MapTileX >= _Map->GetWidth() || MapTileY < 0 || MapTileY >= _Map->GetHeight())      // 안에서도 체크하는데 일단 넣는다.
            return false;

        MAP_ATTRIBUTE MapAttribute;
        MapAttribute = _Map->GetAttribute(MapTileX, MapTileY);
        if (MAP_ATTRIBUTE_DEFAULT == MapAttribute || MAP_ATTRIBUTE_WALL == MapAttribute)
            return false;
        return true;
    }

    virtual void OnCheckDirection(int X, int Y)
    {
        //MapColor = rand() % 250 + 4;              // 출력용 맵칼라 설정.
    }
    virtual void OnJump(int X, int Y)
    {
        //_Map->SetAttribute(X, Y, MapColor);       // 타일에 칼라값 박기
    }

private:
    //int SearchY(const int StartTileX, const int StartTileY, const int EndTileX, const int EndTileY, st_PATH *PathBuf)
    int SearchY(const int StartTileX, const int StartTileY, const int EndTileX, const int EndTileY, st_TILE_PATH *PathBuf)
    {
        int CntY;
        int DistY;
        int DirY;
        int PathX, PathY;

        PathX = StartTileX;
        PathY = StartTileY;

        DistY = abs(EndTileY - StartTileY);
        DirY = (EndTileY - StartTileY) / DistY;
        if (DirY != 1 && DirY != -1)
        {
            CrashDump::Crash();
            return 0;
        }

        for (CntY = 0; CntY < DistY; ++CntY)
        {
            PathY += DirY;
            if (false == CheckMapAttribute(PathX, PathY))
                return 0;       // 에러가 났다면 못찾은 걸로 한다.
            if (PathY == EndTileY)
            {
                //PathBuf->X = TILE_to_POS_X(PathX);
                //PathBuf->Y = TILE_to_POS_Y(PathY);

                PathBuf->X = PathX;
                PathBuf->Y = PathY;
                return 1;
            }
        }
        return 0;
    }
    //int SearchX(const int StartTileX, const int StartTileY, const int EndTileX, const int EndTileY, st_PATH *PathBuf)
    int SearchX(const int StartTileX, const int StartTileY, const int EndTileX, const int EndTileY, st_TILE_PATH *PathBuf)
    {
        int CntX;
        int DistX;
        int DirX;
        int PathX, PathY;

        PathX = StartTileX;
        PathY = StartTileY;

        DistX = abs(EndTileX - StartTileX);
        DirX = (EndTileX - StartTileX) / DistX;
        if (DirX != 1 && DirX != -1)
        {
            CrashDump::Crash();
        }

        for (CntX = 0; CntX < DistX; ++CntX)
        {
            PathX += DirX;
            if (false == CheckMapAttribute(PathX, PathY))
                return 0;       // 에러가 났다면 못찾은 걸로 한다.
            if (PathX == EndTileX)
            {
                //PathBuf->X = TILE_to_POS_X(PathX);
                //PathBuf->Y = TILE_to_POS_Y(PathY);

                PathBuf->X = PathX;
                PathBuf->Y = PathY;
                return 1;
            }
        }
        return 0;
    }
    //int SearchDiagonal(const int StartTileX, const int StartTileY, const int EndTileX, const int EndTileY, st_PATH *PathBuf)
    int SearchDiagonal(const int StartTileX, const int StartTileY, const int EndTileX, const int EndTileY, st_TILE_PATH *PathBuf)
    {
        int CntX;
        int DistX, DistY;
        int DirX, DirY;
        int PathX, PathY;

        PathX = StartTileX;
        PathY = StartTileY;

        DistX = abs(EndTileX - StartTileX);
        DistY = abs(EndTileY - StartTileY);

        DirX = (EndTileX - StartTileX) / DistX;
        DirY = (EndTileY - StartTileY) / DistY;
        if (DirX != 1 && DirX != -1 && DirY != 1 && DirY != -1)
        {
            CrashDump::Crash();
        }

        for (CntX = 0; CntX < DistX; ++CntX)
        {
            PathX += DirX;
            PathY += DirY;
            if (false == CheckMapAttribute(PathX, PathY))
                return 0;       // 에러가 났다면 못찾은 걸로 한다.
            if (PathX == EndTileX && PathY == EndTileY)
            {
                //PathBuf->X = TILE_to_POS_X(PathX);
                //PathBuf->Y = TILE_to_POS_Y(PathY);

                PathBuf->X = PathX;
                PathBuf->Y = PathY;
                return 1;
            }
        }
        return 0;
    }
    //int SearchMixed(const int StartTileX, const int StartTileY, const int EndTileX, const int EndTileY, st_PATH *PathBuf)
    int SearchMixed(const int StartTileX, const int StartTileY, const int EndTileX, const int EndTileY, st_TILE_PATH *PathBuf)
    {
        int CntX, CntY;
        int DistX, DistY;
        int DirX, DirY;
        int PathX, PathY;
        int DistDgnlX, DistDgnlY;

        PathX = StartTileX;
        PathY = StartTileY;

        DistX = abs(EndTileX - StartTileX);
        DistY = abs(EndTileY - StartTileY);

        DirX = (EndTileX - StartTileX) / DistX;
        DirY = (EndTileY - StartTileY) / DistY;
        if (DirX != 1 && DirX != -1 && DirY != 1 && DirY != -1)
        {
            CrashDump::Crash();
        }

        if (DistX > DistY)
        {
            if (DistY > DistX - DistY)
            {
                // 대각선 우선 진행
                for (CntX = 0; CntX < DistY; ++CntX)
                {
                    PathX += DirX;
                    PathY += DirY;
                    if (false == CheckMapAttribute(PathX, PathY))
                        return 0;       // 에러가 났다면 못찾은 걸로 한다.
                }

                PathBuf[0].X = PathX;
                PathBuf[0].Y = PathY;

                for (CntX = 0; CntX < DistX - DistY; ++CntX)
                {
                    PathX += DirX;
                    if (false == CheckMapAttribute(PathX, PathY))
                        return 0;       // 에러가 났다면 못찾은 걸로 한다.
                    if (PathX == EndTileX && PathY == EndTileY)
                    {
                        //PathBuf[1].X = TILE_to_POS_X(PathX);
                        //PathBuf[1].Y = TILE_to_POS_Y(PathY);

                        PathBuf[1].X = PathX;
                        PathBuf[1].Y = PathY;
                        return 2;
                    }
                }
            }
            else
            {
                // 직선 우선 진행
                for (CntX = 0; CntX < DistX - DistY; ++CntX)
                {
                    PathX += DirX;
                    if (false == CheckMapAttribute(PathX, PathY))
                        return 0;       // 에러가 났다면 못찾은 걸로 한다.
                }
                //PathBuf[0].X = TILE_to_POS_X(PathX);
                //PathBuf[0].Y = TILE_to_POS_Y(PathY);

                PathBuf[0].X = PathX;
                PathBuf[0].Y = PathY;

                DistDgnlX = abs(EndTileX - PathX);
                DistDgnlY = abs(EndTileY - PathY);
                if (DistDgnlX != DistDgnlY)
                    CrashDump::Crash();

                DirX = (EndTileX - PathX) / DistDgnlX;
                DirY = (EndTileY - PathY) / DistDgnlY;

                for (CntX = 0; CntX < DistDgnlX; ++CntX)
                {
                    PathX += DirX;
                    PathY += DirY;
                    if (false == CheckMapAttribute(PathX, PathY))
                        return 0;       // 에러가 났다면 못찾은 걸로 한다.
                    if (PathX == EndTileX && PathY == EndTileY)
                    {
                        //PathBuf[1].X = TILE_to_POS_X(PathX);
                        //PathBuf[1].Y = TILE_to_POS_Y(PathY);

                        PathBuf[1].X = PathX;
                        PathBuf[1].Y = PathY;
                        return 2;
                    }
                }
            }

            return 0;
        }
        else if (DistX < DistY)
        {
            if (DistX > DistY - DistX)
            {
                // 대각선 우선 진행
                for (CntX = 0; CntX < DistX; ++CntX)
                {
                    PathX += DirX;
                    PathY += DirY;
                    if (false == CheckMapAttribute(PathX, PathY))
                        return 0;       // 에러가 났다면 못찾은 걸로 한다.
                }

                PathBuf[0].X = PathX;
                PathBuf[0].Y = PathY;

                for (CntY = 0; CntY < DistY - DistX; ++CntY)
                {
                    PathY += DirY;
                    if (false == CheckMapAttribute(PathX, PathY))
                        return 0;       // 에러가 났다면 못찾은 걸로 한다.
                    if (PathX == EndTileX && PathY == EndTileY)
                    {
                        PathBuf[1].X = PathX;
                        PathBuf[1].Y = PathY;
                        return 2;
                    }
                }
            }
            else
            {
                // 직선 우선 진행
                for (CntY = 0; CntY < DistY - DistX; ++CntY)
                {
                    PathY += DirY;
                    if (false == CheckMapAttribute(PathX, PathY))
                        return 0;       // 에러가 났다면 못찾은 걸로 한다.
                }
                //PathBuf[0].X = TILE_to_POS_X(PathX);
                //PathBuf[0].Y = TILE_to_POS_Y(PathY);

                PathBuf[0].X = PathX;
                PathBuf[0].Y = PathY;

                DistDgnlX = abs(EndTileX - PathX);
                DistDgnlY = abs(EndTileY - PathY);
                if (DistDgnlX != DistDgnlY)
                    CrashDump::Crash();

                DirX = (EndTileX - PathX) / DistDgnlX;
                DirY = (EndTileY - PathY) / DistDgnlY;

                for (CntX = 0; CntX < DistDgnlX; ++CntX)
                {
                    PathX += DirX;
                    PathY += DirY;
                    if (false == CheckMapAttribute(PathX, PathY))
                        return 0;       // 에러가 났다면 못찾은 걸로 한다.
                    if (PathX == EndTileX && PathY == EndTileY)
                    {
                        //PathBuf[1].X = TILE_to_POS_X(PathX);
                        //PathBuf[1].Y = TILE_to_POS_Y(PathY);

                        PathBuf[1].X = PathX;
                        PathBuf[1].Y = PathY;
                        return 2;
                    }
                }
            }
            
            return 0;
        }
        else
        {
            //if (DistX == DistY)
            return SearchDiagonal(StartTileX, StartTileY, EndTileX, EndTileY, &PathBuf[0]);
        }

        //if (DistX > DistY)
        //{
        //    for (CntX = 0; CntX < DistX - DistY; ++CntX)
        //    {
        //        PathX += DirX;
        //        if (false == CheckMapAttribute(PathX, PathY))
        //            return 0;       // 에러가 났다면 못찾은 걸로 한다.
        //    }
        //    //PathBuf[0].X = TILE_to_POS_X(PathX);
        //    //PathBuf[0].Y = TILE_to_POS_Y(PathY);
        //
        //    PathBuf[0].X = PathX;
        //    PathBuf[0].Y = PathY;
        //
        //    DistDgnlX = abs(EndTileX - PathX);
        //    DistDgnlY = abs(EndTileY - PathY);
        //    if (DistDgnlX != DistDgnlY)
        //        CrashDump::Crash();
        //
        //    DirX = (EndTileX - PathX) / DistDgnlX;
        //    DirY = (EndTileY - PathY) / DistDgnlY;
        //
        //    for (CntX = 0; CntX < DistDgnlX; ++CntX)
        //    {
        //        PathX += DirX;
        //        PathY += DirY;
        //        if (false == CheckMapAttribute(PathX, PathY))
        //            return 0;       // 에러가 났다면 못찾은 걸로 한다.
        //        if (PathX == EndTileX && PathY == EndTileY)
        //        {
        //            //PathBuf[1].X = TILE_to_POS_X(PathX);
        //            //PathBuf[1].Y = TILE_to_POS_Y(PathY);
        //
        //            PathBuf[1].X = PathX;
        //            PathBuf[1].Y = PathY;
        //            return 2;
        //        }
        //    }
        //    return 0;
        //}
        //else if (DistX < DistY)
        //{
        //    for (CntY = 0; CntY < DistY - DistX; ++CntY)
        //    {
        //        PathY += DirY;
        //        if (false == CheckMapAttribute(PathX, PathY))
        //            return 0;       // 에러가 났다면 못찾은 걸로 한다.
        //    }
        //    //PathBuf[0].X = TILE_to_POS_X(PathX);
        //    //PathBuf[0].Y = TILE_to_POS_Y(PathY);
        //
        //    PathBuf[0].X = PathX;
        //    PathBuf[0].Y = PathY;
        //
        //    DistDgnlX = abs(EndTileX - PathX);
        //    DistDgnlY = abs(EndTileY - PathY);
        //    if (DistDgnlX != DistDgnlY)
        //        CrashDump::Crash();
        //
        //    DirX = (EndTileX - PathX) / DistDgnlX;
        //    DirY = (EndTileY - PathY) / DistDgnlY;
        //
        //    for (CntX = 0; CntX < DistDgnlX; ++CntX)
        //    {
        //        PathX += DirX;
        //        PathY += DirY;
        //        if (false == CheckMapAttribute(PathX, PathY))
        //            return 0;       // 에러가 났다면 못찾은 걸로 한다.
        //        if (PathX == EndTileX && PathY == EndTileY)
        //        {
        //            //PathBuf[1].X = TILE_to_POS_X(PathX);
        //            //PathBuf[1].Y = TILE_to_POS_Y(PathY);
        //
        //            PathBuf[1].X = PathX;
        //            PathBuf[1].Y = PathY;
        //            return 2;
        //        }
        //    }
        //    return 0;
        //}
        //else
        //{
        //    //if (DistX == DistY)
        //    return SearchDiagonal(StartTileX, StartTileY, EndTileX, EndTileY, &PathBuf[0]);
        //}
    }
};

#endif