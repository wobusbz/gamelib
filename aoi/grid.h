#ifndef __GRID_H__
#define __GRID_H__

#include <functional>
#include <unordered_map>
#include <vector>

#include "object_aoi.h"

namespace aoi {

class Grid {
    friend class GridAOIMannger;
    Grid(int id);
    ~Grid();

public:
    void addObjs(TowerObj* obj);
    void removeObjs(TowerObj* obj);
    bool hasObjs(TowerObj* obj);

private:
    int m_gridId;
    std::unordered_map<uint64_t, TowerObj*> m_objs;
};

class GridAOIMannger {
public:
    GridAOIMannger(int minX, int minY, int maxX, int maxY, int xSize, int ySize);
    ~GridAOIMannger();
    void enter(TowerObj* obj);
    void moved(TowerObj* obj, int x, int y);
    void leave(TowerObj* obj);

    int gridId(int x, int y);
    bool visitWatchedGridObjs(TowerObj* obj, std::function<void(Grid*)>);
    bool visitWatchedGridObjs(TowerObj* obj, std::unordered_map<uint64_t, TowerObj*>& objs);
    bool visitWatchedGridObjs(TowerObj* obj, std::vector<TowerObj*>& objs);

private:
    void initGrid();
    int transX(int x);
    int transY(int y);

    std::unordered_map<int, Grid*> m_grids;
    int m_minX;
    int m_minY;
    int m_maxX;
    int m_maxY;
    int m_xSize;
    int m_ySize;
    int m_xNum;
    int m_yNum;
};

} // namespace aoi

#endif // !__GRID_H__
