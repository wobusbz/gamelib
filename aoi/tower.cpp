#include "tower.h"
#include <vector>

aoi::TowerObj::TowerObj() : m_x(0), m_y(0), m_z(0), m_dist(0) {}

aoi::TowerObj::~TowerObj() {}

aoi::TowerAOIMannger::TowerAOIMannger(int minX, int maxX, int minY, int maxY, int range)
    : m_minX(minX), m_maxX(maxX), m_minY(minY), m_maxY(maxY), m_range(range), m_xTowerNum(0), m_yTowerNum(0) {
    initialize();
}

aoi::TowerAOIMannger::~TowerAOIMannger() {
    for (auto& it : m_towers) {
        for (auto& itr : it) {
            if (!itr) {
                continue;
            }
            delete itr;
            itr = nullptr;
        }
    }
    m_towers.clear();
}

int aoi::TowerAOIMannger::transX(int x) const {
    int tx = (x - m_minX) / m_range;
    if (tx < 0) {
        tx = 0;
    } else if (tx >= m_xTowerNum) {
        tx = m_xTowerNum - 1;
    }
    return tx;
}

int aoi::TowerAOIMannger::transY(int y) const {
    int ty = (y - m_minY) / m_range;
    if (ty < 0) {
        ty = 0;
    } else if (ty >= m_yTowerNum) {
        ty = m_yTowerNum - 1;
    }
    return ty;
}

aoi::TowerAOI* aoi::TowerAOIMannger::getTower(int x, int y) { return m_towers[transX(x)][transY(y)]; }

std::unordered_set<aoi::TowerAOI*> aoi::TowerAOIMannger::getWatchedTowers(int x, int y, int range) const {
    std::unordered_set<aoi::TowerAOI*> towers;
    if (range < 0) {
        return towers;
    }
    int minX = transX(x - range);
    int maxX = transX(x + range);
    int minY = transX(y - range);
    int maxY = transX(y + range);
    for (int xi = minX; xi <= maxX; xi++) {
        for (int yj = minY; yj <= maxY; yj++) {
            towers.insert(m_towers[xi][yj]);
        }
    }
    return towers;
}

void aoi::TowerAOIMannger::initialize() {
    m_xTowerNum = (m_maxX - m_minX) / m_range + 1;
    m_yTowerNum = (m_maxY - m_minY) / m_range + 1;
    m_towers.resize(m_xTowerNum);
    for (int i = 0; i < m_xTowerNum; i++) {
        m_towers[i].resize(m_yTowerNum);
        for (int j = 0; j < m_yTowerNum; j++) {
            m_towers[i][j] = new TowerAOI();
        }
    }
}

void aoi::TowerAOIMannger::enter(TowerObj* obj) {
    if (!obj) {
        return;
    }

    visitWatchedTowersByWorld(obj->x(), obj->y(), obj->dist(),
                              [&](aoi::TowerAOI* objTowerAOI) { objTowerAOI->addWatchersObj(obj); });

    getTower(obj->x(), obj->y())->addObjs(obj);
}

void aoi::TowerAOIMannger::moved(TowerObj* obj, int x, int y) {
    if (obj->x() == x && obj->y() == y) {
        return;
    }

    auto oldTower = getWatchedTowers(obj->x(), obj->y(), obj->dist());
    auto newTower = getWatchedTowers(x, y, obj->dist());

    std::vector<TowerAOI*> towersToLevels;
    towersToLevels.resize(oldTower.size());
    for (auto tower : oldTower) {
        if (newTower.find(tower) != newTower.end()) {
            continue;
        }
        towersToLevels.emplace_back(tower);
    }

    std::vector<TowerAOI*> towersToEnters;
    towersToLevels.resize(oldTower.size());
    for (auto tower : newTower) {
        if (oldTower.find(tower) != oldTower.end()) {
            continue;
        }
        towersToEnters.emplace_back(tower);
    }

    for (auto tower : towersToLevels) {
        tower->removeWatchersObj(obj);
    }

    for (auto tower : towersToEnters) {
        tower->addWatchersObj(obj);
    }

    obj->x(x);
    obj->y(y);

    auto newObjTower = getTower(x, y);
    if (obj->m_towerAOI == newObjTower) {
        return;
    }
    if (obj->m_towerAOI) {
        obj->m_towerAOI->removeObjs(obj);
    }
    newObjTower->addObjs(obj);
}

