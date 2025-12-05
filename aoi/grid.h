#ifndef __GRID_H__
#define __GRID_H__

#include <functional>
#include <unordered_map>
#include <vector>

#include "object_aoi.h"

namespace aoi {

class Grid {
    friend class GridAOIManger;
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

class GridAOIManger {
public:
    GridAOIManger(int minX, int minY, int maxX, int maxY, int xSize, int ySize);
    ~GridAOIManger();
    void enter(TowerObj* obj);
    void moved(TowerObj* obj, int x, int y);
    void leave(TowerObj* obj);

    int gridId(int x, int y);

    template <typename Function>
    bool visitWatchedGridObjs(TowerObj* obj, Function&& fn) {
        auto itGrids = m_grids.find(gridId(obj->x(), obj->y()));
        if (itGrids == m_grids.end()) {
            return false;
        }
        fn(itGrids->second);
        int x = transX(obj->x());
        int y = transY(obj->y());
        int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        for (int i = 0; i < 8; i++) {
            int newX = x + dx[i];
            int newY = y + dy[i];
            if (newX < 0 || newX >= m_xNum || newY < 0 || newY >= m_yNum) {
                continue;
            }
            auto itNewGrids = m_grids.find(newY * m_xNum + newX);
            if (itNewGrids == m_grids.end()) {
                continue;
            }
            fn(itNewGrids->second);
        }
        return true;
    }
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
