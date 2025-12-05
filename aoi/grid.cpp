#include "grid.h"
#include <cmath>

namespace aoi {

Grid::Grid(int id) : m_gridId(id) {}

Grid::~Grid() { m_objs.clear(); }

void Grid::addObjs(TowerObj* obj) {
    if (hasObjs(obj)) {
        return;
    }
    m_objs[obj->id()] = obj;
}

void Grid::removeObjs(TowerObj* obj) {
    if (!hasObjs(obj)) {
        return;
    }
    m_objs.erase(obj->id());
}

bool Grid::hasObjs(TowerObj* obj) {
    auto itObjs = m_objs.find(obj->id());
    if (itObjs == m_objs.end()) {
        return false;
    }
    return true;
}

GridAOIManger::GridAOIManger(int minX, int minY, int maxX, int maxY, int xSize, int ySize)
    : m_minX(minX), m_minY(minY), m_maxX(maxX), m_maxY(maxY), m_xSize(xSize), m_ySize(ySize) {
    initGrid();
}

GridAOIManger::~GridAOIManger() {
    for (auto& it : m_grids) {
        if (!it.second) {
            continue;
        }
        delete it.second;
        it.second = nullptr;
    }
    m_grids.clear();
}

void GridAOIManger::enter(TowerObj* obj) {
    auto itGrids = m_grids.find(gridId(obj->x(), obj->y()));
    if (itGrids == m_grids.end()) {
        return;
    }
    itGrids->second->addObjs(obj);
    std::vector<TowerObj*> objs;
    if (!visitWatchedGridObjs(obj, objs)) {
        return;
    }
    for (auto it : objs) {
        it->onEnter({obj});
    }
    if (objs.size() == 0) {
        return;
    }
    obj->onEnter(objs);
}

void GridAOIManger::moved(TowerObj* obj, int x, int y) {
    if (obj->x() == x && obj->y() == y) {
        return;
    }

    std::unordered_map<uint64_t, TowerObj*> oldObjs;
    visitWatchedGridObjs(obj, oldObjs);
    int oldGridid = gridId(obj->x(), obj->y());
    auto itOldGrids = m_grids.find(oldGridid);
    if (itOldGrids == m_grids.end()) {
        return;
    }
    itOldGrids->second->removeObjs(obj);

    std::unordered_map<uint64_t, TowerObj*> newObjs;
    obj->x(x);
    obj->y(y);
    visitWatchedGridObjs(obj, newObjs);
    int newGridID = gridId(obj->x(), obj->y());
    auto itNewGrids = m_grids.find(newGridID);
    if (itNewGrids == m_grids.end()) {
        return;
    }
    itNewGrids->second->addObjs(obj);

    std::vector<TowerObj*> lostGrids;
    for (auto& itObjs : oldObjs) {
        auto itNewObjs = newObjs.find(itObjs.second->id());
        if (itNewObjs != newObjs.end()) {
            continue;
        }
        itObjs.second->onLeave({obj});
        lostGrids.emplace_back(itObjs.second);
    }
    if (!lostGrids.empty()) {
        obj->onLeave({lostGrids});
    }
    std::vector<TowerObj*> bornGrids;
    for (auto& itObjs : newObjs) {
        auto itOldObjs = oldObjs.find(itObjs.second->id());
        if (itOldObjs != oldObjs.end()) {
            continue;
        }
        itObjs.second->onEnter({obj});
        bornGrids.emplace_back(itObjs.second);
    }
    if (!bornGrids.empty()) {
        obj->onEnter({bornGrids});
    }
}

void GridAOIManger::leave(TowerObj* obj) {
    auto itGrids = m_grids.find(gridId(obj->x(), obj->y()));
    if (itGrids == m_grids.end()) {
        return;
    }
    itGrids->second->removeObjs(obj);
    std::vector<TowerObj*> objs;
    if (!visitWatchedGridObjs(obj, objs)) {
        return;
    }
    for (auto it : objs) {
        it->onLeave({obj});
    }
    obj->onLeave(objs);
}

int GridAOIManger::gridId(int x, int y) { return transY(y) * m_xNum + transX(x); }

bool GridAOIManger::visitWatchedGridObjs(TowerObj* obj, std::unordered_map<uint64_t, TowerObj*>& objs) {
    if (!visitWatchedGridObjs(obj, [&](Grid* gridObj) {
            for (auto& itGridObj : gridObj->m_objs) {
                if (itGridObj.second->id() == obj->id()) {
                    continue;
                }
                objs[itGridObj.second->id()] = itGridObj.second;
            }
        })) {
        return false;
    }
    return objs.size() > 0;
}

bool GridAOIManger::visitWatchedGridObjs(TowerObj* obj, std::vector<TowerObj*>& objs) {
    if (!visitWatchedGridObjs(obj, [&](Grid* gridObj) {
            for (auto& itGridObj : gridObj->m_objs) {
                if (itGridObj.second->id() == obj->id()) {
                    continue;
                }
                objs.emplace_back(itGridObj.second);
            }
        })) {
        return false;
    }
    return objs.size() > 0;
}

void GridAOIManger::initGrid() {
    m_yNum = transY(m_maxY);
    m_xNum = transY(m_maxX);
    for (int y = 0; y < m_yNum; y++) {
        for (int x = 0; x < m_xNum; x++) {
            int gid = y * m_xNum + x;
            m_grids[gid] = new Grid(gid);
        }
    }
}

int GridAOIManger::transX(int x) {
    int tx = (int)(std::ceil(x - m_minX) / (double)m_xSize);
    if (tx < 0) {
        tx = 0;
    }
    return tx;
}

int GridAOIManger::transY(int y) {
    int ty = (int)(std::ceil(y - m_minY) / (double)m_ySize);
    if (ty < 0) {
        ty = 0;
    }
    return ty;
}

} // namespace aoi
