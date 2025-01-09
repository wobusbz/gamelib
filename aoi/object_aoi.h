#ifndef __ObJECT_AOI_H__
#define __ObJECT_AOI_H__

#include <stdint.h>

#include <vector>

namespace aoi {

class TowerAOI;

class TowerObj {
    friend class TowerAOI;
    friend class GridAOIMannger;
    friend class TowerAOIMannger;

public:
    TowerObj();
    virtual ~TowerObj();
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
};
} // namespace aoi

#endif
