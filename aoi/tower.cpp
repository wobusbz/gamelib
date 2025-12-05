#include "tower.h"

namespace aoi {

TowerAOIManger::TowerAOIManger(int minX, int maxX, int minY, int maxY, int range)
    : m_minX(minX), m_maxX(maxX), m_minY(minY), m_maxY(maxY), m_range(range), m_xTowerNum(0), m_yTowerNum(0) {
    initialize();
}

TowerAOIManger::~TowerAOIManger() {
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

int TowerAOIManger::transX(int x) const {
    int tx = (x - m_minX) / m_range;
    if (tx < 0) {
        tx = 0;
    } else if (tx >= m_xTowerNum) {
        tx = m_xTowerNum - 1;
    }
    return tx;
}

int TowerAOIManger::transY(int y) const {
    int ty = (y - m_minY) / m_range;
    if (ty < 0) {
        ty = 0;
    } else if (ty >= m_yTowerNum) {
        ty = m_yTowerNum - 1;
    }
    return ty;
}

TowerAOI* TowerAOIManger::getTower(int x, int y) { return m_towers[transX(x)][transY(y)]; }

std::unordered_set<TowerAOI*> TowerAOIManger::getWatchedTowers(int x, int y, int range) const {
    std::unordered_set<aoi::TowerAOI*> towers;
    if (range < 0) {
        return towers;
    }
    int minX = transX(x - range);
    int maxX = transX(x + range);
    int minY = transY(y - range);
    int maxY = transY(y + range);
    for (int xi = minX; xi <= maxX; xi++) {
        for (int yj = minY; yj <= maxY; yj++) {
            towers.insert(m_towers[xi][yj]);
        }
    }
    return towers;
}

void TowerAOIManger::initialize() {
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

std::vector<TowerObj*> TowerAOIManger::collectVisibleObjsAtPos(TowerObj* obj, int x, int y) const {
    std::vector<TowerObj*> result;
    if (!obj) {
        return result;
    }
    int r2 = obj->dist() * obj->dist();
    std::unordered_set<uint64_t> seen;
    int minX = transX(x - obj->dist());
    int maxX = transX(x + obj->dist());
    int minY = transY(y - obj->dist());
    int maxY = transY(y + obj->dist());
    for (int xi = minX; xi <= maxX; xi++) {
        for (int yi = minY; yi <= maxY; yi++) {
            TowerAOI* towers = m_towers[xi][yi];
            if (!towers) {
                continue;
            }
            for (const auto& it : towers->m_objs) {
                if (!it.second) {
                    continue;
                }
                if (it.second->id() == obj->id()) {
                    continue;
                }
                if (seen.find(it.second->id()) != seen.end()) {
                    continue;
                }
                if (TowerAOI::dist2(obj, it.second) > r2) {
                    continue;
                }
                seen.insert(it.second->id());
                result.emplace_back(it.second);
            }
        }
    }
    return result;
}

void TowerAOIManger::enter(TowerObj* obj) {
    if (!obj) {
        return;
    }

    visitWatchedTowersByWorld(obj->x(), obj->y(), obj->dist(),
                              [&](aoi::TowerAOI* objTowerAOI) { objTowerAOI->addWatchersObj(obj); });

    getTower(obj->x(), obj->y())->addObjs(obj);

    std::vector<TowerObj*> visible = collectVisibleObjsAtPos(obj, obj->x(), obj->y());
    obj->m_lastSeen.clear();
    for (auto o : visible) {
        obj->m_lastSeen.insert(o->id());
    }
}

void TowerAOIManger::moved(TowerObj* obj, int x, int y) {
    if (!obj) {
        return;
    }
    if (obj->x() == x && obj->y() == y) {
        return;
    }

    auto oldTower = getWatchedTowers(obj->x(), obj->y(), obj->dist());
    auto newTower = getWatchedTowers(x, y, obj->dist());

    std::vector<TowerAOI*> towersToLevels;
    towersToLevels.reserve(oldTower.size());
    for (auto tower : oldTower) {
        if (newTower.find(tower) != newTower.end()) {
            continue;
        }
        towersToLevels.emplace_back(tower);
    }

    std::vector<TowerAOI*> towersToEnters;
    towersToEnters.reserve(newTower.size());
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

    int oldx = obj->x();
    int oldy = obj->y();
    obj->x(x);
    obj->y(y);

    auto newObjTower = getTower(x, y);
    if (obj->m_towerAOI != newObjTower) {
        if (obj->m_towerAOI) {
            obj->m_towerAOI->removeObjs(obj);
        }
        newObjTower->addObjs(obj);
    }

    std::unordered_set<uint64_t> prevSeen = obj->m_lastSeen;

    std::vector<TowerObj*> nowObjs = collectVisibleObjsAtPos(obj, x, y);
    std::unordered_set<uint64_t> nowIds;
    nowIds.reserve(nowObjs.size());
    for (auto o : nowObjs) {
        nowIds.insert(o->id());
    }

    std::vector<TowerObj*> entered;
    for (auto o : nowObjs) {
        if (prevSeen.find(o->id()) == prevSeen.end()) {
            entered.emplace_back(o);
        }
    }

    std::vector<TowerObj*> left;
    for (auto id : prevSeen) {
        if (nowIds.find(id) == nowIds.end()) {
            bool found = false;
            int r = obj->dist();
            int minX = transX(oldx - r);
            int maxX = transX(oldx + r);
            int minY = transY(oldy - r);
            int maxY = transY(oldy + r);
            for (int xi = minX; xi <= maxX && !found; xi++) {
                for (int yi = minY; yi <= maxY && !found; yi++) {
                    TowerAOI* t = m_towers[xi][yi];
                    if (!t)
                        continue;
                    auto it = t->m_objs.find(id);
                    if (it != t->m_objs.end()) {
                        left.emplace_back(it->second);
                        found = true;
                    }
                }
            }
        }
    }

    if (!entered.empty()) {
        obj->onEnter(entered);
    }
    if (!left.empty()) {
        obj->onLeave(left);
    }

    obj->m_lastSeen.clear();
    for (auto id : nowIds) {
        obj->m_lastSeen.insert(id);
    }
}

void TowerAOIManger::leave(TowerObj* obj) {
    if (!obj) {
        return;
    }

    if (obj->m_towerAOI) {
        obj->m_towerAOI->removeObjs(obj, true);
    }

    visitWatchedTowersByWorld(obj->x(), obj->y(), obj->dist(),
                              [&](aoi::TowerAOI* objTowerAOI) { objTowerAOI->removeWatchersObj(obj); });

    obj->m_lastSeen.clear();
}

void TowerAOI::addObjs(TowerObj* obj) {
    if (!obj) {
        return;
    }
    obj->m_towerAOI = this;
    auto itObjs = m_objs.find(obj->id());
    if (itObjs == m_objs.end()) {
        m_objs[obj->id()] = obj;
    }

    std::vector<TowerObj*> other;
    other.resize(1);
    for (auto& it : m_watchers) {
        TowerObj* watcher = it.second;
        if (!watcher) {
            continue;
        }
        if (watcher->id() == obj->id()) {
            continue;
        }
        int r2 = watcher->dist() * watcher->dist();
        if (dist2(watcher, obj) > r2) {
            continue;
        }
        other[0] = obj;
        watcher->onEnter(other);
    }
}

void TowerAOI::addWatchersObj(TowerObj* obj) {
    if (!obj) {
        return;
    }
    auto itWatchers = m_watchers.find(obj->id());
    if (itWatchers == m_watchers.end()) {
        m_watchers[obj->id()] = obj;
    }
    std::vector<TowerObj*> objs;
    objs.reserve(m_objs.size());
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

void TowerAOI::removeObjs(TowerObj* obj, bool notify) {
    if (!obj) {
        return;
    }
    m_objs.erase(obj->id());
    if (!notify) {
        return;
    }

    std::vector<TowerObj*> other;
    other.resize(1);
    for (auto& it : m_watchers) {
        TowerObj* watcher = it.second;
        if (!watcher) {
            continue;
        }
        if (watcher->id() == obj->id()) {
            continue;
        }
        int r2 = watcher->dist() * watcher->dist();
        if (dist2(obj, watcher) > r2) {
            continue;
        }
        other[0] = obj;
        watcher->onLeave(other);
    }
}

void TowerAOI::removeWatchersObj(TowerObj* obj) {
    if (!obj) {
        return;
    }
    m_watchers.erase(obj->id());

    std::vector<TowerObj*> objs;
    objs.reserve(m_objs.size());
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
    obj->onLeave(objs);
}

} // namespace aoi
