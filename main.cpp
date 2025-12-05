#include <iostream>
#include <random>
#include <string>

#include "aoi/grid.h"
#include "aoi/tower.h"
#include "zskip/zskip_dict.h"

std::random_device rd;
std::mt19937 gen(rd());
template <typename T>
T Rand(T n) {
    std::uniform_int_distribution<T> dis(1, n);
    return dis(gen);
}

class Objec : public aoi::TowerObj {
public:
    Objec(uint64_t id) : m_id(id) {
        m_x = 100 + Rand<int>(300);
        m_y = Rand(500);
    }
    void onEnter(std::vector<TowerObj*> other) override {
        std::cout << "ObjID: " << m_id << "  Size: " << other.size() << "\t OnEnter Obj ID: ";
        for (auto itOther : other) {
            std::cout << itOther->id() << "\t";
        }
        std::cout << std::endl;
    };

    void onLeave(std::vector<TowerObj*> other) override {
        std::cout << "ObjID: " << m_id << "  Size: " << other.size() << "\t OnLeave ID: ";
        for (auto itOther : other) {
            std::cout << itOther->id() << "\t";
        }
        std::cout << std::endl;
    };

    uint64_t id() override { return m_id; }
    uint64_t m_id;
};

int main(int argc, char** argv) {

    aoi::GridAOIManger gridMannger(0, 0, 1000, 1000, 10, 10);
    aoi::TowerAOIManger towerMannger(0, 1000, 0, 1000, 10);
    std::vector<Objec*> objs;
    for (int i = 1; i < 1000; i++) {
        Objec* obj = new Objec(i);
        gridMannger.enter(obj);
        // towerMannger.enter(obj);
        objs.emplace_back(obj);
    }
    for (auto it : objs) {
        gridMannger.moved(it, 400 + Rand(300), 500 + Rand(200));
        // towerMannger.Moved(it, 400+ Rand(300), 500 + Rand(200));
    }
    for (auto it : objs) {
        delete it;
        it = nullptr;
    }
    objs.clear();

    zskip::ZskipList zskipDict(10);
    zskipDict.zslInsert(101, "101");
    zskipDict.zslInsert(102, "102");
    zskipDict.zslInsert(100, "100");
    /*zskipDict.zslRange(0, 10000, true, [&](zskip::ZskipNode* node) { std::cout << node->m_Value << std::endl; });*/
    zskipDict.print();
    std::cout << zskipDict.zslLen() << std::endl;
    return 0;
}
