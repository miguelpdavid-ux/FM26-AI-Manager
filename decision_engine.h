#pragma once
#include <string>
#include <vector>
#include <algorithm>

enum class Risk { Low, Medium, High, Blocked };
enum class AgentMode { Observe, Assist, Confirm, Autonomous };

struct ClubState {
    std::wstring screen = L"UNKNOWN";
    double screenConfidence = 0.0;
    bool fmDetected = false;
    bool paused = false;
    bool emergencyStop = false;
    double balance = 0.0;
    double transferBudget = 0.0;
    double wageBudget = 0.0;
    double wageCommitted = 0.0;
};

struct Objective {
    std::wstring id;
    std::wstring description;
    int priority = 0;
};

struct ProposedAction {
    std::wstring id;
    std::wstring description;
    std::wstring requiredScreen;
    Risk risk = Risk::Low;
    double confidence = 0.0;
    bool changesSave = false;
    bool approved = false;
};

struct SafetyResult { bool allowed; std::wstring reason; };

class SafetyValidator {
public:
    SafetyResult validate(const ClubState& s, const ProposedAction& a, AgentMode mode) const {
        if (!s.fmDetected) return {false, L"FM26 not detected"};
        if (s.emergencyStop) return {false, L"Emergency stop active"};
        if (s.paused) return {false, L"Agent paused"};
        if (a.confidence < 0.80) return {false, L"Action confidence below 80%"};
        if (!a.requiredScreen.empty() && s.screen != a.requiredScreen) return {false, L"Wrong FM26 screen"};
        if (a.risk == Risk::Blocked) return {false, L"Action explicitly blocked"};
        if (a.changesSave && mode == AgentMode::Observe) return {false, L"Observe mode cannot change save"};
        if (a.changesSave && mode == AgentMode::Assist) return {false, L"Assist mode recommends only"};
        if (a.changesSave && mode == AgentMode::Confirm && !a.approved) return {false, L"Waiting for user confirmation"};
        return {true, L"Safety validation passed"};
    }
};

class DecisionEngine {
public:
    std::vector<Objective> objectives(const ClubState& s) const {
        std::vector<Objective> out;
        if (!s.fmDetected) { out.push_back({L"detect", L"Detect Football Manager 26", 100}); return out; }
        if (s.screen == L"UNKNOWN") out.push_back({L"vision", L"Improve screen recognition before any action", 100});
        out.push_back({L"finance", L"Protect club financial stability", 95});
        out.push_back({L"squad", L"Maintain a balanced, valuable squad", 85});
        out.push_back({L"market", L"Identify undervalued players with sporting and resale value", 80});
        out.push_back({L"staff", L"Improve staff quality within sustainable cost", 70});
        std::sort(out.begin(), out.end(), [](auto&a,auto&b){return a.priority>b.priority;});
        return out;
    }
};