void aoi::TowerAOIMannger::leave(TowerObj* obj) {
    if (obj->m_towerAOI) {
        obj->m_towerAOI->removeObjs(obj, true);
    }
    visitWatchedTowersByWorld(obj->x(), obj->y(), obj->dist(),
                              [&](aoi::TowerAOI* objTowerAOI) { objTowerAOI->removeWatchersObj(obj); });
}

void aoi::TowerAOI::addObjs(TowerObj* obj, TowerAOI* fromObjs) {
    obj->m_towerAOI = this;
    auto itObjs = m_objs.find(obj->id());
    if (itObjs == m_objs.end()) {
        m_objs[obj->id()] = obj;
    }
    if (!fromObjs) {
        for (auto& it : m_watchers) {
            if (!it.second) {
                continue;
            }
            if (it.second->id() == obj->id()) {
                continue;
            }
            if (dist2(it.second, obj) > it.second->dist() * it.second->dist()) {
                continue;
            }
            it.second->onEnter({obj});
        }
    } else {
        for (auto& it : fromObjs->m_watchers) {
            if (!it.second) {
                continue;
            }
            if (it.second->id() == obj->id()) {
                continue;
            }
            auto itWatchers = m_watchers.find(it.second->id());
            if (itWatchers != m_watchers.end()) {
                continue;
            }
            if (dist2(it.second, obj) <= it.second->dist() * it.second->dist()) {
                continue;
            }
            it.second->onLeave({obj});
        }
        for (auto& it : m_watchers) {
            if (!it.second) {
                continue;
            }
            if (it.second->id() == obj->id()) {
                continue;
            }
            auto itWatchers = fromObjs->m_watchers.find(it.second->id());
            if (itWatchers != fromObjs->m_watchers.end()) {
                continue;
            }
            if (dist2(it.second, obj) > it.second->dist() * it.second->dist()) {
                continue;
            }
            it.second->onEnter({obj});
        }
    }
}

void aoi::TowerAOI::addWatchersObj(TowerObj* obj) {
    if (!obj) {
        return;
    }
    auto itWatchers = m_watchers.find(obj->id());
    if (itWatchers == m_watchers.end()) {
        m_watchers[obj->id()] = obj;
    }
    std::vector<TowerObj*> objs(m_objs.size());
    int r2 = obj->dist() * obj->dist();
    for (auto& it : m_objs) {
        if (!it.second) {
            continue;
        }
        if (it.second->id() == obj->id()) {
            continue;
        }
        if (dist2(obj, it.second) > r2) {
            continue;
        }
        objs.emplace_back(it.second);
    }
    if (objs.empty()) {
        return;
    }
    obj->onEnter(objs);
}

void aoi::TowerAOI::removeObjs(TowerObj* obj, bool notify) {
    m_objs.erase(obj->id());
    if (!notify) {
        return;
    }
    for (auto& it : m_watchers) {
        if (!it.second) {
            continue;
        }
        if (it.second->id() == obj->id()) {
            continue;
        }
        if (dist2(obj, it.second) > it.second->dist() * it.second->dist()) {
            continue;
        }
        it.second->onLeave({obj});
    }
}

void aoi::TowerAOI::removeWatchersObj(TowerObj* obj) {
    if (!obj) {
        return;
    }
    m_watchers.erase(obj->id());
    std::vector<TowerObj*> objs(m_objs.size());
    int r2 = obj->dist() * obj->dist();
    for (auto& it : m_objs) {
        if (it.second->id() == obj->id()) {
            continue;
        }
        if (dist2(obj, it.second) > r2) {
            continue;
        }
        objs.emplace_back(it.second);
    }
    if (objs.empty()) {
        return;
    }
    obj->onLeave(objs);
}
