#include "zskip_dict.h"

#include <iostream>

namespace zskip {
ZskipNode::ZskipNode(int level, uint64_t score, std::string value)
    : m_Score(score), m_Value(value), m_Backward(nullptr) {
    m_Level.resize(level);
    for (int i = 0; i < level; i++) {
        m_Level[i] = new zskipLevel();
        m_Level[i]->m_Forward = nullptr;
        m_Level[i]->m_Span = 0;
    }
}

ZskipNode::~ZskipNode() {
    for (int i = 0; i < m_Level.size(); i++) {
        if (!m_Level[i]) {
            continue;
        }
        delete m_Level[i];
    }
    m_Level.clear();
};

ZskipNode* newZskipNode(int level, uint64_t score, std::string value) { return new ZskipNode(level, score, value); };

ZskipList::ZskipList(int level) : m_Level(1), m_MaxLevel(level), m_Length(0) {
    m_Head = newZskipNode(level, 0);
    m_Tail = nullptr;
}

ZskipList::~ZskipList() {
    if (m_Head != nullptr) {
        delete m_Head;
        m_Head = nullptr;
    }
    if (m_Tail != nullptr) {
        delete m_Tail;
        m_Tail = nullptr;
    }
}

int ZskipList::randomLevel() {
    int level = 1;
    while ((std::rand() & 0xFFFF) < 0xFFFF / 4) {
        level++;
    }
    return level < m_MaxLevel ? level : m_MaxLevel;
}

ZskipNode* ZskipList::zslInsert(uint64_t score, std::string value) {
    std::vector<ZskipNode*> update(m_MaxLevel, nullptr);
    std::vector<int> rank(m_MaxLevel, 0);
    ZskipNode* head = m_Head;
    for (int i = m_Level - 1; i >= 0; i--) {
        if (i != m_Level - 1) {
            rank[i] = rank[i + 1];
        }
        while (head->m_Level[i]->m_Forward && head->m_Level[i]->m_Forward->m_Score < score ||
               (head->m_Level[i]->m_Forward && head->m_Level[i]->m_Forward->m_Score == score &&
                head->m_Level[i]->m_Forward->m_Value < value)) {
            rank[i] += head->m_Level[i]->m_Span;
            head = head->m_Level[i]->m_Forward;
        }
        update[i] = head;
    }
    int level = randomLevel();
    if (level > m_Level) {
        for (int i = m_Level; i < level; i++) {
            rank[i] = 0;
            update[i] = m_Head;
            update[i]->m_Level[i]->m_Span = m_Length;
        }
        m_Level = level;
    }

    ZskipNode* newNode = newZskipNode(level, score, value);
    for (int i = 0; i < level; i++) {
        newNode->m_Level[i]->m_Forward = update[i]->m_Level[i]->m_Forward;
        update[i]->m_Level[i]->m_Forward = newNode;
        newNode->m_Level[i]->m_Span = update[i]->m_Level[i]->m_Span - (rank[0] - rank[i]);
        update[i]->m_Level[i]->m_Span = (rank[0] - rank[i]) + 1;
    }
    for (int i = level; i < m_Level; i++) {
        update[i]->m_Level[i]->m_Span++;
    }
    if (update[0] != m_Head) {
        newNode->m_Backward = update[0];
    } else {
        newNode->m_Backward = nullptr;
    }
    if (newNode->m_Level[0]->m_Forward != nullptr) {
        newNode->m_Level[0]->m_Forward->m_Backward = newNode;
    } else {
        m_Tail = newNode;
    }

    m_Length++;
    return newNode;
}

void ZskipList::zslDeleteNode(ZskipNode* node, std::vector<ZskipNode*> updates) {
    for (int i = 0; i < m_Level; i++) {
        if (updates[i]->m_Level[i]->m_Forward == node) {
            updates[i]->m_Level[i]->m_Span += node->m_Level[i]->m_Span - 1;
            updates[i]->m_Level[i]->m_Forward = node->m_Level[i]->m_Forward;
        } else {
            updates[i]->m_Level[i]->m_Span--;
        }
    }
    if (node->m_Level[0]->m_Forward) {
        node->m_Level[0]->m_Forward->m_Backward = node->m_Backward;
    } else {
        m_Tail = node->m_Backward;
    }
    while (m_Level > 1 && m_Head->m_Level[m_Level - 1]->m_Forward == nullptr) {
        m_Level--;
    }
    m_Length--;
    delete node;
    node = nullptr;
}

void ZskipList::zslDelete(uint64_t score) {
    std::vector<ZskipNode*> updates(m_Level, nullptr);
    ZskipNode* head = m_Head;
    for (int i = m_Level - 1; i >= 0; i--) {
        while (head->m_Level[i]->m_Forward != nullptr && head->m_Level[i]->m_Forward->m_Score < score) {
            head = head->m_Level[i]->m_Forward;
        }
        updates[i] = head;
    }
    head = head->m_Level[0]->m_Forward;
    if (head != nullptr && head->m_Score == score) {
        zslDeleteNode(head, updates);
    }
}

ZskipNode* ZskipList::zslGetNodeByRank(int64_t rank) {
    ZskipNode* head = m_Head;
    int64_t traversed = 0;
    for (int i = m_Level - 1; i >= 0; i--) {
        while (head->m_Level[i]->m_Forward && (traversed + head->m_Level[i]->m_Span <= rank)) {
            traversed += head->m_Level[i]->m_Span;
            head = head->m_Level[i]->m_Forward;
        }
        if (traversed == rank) {
            return head;
        }
    }
    return nullptr;
}

int64_t ZskipList::zslGetNodeByRankNo(uint64_t score, std::string value) {
    print();
    ZskipNode* head = m_Head;
    int64_t rank = 0;
    for (int i = m_Level - 1; i >= 0; i--) {
        while (head->m_Level[i]->m_Forward &&
               (head->m_Level[i]->m_Forward->m_Score < score ||
                (head->m_Level[i]->m_Forward->m_Score == score && head->m_Level[i]->m_Forward->m_Value <= value))) {
            rank += head->m_Level[i]->m_Span;
            head = head->m_Level[i]->m_Forward;
        }
        if (head && head->m_Score == score) {
            return rank;
        }
    }
    return 0;
}

ZskipNode* ZskipList::zslGetNodeBySocre(uint64_t score) {
    ZskipNode* head = m_Head;
    for (int i = m_Level - 1; i >= 0; i--) {
        while (head->m_Level[i]->m_Forward && (head->m_Level[i]->m_Forward->m_Score == score)) {
            head = head->m_Level[i]->m_Forward;
        }
        if (head->m_Score == score) {
            return head;
        }
    }
    return nullptr;
}

void ZskipList::zslRange(int64_t min, int64_t max, bool reverse, std::function<void(ZskipNode* node)> callback) {
    if (!callback || min > max) {
        return;
    }
    max = m_Length < max ? m_Length : max;
    ZskipNode* node;
    if (reverse) {
        node = zslGetNodeByRank(max - min);
    } else {
        node = zslGetNodeByRank(min);
    }
    if (!node) {
        return;
    }
    int64_t span = max - min + 1;
    while (node && span > 0) {
        span--;
        callback(node);
        if (reverse) {
            node = node->m_Backward;
        } else {
            node = node->m_Level[0]->m_Forward;
        }
    }
}

int ZskipList::zslLen() { return m_Length; }

void ZskipList::print() {
    for (int i = m_Level - 1; i >= 0; i--) {
        ZskipNode* head = m_Head;
        while (head && head->m_Level[i]->m_Forward) {
            std::cout << "value: " << head->m_Level[i]->m_Forward->m_Value
                      << "\tscore: " << head->m_Level[i]->m_Forward->m_Score << "\tspan: " << head->m_Level[i]->m_Span
                      << "\tlevel: " << i << "\n";
            head = head->m_Level[i]->m_Forward;
        }
        std::cout << std::endl;
    }
}
}; // namespace zskip
