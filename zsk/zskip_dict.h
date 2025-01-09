#ifndef __ZSKIP_DICT_H__
#define __ZSKIP_DICT_H__
#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace zsk {
struct ZskipNode;
struct zskipLevel {
    ZskipNode* m_Forward; // 前进指针，指向的是当前节点本身
    int m_Span;
};

struct ZskipNode {
    uint64_t m_Score;
    std::string m_Value;
    ZskipNode* m_Backward;
    std::vector<zskipLevel*> m_Level;

    ZskipNode(int level, uint64_t score, std::string value);
    virtual ~ZskipNode();
};

ZskipNode* newZskipNode(int level, uint64_t score = 0, std::string value = "");

class ZskipList {
public:
    ZskipList(int level);
    virtual ~ZskipList();

    ZskipNode* zslInsert(uint64_t score, std::string value);
    void zslDelete(uint64_t score);
    void zslDeleteNode(ZskipNode* node, std::vector<ZskipNode*> updates);
    ZskipNode* zslGetNodeByRank(int64_t rank);
    int64_t zslGetNodeByRankNo(uint64_t score, std::string value);
    ZskipNode* zslGetNodeBySocre(uint64_t score);
    void zslRange(int64_t min, int64_t max, bool reverse, std::function<void(ZskipNode* node)> callback);
    int zslLen();
    void print();

private:
    int randomLevel(); // 随机层数

    ZskipNode* m_Head;
    ZskipNode* m_Tail;
    int m_Level;    // 当前层数
    int m_MaxLevel; // 最大层数
    int m_Length;   // 长度
};

template <typename T>
class Element {
public:
    Element<T>(uint64_t score, T ele) : m_Score(score), m_Ele(ele) {}
    virtual ~Element<T>() {
        if (!m_Ele) {
            return;
        }
        delete m_Ele;
        m_Ele = nullptr;
    };
    inline void setEle(T ele) { m_Ele = ele; };
    inline T getEle() const { return m_Ele; };
    inline void setScore(uint64_t score) { m_Score = score; };
    inline uint64_t getScore() const { return m_Score; };

private:
    T m_Ele;
    uint64_t m_Score;
};

template <typename T>
class ZslDict {
public:
    ZslDict(int level) : m_Zsl(new ZskipList(level)), m_Level(level) {}
    virtual ~ZslDict() {
        for (auto it = m_Dict.begin(); it != m_Dict.end(); ++it) {
            if (!it->second) {
                continue;
            }
            delete it->second;
        }
        if (!m_Zsl) {
            return;
        }
        delete m_Zsl;
    }

    void zslSet(std::string key, uint64_t score, T ele) {
        auto it = m_Dict.find(key);
        if (it == m_Dict.end()) {
            m_Dict[key] = new Element<T>(score, ele);
            m_Zsl->zslInsert(score, key);
        } else {
            m_Zsl->zslDelete(it->second->getScore());
            m_Zsl->zslInsert(score, key);
            it->second->setScore(score);
            it->second->setEle(ele);
        }
    }

    T zslGet(std::string key) {
        auto it = m_Dict.find(key);
        if (it == m_Dict.end()) {
            return nullptr;
        }
        return it->second->getEle();
    }

    unsigned int zslGetRankNo(std::string key, bool reverse = true) {
        auto it = m_Dict.find(key);
        if (it == m_Dict.end()) {
            return 0;
        }
        unsigned int rankNo = m_Zsl->zslGetNodeByRankNo(it->second->getScore(), key);
        return reverse ? zslLen() + 1 - rankNo : rankNo;
    };

    void zslDel(std::string key) {
        auto it = m_Dict.find(key);
        if (it == m_Dict.end()) {
            return;
        }
        m_Zsl->zslDelete(it->second->getScore());
        m_Dict.erase(key);
    }

    void zslRange(int min, int max, std::function<bool(std::string key, T ele)> callback, bool reverse = true) {
        m_Zsl->zslRange(min, max, reverse, [&](ZskipNode* node) {
            if (!node) {
                return;
            }
            auto it = m_Dict.find(node->m_Value);
            if (it == m_Dict.end()) {
                return;
            }
            if (!callback) {
                return;
            }
            if (callback(node->m_Value, it->second->getEle())) {
                return;
            }
        });
    }
    int zslLen() { return m_Zsl->zslLen(); }

    void zslReset() {
        for (auto it = m_Dict.begin(); it != m_Dict.end(); ++it) {
            if (!it->second) {
                continue;
            }
            delete it->second;
        }
        m_Dict.clear();
        if (m_Zsl) {
            delete m_Zsl;
            m_Zsl = nullptr;
            m_Zsl = new ZskipList(m_Level);
        }
    }

private:
    std::map<std::string, Element<T>*> m_Dict;
    ZskipList* m_Zsl;
    int64_t m_Level;
};
} // namespace zsk

#endif
