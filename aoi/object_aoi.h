#ifndef __ObJECT_AOI_H__
#define __ObJECT_AOI_H__

#include <stdint.h>
#include <vector>

namespace aoi {

class TowerObj;
class TowerAOI;

class ObJectAOI {
public:
	~ObJectAOI() {};
	virtual void addObjs(TowerObj* obj, ObJectAOI* fromObjs = nullptr) = 0;
	virtual void addWatchersObj(TowerObj* obj) = 0;
	virtual void removeObjs(TowerObj* obj, bool notify = false) = 0;
	virtual void removeWatchersObj(TowerObj* obj) = 0;
	virtual bool hasWatchersObj(TowerObj* obj) = 0;
};

class TowerObj {
	friend class TowerAOI;
	friend class GridAOIMannger;
	friend class TowerAOIMannger;
public:
	TowerObj();
	virtual ~TowerObj();
	virtual void OnEnter(std::vector<TowerObj*> other) = 0;
	virtual void OnLeave(std::vector<TowerObj*> other) = 0;
	virtual uint64_t ID() = 0;
	int X();
	int Y();
	int Dist();

protected:
	int m_x;
	int m_y;
	int m_z;
	int m_dist;
	TowerAOI* m_towerAOI = nullptr;
};
}

#endif