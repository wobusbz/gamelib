#ifndef __TOWER_H__
#define __TOWER_H__

#include <unordered_map>
#include <vector>

#include "object_aoi.h"

namespace aoi {

class TowerAOI {
    friend class TowerAOIMannger;

    static inline int dist2(TowerObj* a, TowerObj* b) {
        int dx = a->x() - b->x();
        int dy = a->y() - b->y();
        return dx * dx + dy * dy;
    }

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

    template <typename Function>
    void visitWatchedTowersByWorld(int x, int y, int range, Function&& fn) {
        int minX = transX(x - range);
        int maxX = transX(x + range);
        int minY = transY(y - range);
        int maxY = transY(y + range);
        for (int xi = minX; xi <= maxX; xi++) {
            for (int yi = minY; yi <= maxY; yi++) {
                fn(m_towers[xi][yi]);
            }
        }
    }

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
