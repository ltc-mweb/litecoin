#pragma once

#include <mw/models/tx/UTXO.h>
#include <unordered_map>

struct CoinAction {
    bool IsSpend() const noexcept { return pUTXO == nullptr; }

    UTXO::CPtr pUTXO;
};

class CoinsViewUpdates
{
public:
    CoinsViewUpdates() = default;

    void AddUTXO(const UTXO::CPtr& pUTXO)
    {
        AddAction(pUTXO->GetCommitment(), CoinAction{pUTXO});
    }

    void SpendUTXO(const Commitment& commitment)
    {
        AddAction(commitment, CoinAction{nullptr});
    }

    const std::unordered_map<Commitment, std::vector<CoinAction>>& GetActions() const noexcept { return m_actions; }

    std::vector<CoinAction> GetActions(const Commitment& commitment) const noexcept
    {
        auto iter = m_actions.find(commitment);
        if (iter != m_actions.cend()) {
            return iter->second;
        }

        return {};
    }

    void Clear() noexcept
    {
        m_actions.clear();
    }

private:
    void AddAction(const Commitment& commitment, CoinAction&& action)
    {
        auto iter = m_actions.find(commitment);
        if (iter != m_actions.end()) {
            std::vector<CoinAction>& actions = iter->second;
            actions.emplace_back(std::move(action));
        } else {
            std::vector<CoinAction> actions;
            actions.emplace_back(std::move(action));
            m_actions.insert({commitment, actions});
        }
    }

    std::unordered_map<Commitment, std::vector<CoinAction>> m_actions;
};