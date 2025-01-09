#ifndef __TOWER_H__
#define __TOWER_H__

#include <functional>
#include <unordered_map>
#include <vector>

#include "object_aoi.h"

namespace aoi {

class TowerAOI {
    friend class TowerAOIMannger;

    void addObjs(TowerObj* obj, TowerAOI* fromObjs = nullptr);
    void addWatchersObj(TowerObj* obj);
    void removeObjs(TowerObj* obj, bool notify = false);
    void removeWatchersObj(TowerObj* obj);

    std::unordered_map<uint64_t, TowerObj*> m_objs;
    std::unordered_map<uint64_t, TowerObj*> m_watchers;
};

class TowerAOIMannger {
public:
    TowerAOIMannger(int minX, int maxX, int minY, int maxY, int range);
    virtual ~TowerAOIMannger();
    void enter(TowerObj* obj);
    void moved(TowerObj* obj, int x, int y);
    void leave(TowerObj* obj);

private:
    int transX(int x) const;
    int transY(int y) const;

    TowerAOI* getTower(int x, int y);
    void visitWatchedTowers(int x, int y, int range, std::function<void(TowerAOI*)>);

    void initialize();

private:
    int m_minX;
    int m_maxX;
    int m_minY;
    int m_maxY;
    int m_range;
    int m_xTowerNum;
    int m_yTowerNum;
    std::vector<std::vector<TowerAOI*>> m_towers;
};

} // namespace aoi

#endif // !__TOWER_H__
