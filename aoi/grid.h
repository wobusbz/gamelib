#ifndef __GRID_H__
#define __GRID_H__

#include "object_aoi.h"

#include <vector>
#include <functional>
#include <unordered_map>

namespace aoi {

class Grid {
	friend class GridAOIMannger;
public:
	Grid(int id);
	~Grid();
	void addObjs(TowerObj* obj) ;
	void removeObjs(TowerObj* obj);

private:
	int		m_gridId;
	std::unordered_map<uint64_t, TowerObj*> m_objs;
};

class GridAOIMannger {

public:
	GridAOIMannger(int minX, int minY, int maxX, int maxY, int xSize, int ySize);
	~GridAOIMannger();
	void Enter(TowerObj* obj);
	void Moved(TowerObj* obj, int x, int y);
	void Leave(TowerObj* obj);

	int GridID(int x, int y);
	bool visitWatchedGridObjs(TowerObj* obj, std::function<void(Grid*)>);
	bool visitWatchedGridObjs(TowerObj* obj, std::unordered_map<uint64_t, TowerObj*> & objs);
	bool visitWatchedGridObjs(TowerObj* obj, std::vector<TowerObj*> & objs);

private:
	void initGrid();
	int transX(int x);
	int transY(int y);

	std::unordered_map<int, Grid*> m_grids;
	int m_minX;
	int m_minY;
	int m_maxX;
	int m_maxY;
	int m_xSize;
	int m_ySize;
	int m_xNum;
	int m_yNum;
};

}

#endif // !__GRID_H__
