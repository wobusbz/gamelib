#ifndef __ObJECT_AOI_H__
#define __ObJECT_AOI_H__

#include <stdint.h>

#include <unordered_set>
#include <vector>

namespace aoi {

class TowerAOI;

class TowerObj {
    friend class TowerAOI;
    friend class GridAOIManger;
    friend class TowerAOIManger;

public:
    TowerObj() : m_x(0), m_y(0), m_z(0), m_dist(0) {};
    virtual ~TowerObj() = default;
    virtual void onEnter(std::vector<TowerObj*> other) = 0;
    virtual void onLeave(std::vector<TowerObj*> other) = 0;
    virtual uint64_t id() = 0;

    int x() const { return m_x; };
    void x(const int x) { m_x = x; };
    int y() const { return m_y; };
    void y(const int y) { m_y = y; };
    int z() const { return m_z; }
    void z(const int z) { m_z = z; };
    int dist() const { return m_dist; };

protected:
    int m_x;
    int m_y;
    int m_z;
    int m_dist;
    TowerAOI* m_towerAOI = nullptr;
    std::unordered_set<uint64_t> m_lastSeen;
};
} // namespace aoi

#endif
