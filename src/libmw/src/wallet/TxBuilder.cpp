#include <mw/wallet/TxBuilder.h>
#include "WalletUtil.h"

#include <mw/consensus/Weight.h>
#include <mw/crypto/Blinds.h>
#include <mw/exceptions/InsufficientFundsException.h>
#include <numeric>

mw::Transaction::CPtr TxBuilder::BuildTx(
    const std::vector<mw::Coin>& input_coins,
    const std::vector<mw::Recipient>& recipients,
    const std::vector<PegOutCoin>& pegouts,
    const boost::optional<CAmount>& pegin_amount,
    const CAmount fee)
{
    if (pegouts.size() > 1) {
        throw std::runtime_error("Only supporting one pegout at this time.");
    }

    CAmount pegout_total = std::accumulate(
        pegouts.cbegin(), pegouts.cend(), (CAmount)0,
        [](CAmount sum, const PegOutCoin& pegout) { return sum + pegout.GetAmount(); }
    );

    CAmount recipient_total = std::accumulate(
        recipients.cbegin(), recipients.cend(), (CAmount)0,
        [](CAmount sum, const mw::Recipient& recipient) { return sum + recipient.amount; }
    );

    // Get input coins
    LOG_INFO_F(
        "Creating Txs: Inputs({}), pegins({}), pegouts({}), recipient_total({}), fee({})",
        WalletUtil::TotalAmount(input_coins),
        pegin_amount.value_or(0),
        pegout_total,
        recipient_total,
        fee
    );
    if ((WalletUtil::TotalAmount(input_coins) + pegin_amount.value_or(0)) != (pegout_total + recipient_total + fee)) {
        ThrowInsufficientFunds("Total amount mismatch");
    }

    // Sign inputs
    std::vector<Input> inputs = WalletUtil::SignInputs(input_coins);

    // Create outputs
    TxBuilder::Outputs outputs = CreateOutputs(recipients);

    // Total kernel offset is split between raw kernel_offset and the kernel's blinding factor.
    // sum(output.blind) - sum(input.blind) = kernel_offset + sum(kernel.blind)
    std::vector<BlindingFactor> input_blinds = WalletUtil::GetBlindingFactors(input_coins);
    BlindingFactor kernel_offset = Random::CSPRNG<32>();
    BlindingFactor kernel_blind = Blinds()
        .Add(outputs.total_blind)
        .Sub(input_blinds)
        .Sub(kernel_offset)
        .Total();

    // Create the kernel
    Kernel kernel = Kernel::Create(
        kernel_blind,
        fee,
        pegin_amount,
        pegouts.empty() ? boost::none : boost::make_optional(pegouts.front()),
        boost::none
    );

    // FUTURE: Only necessary when none of the addresses are owned by this wallet?
    // MW: TODO - OwnerSignature won't be enough for peg-ins with no MWEB inputs or MWEB change.
    // Would be better to just include change when none of the MWEB inputs are your previous change (i.e. self-generated blinds)
    SecretKey owner_sig_key = Random::CSPRNG<32>();
    SignedMessage owner_sig = Schnorr::SignMessage(owner_sig_key, kernel.GetHash());

    // Total owner offset is split between raw owner_offset and the owner_sig's key.
    // sum(output.sender_key) - sum(input.key) = owner_offset + sum(owner_sig.key)
    std::vector<SecretKey> input_keys = WalletUtil::GetKeys(input_coins);
    BlindingFactor owner_offset = Blinds()
        .Add(outputs.total_key)
        .Sub(input_keys)
        .Sub(owner_sig_key)
        .Total();

    // Build the transaction
    return mw::Transaction::Create(
        std::move(kernel_offset),
        std::move(owner_offset),
        std::move(inputs),
        std::move(outputs.outputs),
        std::vector<Kernel>{ std::move(kernel) },
        std::vector<SignedMessage>{ std::move(owner_sig) }
    );
}

TxBuilder::Outputs TxBuilder::CreateOutputs(const std::vector<mw::Recipient>& recipients)
{
    Blinds output_blinds;
    Blinds output_keys;
    std::vector<Output> outputs;
    std::transform(
        recipients.cbegin(), recipients.cend(), std::back_inserter(outputs),
        [&output_blinds, &output_keys](const mw::Recipient& recipient) {
            BlindingFactor blind;
            SecretKey ephemeral_key = Random::CSPRNG<32>();
            Output output = Output::Create(
                blind,
                ephemeral_key,
                recipient.address,
                recipient.amount
            );

            output_blinds.Add(blind);
            output_keys.Add(ephemeral_key);
            return output;
        }
    );

    return TxBuilder::Outputs{
        output_blinds.Total(),
        SecretKey(output_keys.Total().data()),
        std::move(outputs)
    };
}