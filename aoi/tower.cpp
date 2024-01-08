#include "tower.h"

aoi::TowerObj::TowerObj(): m_x(0), m_y(0), m_z(0), m_dist(0)
{
}

aoi::TowerObj::~TowerObj()
{
}

int aoi::TowerObj::X()
{
	return m_x;
}

int aoi::TowerObj::Y()
{
	return m_y;
}

int aoi::TowerObj::Dist()
{
	return m_dist;
}

aoi::TowerAOIMannger::TowerAOIMannger(int minX, int maxX, int minY, int maxY, int range) : 
	m_minX(minX),m_maxX(maxX), m_minY(minY), m_maxY(maxY), m_range(range), m_xTowerNum(0), m_yTowerNum(0) {
	init();
}

aoi::TowerAOIMannger::~TowerAOIMannger()
{
	for (auto & it : m_towers) {
		for (auto itr : it) {
			delete itr;
			itr = nullptr;
		}
	}
	m_towers.clear();
}

int aoi::TowerAOIMannger::transX(int x)
{
	int tx = (x - m_minX) / m_range;
	if (tx < 0) {
		tx = 0;
	}
	else if (tx >= m_xTowerNum) {
		tx = m_xTowerNum - 1;
	}
	return tx;
}

int aoi::TowerAOIMannger::transY(int y)
{
	int ty = (y - m_minY) / m_range;
	if (ty < 0) {
		ty = 0;
	}
	else if (ty >= m_yTowerNum) {
		ty = m_yTowerNum - 1;
	}
	return ty;
}

aoi::TowerAOI* aoi::TowerAOIMannger::getTower(int x, int y)
{
	return m_towers[transX(x)][transY(y)];
}

void aoi::TowerAOIMannger::visitWatchedTowers(int x, int y, int range, std::function<void(TowerAOI*)> func)
{
	int minX = transX(x - range);
	int maxX = transX(x + range);
	int minY = transY(y - range);
	int maxY = transY(y + range);
	for (int xi = minX; xi <= maxX; xi++) {
		for (int yi = minY; yi <= maxY; yi++) {
			if (!func) {
				continue;
			}
			func(m_towers[xi][yi]);
		}
	}
}

void aoi::TowerAOIMannger::init()
{
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

void aoi::TowerAOIMannger::Enter(TowerObj* obj)
{
	visitWatchedTowers(obj->X(), obj->Y(), obj->Dist(), [&](aoi::TowerAOI* objTowerAOI) {
		objTowerAOI->addWatchersObj(obj);
	});
	getTower(obj->X(), obj->Y())->addObjs(obj);
}

void aoi::TowerAOIMannger::Moved(TowerObj* obj, int x, int y)
{
	if (obj->X() == x && obj->Y() == y) {
		return;
	}
	obj->m_towerAOI->removeObjs(obj);
	int oldMinX = transX(obj->X() - obj->m_dist);
	int oldMaxX = transX(obj->X() + obj->m_dist);
	int oldMinY = transY(obj->Y() - obj->m_dist);
	int oldMaxY = transY(obj->Y() + obj->m_dist);
	obj->m_x = x;
	obj->m_y = y;
	getTower(x, y)->addObjs(obj, obj->m_towerAOI);
	int newMinX = transX(obj->X() - obj->m_dist);
	int newMaxX = transX(obj->X() + obj->m_dist);
	int newMinY = transY(obj->Y() - obj->m_dist);
	int newMaxY = transY(obj->Y() + obj->m_dist);
	for (int xi = oldMinX; xi <= oldMaxX; xi++) {
		for (int yj = oldMinY; yj <= oldMaxY; yj++) {
			if (xi >= newMinX && xi <= newMaxX && yj >= newMinY && yj <= newMaxY) {
				continue;
			}
			m_towers[xi][yj] ->removeWatchersObj(obj);
		}
	}
	for (int xi = newMinX; xi < newMaxX; xi++) {
		for (int yj = newMinY; yj < newMaxY; yj++) {
			if (xi >= oldMinX && xi <= oldMaxX && yj >= oldMinY && yj <= oldMaxY) {
				continue;
			}
			m_towers[xi][yj]->addWatchersObj(obj);
		}
	}
}

void aoi::TowerAOIMannger::Leave(TowerObj* obj)
{
	obj->m_towerAOI->removeObjs(obj, true);
	visitWatchedTowers(obj->X(), obj->Y(), obj->Dist(), [&](aoi::TowerAOI* objTowerAOI) {
		objTowerAOI->removeWatchersObj(obj);
	});
}


void aoi::TowerAOI::addObjs(TowerObj* obj, TowerAOI* fromObjs)
{
	obj->m_towerAOI = this;
	auto itObjs = m_objs.find(obj->ID());
	if (itObjs == m_objs.end()) {
		m_objs[obj->ID()] = obj;
	}
	if (!fromObjs) {
		for (auto& it : m_watchers) {
			if (it.second->ID() == obj->ID()) {
				continue;
			} 
			it.second->OnEnter({ obj });
		}
	}
	else {
		for (auto& it : fromObjs->m_watchers) {
			if (it.second->ID() == obj->ID()) {
				continue;
			}
			auto itWatchers = m_watchers.find(it.second->ID());
			if (itWatchers != m_watchers.end()) {
				continue;
			}
			it.second->OnLeave({ obj });
		}
		for (auto& it : m_watchers) {
			if (it.second->ID() == obj->ID()) {
				continue;
			}
			auto itWatchers = fromObjs->m_watchers.find(it.second->ID());
			if (itWatchers != fromObjs->m_watchers.end()) {
				continue;
			}
			it.second->OnEnter({ obj });
		}
	}
}

void aoi::TowerAOI::addWatchersObj(TowerObj* obj)
{
	auto itWatchers =  m_watchers.find(obj->ID());
	if (itWatchers == m_watchers.end()) {
		m_watchers[obj->ID()] = obj;
	}
	std::vector<TowerObj*> objs;
	for (auto& it : m_objs) {
		if (it.second->ID() == obj->ID()) {
			continue;
		}
		objs.emplace_back(it.second);
	}
	if (objs.empty()) {
		return;
	}
	obj->OnEnter(objs);
}

void aoi::TowerAOI::removeObjs(TowerObj* obj, bool notify)
{
	m_objs.erase(obj->ID());
	if (!notify) {
		return;
	}
	for (auto& it : m_watchers) {
		if (it.second->ID() == obj->ID()) {
			continue;
		}
		it.second->OnLeave({ obj });
	}
}

void aoi::TowerAOI::removeWatchersObj(TowerObj* obj)
{
	m_watchers.erase(obj->ID());
	std::vector<TowerObj*> objs;
	for (auto& it : m_objs) {
		if (it.second->ID() == obj->ID()) {
			continue;
		}
		objs.emplace_back(it.second);
	}
	if (objs.empty()) {
		return;
	}
	obj->OnLeave(objs);
}
