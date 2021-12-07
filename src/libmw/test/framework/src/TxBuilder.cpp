#include <test_framework/TxBuilder.h>

#include <mw/crypto/Keys.h>

TEST_NAMESPACE

TxBuilder::TxBuilder()
    : m_amount{ 0 }, m_kernelOffset{}, m_ownerOffset{}, m_inputs{}, m_outputs{}, m_kernels{}
{

}

TxBuilder& TxBuilder::AddInput(const TxOutput& input)
{
    return AddInput(input.GetAmount(), input.GetBlind());
}

TxBuilder& TxBuilder::AddInput(const CAmount amount, const BlindingFactor& blind)
{
    return AddInput(amount, SecretKey::Random(), blind);
}

TxBuilder& TxBuilder::AddInput(
    const CAmount amount,
    const SecretKey& privkey,
    const BlindingFactor& blind)
{
    m_kernelOffset.Sub(blind);
    m_ownerOffset.Sub(privkey);

    SecretKey input_key = SecretKey::Random();
    m_ownerOffset.Add(input_key);

    Commitment commitment = Commitment::Blinded(blind, amount);
    m_inputs.push_back(Input::Create(commitment, input_key, privkey));
    m_amount += (int64_t)amount;
    return *this;
}

TxBuilder& TxBuilder::AddOutput(const CAmount amount)
{
    return AddOutput(amount, SecretKey::Random(), StealthAddress::Random());
}

TxBuilder& TxBuilder::AddOutput(
    const CAmount amount,
    const SecretKey& sender_privkey,
    const StealthAddress& receiver_addr)
{
    TxOutput output = TxOutput::Create(sender_privkey, receiver_addr, amount);
    m_kernelOffset.Add(output.GetBlind());
    m_ownerOffset.Add(sender_privkey);

    m_outputs.push_back(std::move(output));
    m_amount -= (int64_t)amount;
    return *this;
}

TxBuilder& TxBuilder::AddPlainKernel(const CAmount fee, const bool add_stealth_excess)
{
    BlindingFactor kernel_excess = BlindingFactor::Random();
    m_kernelOffset.Sub(kernel_excess);

    boost::optional<BlindingFactor> stealth_excess = boost::none;
    if (add_stealth_excess) {
        SecretKey offset = SecretKey::Random();
        m_ownerOffset.Sub(offset);
        stealth_excess = boost::make_optional(offset);
    }

    Kernel kernel = Kernel::Create(kernel_excess, stealth_excess, fee, boost::none, boost::none, boost::none);

    m_kernels.push_back(std::move(kernel));
    m_amount -= fee;
    return *this;
}

TxBuilder& TxBuilder::AddPeginKernel(const CAmount amount, const boost::optional<CAmount>& fee, const bool add_stealth_excess)
{
    BlindingFactor kernel_excess = BlindingFactor::Random();
    m_kernelOffset.Sub(kernel_excess);

    boost::optional<BlindingFactor> stealth_excess = boost::none;
    if (add_stealth_excess) {
        BlindingFactor offset = BlindingFactor::Random();
        m_ownerOffset.Sub(offset);
        stealth_excess = boost::make_optional(offset);
    }

    Kernel kernel = Kernel::Create(kernel_excess, stealth_excess, fee, amount, boost::none, boost::none);

    m_kernels.push_back(std::move(kernel));
    m_amount += amount;
    return *this;
}

TxBuilder& TxBuilder::AddPegoutKernel(const CAmount amount, const CAmount fee, const bool add_stealth_excess)
{
    BlindingFactor kernel_excess = BlindingFactor::Random();
    m_kernelOffset.Sub(kernel_excess);
    std::vector<uint8_t> ltc_address = SecretKey::Random().vec();

    boost::optional<BlindingFactor> stealth_excess = boost::none;
    if (add_stealth_excess) {
        BlindingFactor offset = BlindingFactor::Random();
        m_ownerOffset.Sub(offset);
        stealth_excess = boost::make_optional(offset);
    }

    Kernel kernel = Kernel::Create(kernel_excess, stealth_excess, fee, boost::none, PegOutCoin(amount, CScript(ltc_address.begin(), ltc_address.end())), boost::none);

    m_kernels.push_back(std::move(kernel));
    m_amount -= amount + fee;
    return *this;
}

Tx TxBuilder::Build()
{
    assert(m_amount == 0);

    std::vector<Output> outputs;
    std::transform(
        m_outputs.cbegin(), m_outputs.cend(),
        std::back_inserter(outputs),
        [](const TxOutput& output) { return output.GetOutput(); }
    );

    auto pTransaction = mw::Transaction::Create(
        m_kernelOffset.Total(),
        m_ownerOffset.Total(),
        m_inputs,
        outputs,
        m_kernels
    );
    return Tx{ pTransaction, m_outputs };
}

END_NAMESPACE