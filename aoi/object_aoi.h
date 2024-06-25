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
  virtual void OnEnter(std::vector<TowerObj *> other) = 0;
  virtual void OnLeave(std::vector<TowerObj *> other) = 0;
  virtual uint64_t ID() = 0;
  int X() const;
  int Y() const;
  int Dist() const;

protected:
  int m_x;
  int m_y;
  int m_z;
  int m_dist;
  TowerAOI *m_towerAOI = nullptr;
};
} // namespace aoi

#endif
