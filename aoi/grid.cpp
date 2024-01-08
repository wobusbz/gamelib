#include "grid.h"

#include <iostream>

aoi::Grid::Grid(int id): m_gridId(id)
{
}

aoi::Grid::~Grid()
{
	m_objs.clear();
}

void aoi::Grid::addObjs(TowerObj* obj)
{
	m_objs[obj->ID()] = obj;
}

void aoi::Grid::removeObjs(TowerObj* obj)
{
	m_objs.erase(obj->ID());
}

aoi::GridAOIMannger::GridAOIMannger(int minX, int minY, int maxX, int maxY, int xSize, int ySize):
	m_minX(minX), m_minY(minY), m_maxX(maxX), m_maxY(maxY), m_xSize(xSize), m_ySize(ySize)
{
	initGrid();
}

aoi::GridAOIMannger::~GridAOIMannger()
{
	for (auto & it : m_grids) {
		if (!it.second) {
			continue;
		}
		delete it.second;
		it.second = nullptr;
	}
	m_grids.clear();
}

void aoi::GridAOIMannger::Enter(TowerObj* obj)
{
	auto itGrids = m_grids.find(GridID(obj->X(), obj->Y()));
	if (itGrids == m_grids.end()) {
		return;
	}
	itGrids->second->addObjs(obj);
	std::vector<TowerObj*> objs;
	if (!visitWatchedGridObjs(obj, objs)) {
		return;
	}
	for (auto it : objs) {
		it->OnEnter({ obj });
	}
	if (objs.size() == 0) {
		return;
	}
	obj->OnEnter(objs);
}

void aoi::GridAOIMannger::Moved(TowerObj* obj, int x, int y)
{
	if (obj->X() == x && obj->Y() == y) {
		return;
	}
	std::unordered_map<uint64_t, TowerObj*> oldObjs;
	if (!visitWatchedGridObjs(obj, oldObjs)) {
		return;
	}
	int oldGridID = GridID(obj->X(), obj->Y());
	auto itOldGrids = m_grids.find(oldGridID);
	if (itOldGrids == m_grids.end()) {
		return;
	}
	itOldGrids->second->removeObjs(obj);

	std::unordered_map<uint64_t, TowerObj*> newObjs;
	obj->m_x = x;
	obj->m_y = y;
	if (!visitWatchedGridObjs(obj, newObjs)) {
		return;
	}

	int newGridID = GridID(obj->X(), obj->Y());
	auto itNewGrids = m_grids.find(newGridID);
	if (itNewGrids == m_grids.end()) {
		return;
	}
	itNewGrids->second->addObjs(obj);

	std::vector<TowerObj*> lostGrids;
	for (auto & itObjs : oldObjs) {
		auto itNewObjs = newObjs.find(itObjs.second->ID());
		if (itNewObjs != newObjs.end()) {
			continue;
		}
		itObjs.second->OnLeave({ obj });
		lostGrids.emplace_back(itObjs.second);
	}
	if (!lostGrids.empty()) {
		obj->OnLeave({ lostGrids });
	}
	std::vector< TowerObj*> bornGrids;
	for (auto& itObjs : newObjs) {
		auto itOldObjs = oldObjs.find(itObjs.second->ID());
		if (itOldObjs != oldObjs.end()) {
			continue;
		}
		itObjs.second->OnEnter({ obj });
		bornGrids.emplace_back(itObjs.second);
	}
	if (!bornGrids.empty()) {
		obj->OnEnter({ bornGrids });
	}
}

void aoi::GridAOIMannger::Leave(TowerObj* obj)
{
	auto itGrids = m_grids.find(GridID(obj->X(), obj->Y()));
	if (itGrids == m_grids.end()) {
		return;
	}
	itGrids->second->removeObjs(obj);
	std::vector<TowerObj*> objs;
	visitWatchedGridObjs(obj, objs);
	for (auto it : objs) {
		it->OnLeave({ obj });
	}
	obj->OnLeave(objs);
}

int aoi::GridAOIMannger::GridID(int x, int y)
{
	return transY(y) * m_xNum + transX(x);
}

bool aoi::GridAOIMannger::visitWatchedGridObjs(TowerObj* obj, std::unordered_map<uint64_t, TowerObj*> & objs)
{
	auto itGrids = m_grids.find(GridID(obj->X(), obj->Y()));
	if (itGrids == m_grids.end()) {
		return false;
	}
	int x = transX(obj->X());
	int y = transY(obj->Y());
	int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	for (int i = 0; i < 8; i++) {
		int newX = x + dx[i];
		int newY = y + dy[i];
		if (newX < 0 && newX >= m_xNum && newY < 0 && newY >= m_yNum) {
			continue;
		}
		auto itNewGrids = m_grids.find(newY * m_xNum + newX);
		if (itNewGrids == m_grids.end()) {
			continue;
		}
		for (auto & itGridObj : itNewGrids->second->m_objs) {
			if (itGridObj.second->ID() == obj->ID()) {
				continue;
			}
			objs[itGridObj.second->ID()] = itGridObj.second;
		}
	}
	return true;
}

bool aoi::GridAOIMannger::visitWatchedGridObjs(TowerObj* obj, std::vector<TowerObj*>& objs)
{
	auto itGrids = m_grids.find(GridID(obj->X(), obj->Y()));
	if (itGrids == m_grids.end()) {
		return false;
	}
	for (auto& itGridObj : itGrids->second->m_objs) {
		if (itGridObj.second->ID() == obj->ID()) {
			continue;
		}
		objs.emplace_back(itGridObj.second);
	}
	int x = transX(obj->X());
	int y = transY(obj->Y());
	int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	for (int i = 0; i < 8; i++) {
		int newX = x + dx[i];
		int newY = y + dy[i];
		if (newX <= 0 && newX >= m_xNum && newY <= 0 && newY >= m_yNum) {
			continue;
		}
		auto itNewGrids = m_grids.find(newY * m_xNum + newX);
		if (itNewGrids == m_grids.end()) {
			continue;
		}
		for (auto& itGridObj : itNewGrids->second->m_objs) {
			if (itGridObj.second->ID() == obj->ID()) {
				continue;
			}
			objs.emplace_back(itGridObj.second);
		}
	}
	return true;
}

void aoi::GridAOIMannger::initGrid()
{
	m_yNum = transY(m_maxY);
	m_xNum = transY(m_maxX);
	for (int y = 0; y < m_yNum; y++) {
		for (int x = 0; x < m_xNum; x++) {
			int gid = y * m_xNum + x;
			m_grids[gid] = new Grid(gid);
		}
	}
}

int aoi::GridAOIMannger::transX(int x)
{
	int tx = (int)(std::ceil(x - m_minY) / (double)m_xSize);
	if (tx < 0) {
		tx = 0;
	}
	return tx;
}

int aoi::GridAOIMannger::transY(int y)
{
	int ty = (int)(std::ceil(y - m_minY) / (double)m_ySize);
	if (ty < 0) {
		ty = 1;
	}
	return ty;
}
