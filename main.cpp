#include "aoi/grid.h"
#include "aoi/tower.h"

#include <iostream>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
template <typename T> T Rand(T n) {
  std::uniform_int_distribution<T> dis(1, n);
  return dis(gen);
}

class Objec : public aoi::TowerObj {
public:
  Objec(uint64_t id) : m_id(id) {
    m_x = 100 + Rand<int>(300);
    m_y = Rand(500);
  }
  void OnEnter(std::vector<TowerObj *> other) override {
    std::cout << "ObjID: " << m_id << "  Size: " << other.size()
              << "\t OnEnter Obj ID: ";
    for (auto itOther : other) {
      std::cout << itOther->ID() << "\t";
    }
    std::cout << std::endl;
  };

  void OnLeave(std::vector<TowerObj *> other) override {
    std::cout << "ObjID: " << m_id << "  Size: " << other.size()
              << "\t OnLeave ID: ";
    for (auto itOther : other) {
      std::cout << itOther->ID() << "\t";
    }
    std::cout << std::endl;
  };

  uint64_t ID() override { return m_id; }
  uint64_t m_id;
};

int main(int argc, char **argv) {

  aoi::GridAOIMannger gridMannger(0, 0, 1000, 1000, 10, 10);
  aoi::TowerAOIMannger towerMannger(0, 1000, 0, 1000, 10);
  std::vector<Objec *> objs;
  for (int i = 1; i < 1000; i++) {
    Objec *obj = new Objec(i);
    gridMannger.Enter(obj);
    // towerMannger.Enter(obj);
    objs.emplace_back(obj);
  }
  for (auto it : objs) {
    gridMannger.Moved(it, 400 + Rand(300), 500 + Rand(200));
    // towerMannger.Moved(it, 400+ Rand(300), 500 + Rand(200));
  }
  for (auto it : objs) {
    delete it;
    it = nullptr;
  }
  objs.clear();
  return 0;
}
