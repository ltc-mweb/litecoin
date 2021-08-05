#include <mw/node/BlockValidator.h>
#include <mw/exceptions/ValidationException.h>
#include <unordered_map>

bool BlockValidator::ValidateBlock(
    const mw::Block::CPtr& pBlock,
    const mw::Hash& mweb_hash,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins) noexcept
{
    assert(pBlock != nullptr);

    try {
        BlockValidator().Validate(pBlock, mweb_hash, pegInCoins, pegOutCoins);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to validate {}. Error: {}", *pBlock, e);
    }

    return false;
}

void BlockValidator::Validate(
    const mw::Block::CPtr& pBlock,
    const mw::Hash& mweb_hash,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins)
{
    assert(pBlock != nullptr);

    if (pBlock->GetHash() != mweb_hash) {
        ThrowValidation(EConsensusError::HASH_MISMATCH);
    }

    pBlock->Validate();

    ValidatePegInCoins(pBlock, pegInCoins);
    ValidatePegOutCoins(pBlock, pegOutCoins);
}

void BlockValidator::ValidatePegInCoins(
    const mw::Block::CPtr& pBlock,
    const std::vector<PegInCoin>& pegInCoins)
{
    std::unordered_map<Commitment, CAmount> pegInAmounts;
    std::for_each(
        pegInCoins.cbegin(), pegInCoins.cend(),
        [&pegInAmounts](const PegInCoin& coin) {
            pegInAmounts.insert({ coin.GetCommitment(), coin.GetAmount() });
        }
    );

    auto pegin_coins = pBlock->GetPegIns();
    if (pegin_coins.size() != pegInAmounts.size()) {
        ThrowValidation(EConsensusError::PEGIN_MISMATCH);
    }

    for (const auto& pegin : pegin_coins) {
        auto pIter = pegInAmounts.find(pegin.GetCommitment());
        if (pIter == pegInAmounts.end() || pegin.GetAmount() != pIter->second) {
            ThrowValidation(EConsensusError::PEGIN_MISMATCH);
        }
    }
}

void BlockValidator::ValidatePegOutCoins(
    const mw::Block::CPtr& pBlock,
    const std::vector<PegOutCoin>& pegOutCoins)
{
    std::map<CScript, CAmount> pegOutAmounts;
    std::for_each(
        pegOutCoins.cbegin(), pegOutCoins.cend(),
        [&pegOutAmounts](const PegOutCoin& coin) {
            pegOutAmounts.insert({ coin.GetScriptPubKey(), coin.GetAmount() });
        }
    );

    auto pegout_coins = pBlock->GetPegOuts();
    if (pegout_coins.size() != pegOutAmounts.size()) {
        ThrowValidation(EConsensusError::PEGOUT_MISMATCH);
    }

    for (const auto& pegout : pegout_coins) {
        auto pIter = pegOutAmounts.find(pegout.GetScriptPubKey());
        if (pIter == pegOutAmounts.end() || pegout.GetAmount() != pIter->second) {
            ThrowValidation(EConsensusError::PEGOUT_MISMATCH);
        }
    }
}