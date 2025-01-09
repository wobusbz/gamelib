#include "tower.h"

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

void aoi::TowerAOIMannger::visitWatchedTowers(int x, int y, int range, std::function<void(TowerAOI*)> callback) {
    if (!callback) {
        return;
    }
    int minX = transX(x - range);
    int maxX = transX(x + range);
    int minY = transY(y - range);
    int maxY = transY(y + range);
    for (int xi = minX; xi <= maxX; xi++) {
        for (int yi = minY; yi <= maxY; yi++) {
            callback(m_towers[xi][yi]);
        }
    }
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
    visitWatchedTowers(obj->x(), obj->y(), obj->dist(),
                       [&](aoi::TowerAOI* objTowerAOI) { objTowerAOI->addWatchersObj(obj); });
    getTower(obj->x(), obj->y())->addObjs(obj);
}

void aoi::TowerAOIMannger::moved(TowerObj* obj, int x, int y) {
    if (obj->x() == x && obj->y() == y) {
        return;
    }
    auto oldTower = obj->m_towerAOI;
    int oldMinX = transX(obj->x() - obj->dist());
    int oldMaxX = transX(obj->x() + obj->dist());
    int oldMinY = transY(obj->y() - obj->dist());
    int oldMaxY = transY(obj->y() + obj->dist());
    obj->x(x);
    obj->y(y);

    auto newTower = getTower(x, y);
    if (newTower != oldTower) {
        obj->m_towerAOI->removeObjs(obj);
        newTower->addObjs(obj, oldTower);
        obj->m_towerAOI = newTower;
    }

    int newMinX = transX(obj->x() - obj->dist());
    int newMaxX = transX(obj->x() + obj->dist());
    int newMinY = transY(obj->y() - obj->dist());
    int newMaxY = transY(obj->y() + obj->dist());
    for (int xi = oldMinX; xi <= oldMaxX; xi++) {
        for (int yj = oldMinY; yj <= oldMaxY; yj++) {
            if (xi >= newMinX && xi <= newMaxX && yj >= newMinY && yj <= newMaxY) {
                continue;
            }
            m_towers[xi][yj]->removeWatchersObj(obj);
        }
    }
    for (int xi = newMinX; xi <= newMaxX; xi++) {
        for (int yj = newMinY; yj <= newMaxY; yj++) {
            if (xi >= oldMinX && xi <= oldMaxX && yj >= oldMinY && yj <= oldMaxY) {
                continue;
            }
            m_towers[xi][yj]->addWatchersObj(obj);
        }
    }
}

void aoi::TowerAOIMannger::leave(TowerObj* obj) {
    obj->m_towerAOI->removeObjs(obj, true);
    visitWatchedTowers(obj->x(), obj->y(), obj->dist(),
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
            if (it.second->id() == obj->id()) {
                continue;
            }
            it.second->onEnter({obj});
        }
    } else {
        for (auto& it : fromObjs->m_watchers) {
            if (it.second->id() == obj->id()) {
                continue;
            }
            auto itWatchers = m_watchers.find(it.second->id());
            if (itWatchers != m_watchers.end()) {
                continue;
            }
            it.second->onLeave({obj});
        }
        for (auto& it : m_watchers) {
            if (it.second->id() == obj->id()) {
                continue;
            }
            auto itWatchers = fromObjs->m_watchers.find(it.second->id());
            if (itWatchers != fromObjs->m_watchers.end()) {
                continue;
            }
            it.second->onEnter({obj});
        }
    }
}

void aoi::TowerAOI::addWatchersObj(TowerObj* obj) {
    auto itWatchers = m_watchers.find(obj->id());
    if (itWatchers == m_watchers.end()) {
        m_watchers[obj->id()] = obj;
    }
    std::vector<TowerObj*> objs;
    for (auto& it : m_objs) {
        if (it.second->id() == obj->id()) {
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
        if (it.second->id() == obj->id()) {
            continue;
        }
        it.second->onLeave({obj});
    }
}

void aoi::TowerAOI::removeWatchersObj(TowerObj* obj) {
    m_watchers.erase(obj->id());
    std::vector<TowerObj*> objs;
    for (auto& it : m_objs) {
        if (it.second->id() == obj->id()) {
            continue;
        }
        objs.emplace_back(it.second);
    }
    if (objs.empty()) {
        return;
    }
    obj->onLeave(objs);
}
