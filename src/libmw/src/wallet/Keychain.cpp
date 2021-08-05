#include <mw/wallet/Keychain.h>
#include <mw/crypto/Blinds.h>
#include <mw/crypto/Hasher.h>
#include <mw/common/Logger.h>
#include <mw/models/tx/OutputMask.h>

#define RESTORE_WINDOW 100

MW_NAMESPACE

bool Keychain::RewindOutput(const Output& output, mw::Coin& coin) const
{
    SecretKey t = Hashed(EHashTag::DERIVE, output.Ke().Mul(GetScanSecret()));
    if (t[0] != output.GetViewTag()) {
        return false;
    }

    PublicKey B = output.Ko().Sub(Hashed(EHashTag::OUT_KEY, t));

    // Check if B belongs to wallet
    uint32_t index = 0;
    if (!IsSpendPubKey(B, index)) {
        return false;
    }

    // Calc blinding factor and unmask nonce and amount.
    OutputMask mask = OutputMask::FromShared(t);
    uint64_t value = mask.MaskValue(output.GetMaskedValue());
    BigInt<16> n = mask.MaskNonce(output.GetMaskedNonce());

    if (mask.SwitchCommit(value) != output.GetCommitment()) {
        return false;
    }

    // Calculate Carol's sending key 's' and check that s*B ?= Ke
    StealthAddress wallet_addr = GetStealthAddress(index);
    SecretKey s = Hasher(EHashTag::SEND_KEY)
                      .Append(wallet_addr.A())
                      .Append(wallet_addr.B())
                      .Append(value)
                      .Append(n)
                      .hash();
    if (output.Ke() != wallet_addr.B().Mul(s)) {
        return false;
    }

    SecretKey private_key = Blinds()
        .Add(Hashed(EHashTag::OUT_KEY, t))
        .Add(GetSpendKey(index))
        .ToKey();

    coin.address_index = index;
    coin.key = boost::make_optional(std::move(private_key));
    coin.blind = boost::make_optional(mask.GetRawBlind());
    coin.amount = value;
    coin.commitment = output.GetCommitment();

    return true;
}

StealthAddress Keychain::GetStealthAddress(const uint32_t index) const
{
    assert(index < ULONG_MAX);
    m_addressIndexCounter = std::max(m_addressIndexCounter, index);

    PublicKey Bi = PublicKey::From(GetSpendKey(index));
    PublicKey Ai = Bi.Mul(m_scanSecret);

    return StealthAddress(Ai, Bi);
}

SecretKey Keychain::GetSpendKey(const uint32_t index) const
{
    assert(index < ULONG_MAX);

    SecretKey mi = Hasher(EHashTag::ADDRESS)
        .Append<uint32_t>(index)
        .Append(m_scanSecret)
        .hash();

    return Blinds().Add(m_spendSecret).Add(mi).ToKey();
}

bool Keychain::IsSpendPubKey(const PublicKey& spend_pubkey, uint32_t& index_out) const
{
    // Ensure pubkey cache is fully populated
    while (m_pubkeys.size() <= ((size_t)m_addressIndexCounter + RESTORE_WINDOW)) {
        uint32_t pubkey_index = (uint32_t)m_pubkeys.size();
        PublicKey pubkey = PublicKey::From(GetSpendKey(pubkey_index));

        m_pubkeys.insert({ std::move(pubkey), pubkey_index });
    }

    auto iter = m_pubkeys.find(spend_pubkey);
    if (iter != m_pubkeys.end()) {
        index_out = iter->second;
        return true;
    }

    return false;
}

bool Keychain::IsMine(const StealthAddress& address) const
{
    uint32_t index;
    if (IsSpendPubKey(address.GetSpendPubKey(), index)) {
        return GetStealthAddress(index) == address;
    }

    return false;
}

END_NAMESPACE