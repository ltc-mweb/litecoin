#include <test_framework/models/Tx.h>
#include <mw/crypto/Pedersen.h>

test::Tx test::Tx::CreatePegIn(const CAmount amount, const CAmount fee)
{
    BlindingFactor txOffset = BlindingFactor::Random();

    SecretKey sender_privkey = SecretKey::Random();
    test::TxOutput output = test::TxOutput::Create(
        sender_privkey,
        StealthAddress::Random(),
        amount
    );

    BlindingFactor kernelBF = Pedersen::AddBlindingFactors({ output.GetBlind() }, { txOffset });
    Kernel kernel = Kernel::Create(kernelBF, fee, amount, boost::none, boost::none);

    auto pTx = mw::Transaction::Create(txOffset, sender_privkey, {}, { output.GetOutput() }, { kernel }, {});
    return Tx{ pTx, { output } };
}