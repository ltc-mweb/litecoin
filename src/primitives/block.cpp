// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>
#include <util/strencodings.h>
#include <crypto/common.h>
#include <crypto/scrypt.h>
#include <script/interpreter.h>
#include <script/standard.h>
#include <util/strencodings.h>
#include <bech32.h>
#include <chainparams.h>

uint256 CBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

uint256 CBlockHeader::GetPoWHash() const
{
    uint256 thash;
    scrypt_1024_1_1_256(BEGIN(nVersion), BEGIN(thash));
    return thash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}

std::vector<libmw::PegIn> CBlock::GetPegInCoins() const noexcept
{
    if (!HasHogEx()) {
        return {};
    }

    std::vector<libmw::PegIn> pegins;

    // MW: Alternatively, we could just loop through the inputs on the HogEx transaction
    for (const CTransactionRef& pTx : vtx) {
        for (const CTxOut& output : pTx->vout) {
            int version;
            std::vector<uint8_t> program;
            if (output.scriptPubKey.IsWitnessProgram(version, program)) {
                if (version == Consensus::Mimblewimble::WITNESS_VERSION && program.size() == WITNESS_MW_PEGIN_SIZE) {
                    libmw::PegIn pegin;
                    pegin.amount = output.nValue;
                    std::move(program.begin(), program.begin() + WITNESS_MW_PEGIN_SIZE, pegin.commitment.begin());
                    pegins.push_back(std::move(pegin));
                }
            }
        }
    }

    return pegins;
}

std::vector<libmw::PegOut> CBlock::GetPegOutCoins() const noexcept
{
    if (!HasHogEx()) {
        return {};
    }

    std::vector<libmw::PegOut> pegouts;

    const CTransactionRef& pHogEx = vtx.back();
    for (const CTxOut& output : pHogEx->vout) {
        int version;
        std::vector<uint8_t> program;
        if (output.scriptPubKey.IsWitnessProgram(version, program)) {
            if (version == 0 && program.size() == WITNESS_V0_KEYHASH_SIZE) {
                std::vector<uint8_t> data = {0};
                data.reserve(33);
                ConvertBits<8, 5, true>([&](uint8_t c) { data.push_back(c); }, program.begin(), program.end());

                libmw::PegOut pegout;
                pegout.address = bech32::Encode(Params().Bech32HRP(), data);
                pegout.amount = output.nValue;
                pegouts.push_back(std::move(pegout));
            }
        }
    }

    return pegouts;
}