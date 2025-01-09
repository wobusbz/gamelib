#include "grid.h"

aoi::Grid::Grid(int id) : m_gridId(id) {}

aoi::Grid::~Grid() { m_objs.clear(); }

void aoi::Grid::addObjs(TowerObj* obj) {
    if (hasObjs(obj)) {
        return;
    }
    m_objs[obj->id()] = obj;
}

void aoi::Grid::removeObjs(TowerObj* obj) {
    if (!hasObjs(obj)) {
        return;
    }
    m_objs.erase(obj->id());
}

bool aoi::Grid::hasObjs(TowerObj* obj) {
    auto itObjs = m_objs.find(obj->id());
    if (itObjs == m_objs.end()) {
        return false;
    }
    return true;
}

aoi::GridAOIMannger::GridAOIMannger(int minX, int minY, int maxX, int maxY, int xSize, int ySize)
    : m_minX(minX), m_minY(minY), m_maxX(maxX), m_maxY(maxY), m_xSize(xSize), m_ySize(ySize) {
    initGrid();
}

aoi::GridAOIMannger::~GridAOIMannger() {
    for (auto& it : m_grids) {
        if (!it.second) {
            continue;
        }
        delete it.second;
        it.second = nullptr;
    }
    m_grids.clear();
}

void aoi::GridAOIMannger::enter(TowerObj* obj) {
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

void aoi::GridAOIMannger::moved(TowerObj* obj, int x, int y) {
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

void aoi::GridAOIMannger::leave(TowerObj* obj) {
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

int aoi::GridAOIMannger::gridId(int x, int y) { return transY(y) * m_xNum + transX(x); }

bool aoi::GridAOIMannger::visitWatchedGridObjs(TowerObj* obj, std::function<void(Grid*)> cbFunc) {
    auto itGrids = m_grids.find(gridId(obj->x(), obj->y()));
    if (itGrids == m_grids.end() || !cbFunc) {
        return false;
    }
    cbFunc(itGrids->second);
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
        cbFunc(itNewGrids->second);
    }
    return true;
}

bool aoi::GridAOIMannger::visitWatchedGridObjs(TowerObj* obj, std::unordered_map<uint64_t, TowerObj*>& objs) {
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

bool aoi::GridAOIMannger::visitWatchedGridObjs(TowerObj* obj, std::vector<TowerObj*>& objs) {
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

void aoi::GridAOIMannger::initGrid() {
    m_yNum = transY(m_maxY);
    m_xNum = transY(m_maxX);
    for (int y = 0; y < m_yNum; y++) {
        for (int x = 0; x < m_xNum; x++) {
            int gid = y * m_xNum + x;
            m_grids[gid] = new Grid(gid);
        }
    }
}

int aoi::GridAOIMannger::transX(int x) {
    int tx = (int)(std::ceil(x - m_minX) / (double)m_xSize);
    if (tx < 0) {
        tx = 0;
    }
    return tx;
}

int aoi::GridAOIMannger::transY(int y) {
    int ty = (int)(std::ceil(y - m_minY) / (double)m_ySize);
    if (ty < 0) {
        ty = 0;
    }
    return ty;
}
