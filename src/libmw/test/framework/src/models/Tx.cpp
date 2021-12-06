#include <test_framework/models/Tx.h>
#include <mw/crypto/Pedersen.h>

test::Tx test::Tx::CreatePegIn(const CAmount amount, const CAmount fee)
{
    BlindingFactor txOffset = BlindingFactor::Random();
    BlindingFactor stealth_offset = BlindingFactor::Random();

    SecretKey sender_privkey = SecretKey::Random();
    test::TxOutput output = test::TxOutput::Create(
        sender_privkey,
        StealthAddress::Random(),
        amount
    );

    BlindingFactor kernelBF = Pedersen::AddBlindingFactors({output.GetBlind()}, {txOffset});
    BlindingFactor stealth_blind = Pedersen::AddBlindingFactors({sender_privkey}, {stealth_offset});
    Kernel kernel = Kernel::Create(kernelBF, stealth_blind, fee, amount, boost::none, boost::none);

    auto pTx = mw::Transaction::Create(txOffset, stealth_offset, {}, {output.GetOutput()}, {kernel});
    return Tx{ pTx, { output } };
}