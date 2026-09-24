#pragma once
#include "decision_engine.h"
#include <deque>
#include <optional>

class ActionQueue {
    std::deque<ProposedAction> q;
public:
    void clear(){ q.clear(); }
    void push(const ProposedAction& a){ q.push_back(a); }
    std::optional<ProposedAction> peek() const { if(q.empty()) return std::nullopt; return q.front(); }
    std::optional<ProposedAction> pop(){ if(q.empty()) return std::nullopt; auto a=q.front(); q.pop_front(); return a; }
    size_t size() const { return q.size(); }
};
