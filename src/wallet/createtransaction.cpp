#include <wallet/createtransaction.h>
#include <wallet/fees.h>
#include <mweb/mweb_transact.h>
#include <policy/policy.h>
#include <policy/fees.h>
#include <policy/rbf.h>
#include <util/system.h>
#include <consensus/validation.h>
#include <validation.h>
#include <txmempool.h>
#include <logging.h>
#include <numeric>
#include <boost/optional.hpp>

static bool IsCurrentForAntiFeeSniping(interfaces::Chain::Lock& locked_chain)
{
    if (IsInitialBlockDownload()) {
        return false;
    }
    constexpr int64_t MAX_ANTI_FEE_SNIPING_TIP_AGE = 8 * 60 * 60; // in seconds
    if (chainActive.Tip()->GetBlockTime() < (GetTime() - MAX_ANTI_FEE_SNIPING_TIP_AGE)) {
        return false;
    }
    return true;
}

/**
 * Return a height-based locktime for new transactions (uses the height of the
 * current chain tip unless we are not synced with the current chain
 */
static uint32_t GetLocktimeForNewTransaction(interfaces::Chain::Lock& locked_chain)
{
    uint32_t const height = locked_chain.getHeight().get_value_or(-1);
    uint32_t locktime;
    // Discourage fee sniping.
    //
    // For a large miner the value of the transactions in the best block and
    // the mempool can exceed the cost of deliberately attempting to mine two
    // blocks to orphan the current best block. By setting nLockTime such that
    // only the next block can include the transaction, we discourage this
    // practice as the height restricted and limited blocksize gives miners
    // considering fee sniping fewer options for pulling off this attack.
    //
    // A simple way to think about this is from the wallet's point of view we
    // always want the blockchain to move forward. By setting nLockTime this
    // way we're basically making the statement that we only want this
    // transaction to appear in the next block; we don't want to potentially
    // encourage reorgs by allowing transactions to appear at lower heights
    // than the next block in forks of the best chain.
    //
    // Of course, the subsidy is high enough, and transaction volume low
    // enough, that fee sniping isn't a problem yet, but by implementing a fix
    // now we ensure code won't be written that makes assumptions about
    // nLockTime that preclude a fix later.
    if (IsCurrentForAntiFeeSniping(locked_chain)) {
        locktime = height;

        // Secondly occasionally randomly pick a nLockTime even further back, so
        // that transactions that are delayed after signing for whatever reason,
        // e.g. high-latency mix networks and some CoinJoin implementations, have
        // better privacy.
        if (GetRandInt(10) == 0)
            locktime = std::max(0, (int)locktime - GetRandInt(100));
    } else {
        // If our chain is lagging behind, we can't discourage fee sniping nor help
        // the privacy of high-latency transactions. To avoid leaking a potentially
        // unique "nLockTime fingerprint", set nLockTime to a constant.
        locktime = 0;
    }
    assert(locktime <= height);
    assert(locktime < LOCKTIME_THRESHOLD);
    return locktime;
}

bool VerifyRecipients(const std::vector<CRecipient>& vecSend, std::string& strFailReason)
{
    if (vecSend.empty()) {
        strFailReason = _("Transaction must have at least one recipient");
        return false;
    }

    CAmount nValue = 0;
    for (const auto& recipient : vecSend) {
        if (recipient.IsMWEB() && vecSend.size() > 1) {
            strFailReason = _("Only one MWEB recipient supported at this time");
            return false;
        }

        if (recipient.IsMWEB() && recipient.fSubtractFeeFromAmount) {
            strFailReason = _("Subtract fee from amount is not yet supported for MWEB transactions");
            return false;
        }

        if (nValue < 0 || recipient.nAmount < 0) {
            strFailReason = _("Transaction amounts must not be negative");
            return false;
        }

        nValue += recipient.nAmount;
    }

    return true;
}

bool AddRecipientOutputs(
    CMutableTransaction& txNew,
    const std::vector<CRecipient>& vecSend,
    CoinSelectionParams& coin_selection_params,
    CAmount nFeeRet,
    unsigned int nSubtractFeeFromAmount,
    std::string& strFailReason)
{
    // vouts to the payees
    coin_selection_params.tx_noinputs_size = 11; // Static vsize overhead + outputs vsize. 4 nVersion, 4 nLocktime, 1 input count, 1 output count, 1 witness overhead (dummy, flag, stack size)
    coin_selection_params.input_preference = InputPreference::PREFER_LTC;

    bool fFirst = true;
    for (const auto& recipient : vecSend) {
        if (recipient.IsMWEB()) {
            LogPrintf("Prefer MWEB\n");
            coin_selection_params.input_preference = InputPreference::PREFER_MWEB;
            continue;
        }

        CTxOut txout(recipient.nAmount, recipient.GetScript());

        if (recipient.fSubtractFeeFromAmount) {
            assert(nSubtractFeeFromAmount != 0);
            txout.nValue -= nFeeRet / nSubtractFeeFromAmount; // Subtract fee equally from each selected recipient

            if (fFirst) // first receiver pays the remainder not divisible by output count
            {
                fFirst = false;
                txout.nValue -= nFeeRet % nSubtractFeeFromAmount;
            }
        }
        // Include the fee cost for outputs. Note this is only used for BnB right now
        coin_selection_params.tx_noinputs_size += ::GetSerializeSize(txout, PROTOCOL_VERSION);

        if (IsDust(txout, ::dustRelayFee)) {
            if (recipient.fSubtractFeeFromAmount && nFeeRet > 0) {
                if (txout.nValue < 0)
                    strFailReason = _("The transaction amount is too small to pay the fee");
                else
                    strFailReason = _("The transaction amount is too small to send after the fee has been deducted");
            } else
                strFailReason = _("Transaction amount too small");
            return false;
        }
        txNew.vout.push_back(txout);
    }

    return true;
}


bool SelectCoins(
    CWallet& wallet,
    const std::vector<COutputCoin>& vAvailableCoins,
    const CAmount& nTargetValue,
    std::set<CInputCoin>& setCoinsRet,
    CAmount& nValueRet,
    const CCoinControl& coin_control,
    CoinSelectionParams& coin_selection_params,
    bool& bnb_used)
{
    // BnB only supported for non-MWEB.
    if (coin_selection_params.input_preference == InputPreference::PREFER_LTC && coin_selection_params.use_bnb) {
        CoinSelectionParams params2 = coin_selection_params;
        params2.input_preference = InputPreference::LTC_ONLY;

        // First try SelectCoins with LTC_ONLY since those are the preferred inputs.
        if (wallet.SelectCoins(vAvailableCoins, nTargetValue, setCoinsRet, nValueRet, coin_control, params2, bnb_used)) {
            return true;
        }
    }

    bnb_used = false;
    coin_selection_params.use_bnb = false;
    if (coin_selection_params.input_preference == InputPreference::PREFER_LTC) {
        CoinSelectionParams params2 = coin_selection_params;
        params2.input_preference = InputPreference::LTC_ONLY;

        // First try SelectCoins with LTC_ONLY since those are the preferred inputs.
        if (wallet.SelectCoins(vAvailableCoins, nTargetValue, setCoinsRet, nValueRet, coin_control, params2, bnb_used)) {
            return true;
        }

        params2.input_preference = InputPreference::MWEB_ONLY;
        params2.change_type = OutputType::MWEB;

        // Then try SelectCoins with MWEB_ONLY so we can avoid mixing input types.
        if (wallet.SelectCoins(vAvailableCoins, nTargetValue, setCoinsRet, nValueRet, coin_control, params2, bnb_used)) {
            return true;
        }

        // MW: TODO - Since the preferred method failed, we may have to add on an additional fee?
    } else if (coin_selection_params.input_preference == InputPreference::PREFER_MWEB) {
        CoinSelectionParams params2 = coin_selection_params;
        params2.input_preference = InputPreference::MWEB_ONLY;
        params2.change_type = OutputType::MWEB;

        // First try SelectCoins with MWEB_ONLY since those are the preferred inputs.
        if (wallet.SelectCoins(vAvailableCoins, nTargetValue, setCoinsRet, nValueRet, coin_control, params2, bnb_used)) {
            return true;
        }

        params2.input_preference = InputPreference::LTC_ONLY;
        params2.change_type = coin_selection_params.change_type;

        // Then try SelectCoins with LTC_ONLY so we can avoid mixing input types.
        if (wallet.SelectCoins(vAvailableCoins, nTargetValue, setCoinsRet, nValueRet, coin_control, params2, bnb_used)) {
            return true;
        }

        // MW: TODO - Since the preferred method failed, we may have to add on an additional fee?
    }

    return wallet.SelectCoins(vAvailableCoins, nTargetValue, setCoinsRet, nValueRet, coin_control, coin_selection_params, bnb_used);
}

static bool IsChangeOnMWEB(const MWEB::TxType& mweb_type, const CCoinControl& coin_control)
{
    if (mweb_type == MWEB::TxType::MWEB_TO_MWEB || mweb_type == MWEB::TxType::PEGOUT) {
        return true;
    }

    if (mweb_type == MWEB::TxType::PEGIN) {
        return coin_control.destChange.type() == typeid(CNoDestination)
            || coin_control.destChange.type() == typeid(MWEBAddress);
    }

    return false;
}

static bool ContainsPegIn(const MWEB::TxType& mweb_type, const std::set<CInputCoin>& setCoins)
{
    if (mweb_type == MWEB::TxType::PEGIN) {
        return true;
    }
    
    if (mweb_type == MWEB::TxType::PEGOUT) {
        return std::any_of(
            setCoins.cbegin(), setCoins.cend(),
            [](const CInputCoin& coin) { return !coin.IsMWEB(); });
    }

    return false;
}

bool CreateTransactionEx(
    CWallet& wallet,
    interfaces::Chain::Lock& locked_chain,
    const std::vector<CRecipient>& vecSend,
    CTransactionRef& tx,
    CReserveKey& reservekey,
    CAmount& nFeeRet,
    int& nChangePosInOut,
    std::string& strFailReason,
    const CCoinControl& coin_control,
    bool sign)
{
    if (!VerifyRecipients(vecSend, strFailReason)) {
        return false;
    }

    CAmount nValue = std::accumulate(
        vecSend.cbegin(), vecSend.cend(), CAmount(0),
        [](CAmount amt, const CRecipient& recipient) { return amt + recipient.nAmount; }
    );

    unsigned int nSubtractFeeFromAmount = std::count_if(vecSend.cbegin(), vecSend.cend(),
        [](const CRecipient& recipient) { return recipient.fSubtractFeeFromAmount; }
    );

    int nChangePosRequest = nChangePosInOut;

    CMutableTransaction txNew;
    txNew.nLockTime = GetLocktimeForNewTransaction(locked_chain);

    FeeCalculation feeCalc;
    CAmount nFeeNeeded;

    CAmount mweb_fee = 0;
    bool change_on_mweb = false;

    int nBytes;
    {
        std::set<CInputCoin> setCoins;
        auto locked_chain = wallet.chain().lock();
        LOCK(wallet.cs_wallet);
        {
            std::vector<COutputCoin> vAvailableCoins;
            wallet.AvailableCoins(*locked_chain, vAvailableCoins, true, &coin_control);

            // Create change script that will be used if we need change
            // TODO: pass in scriptChange instead of reservekey so
            // change transaction isn't always pay-to-bitcoin-address
            DestinationScript scriptChange;

            // coin control: send change to custom address
            if (!boost::get<CNoDestination>(&coin_control.destChange)) {
                scriptChange = DestinationScript(coin_control.destChange);
            } else { // no coin control: send change to newly generated address
                // Note: We use a new key here to keep it from being obvious which side is the change.
                //  The drawback is that by not reusing a previous key, the change may be lost if a
                //  backup is restored, if the backup doesn't have the new private key for the change.
                //  If we reused the old key, it would be possible to add code to look for and
                //  rediscover unknown transactions that were written with keys of ours to recover
                //  post-backup change.

                // Reserve a new key pair from key pool
                if (!wallet.CanGetAddresses(true)) {
                    strFailReason = _("Can't generate a change-address key. No keys in the internal keypool and can't generate any keys.");
                    return false;
                }
                CPubKey vchPubKey;
                bool ret;
                ret = reservekey.GetReservedKey(vchPubKey, true);
                if (!ret) {
                    strFailReason = _("Keypool ran out, please call keypoolrefill first");
                    return false;
                }

                const OutputType change_type = wallet.TransactionChangeType(coin_control.m_change_type ? *coin_control.m_change_type : wallet.m_default_change_type, vecSend);

                wallet.LearnRelatedScripts(vchPubKey, change_type);
                scriptChange = DestinationScript(GetDestinationForKey(vchPubKey, change_type));
            }

            CTxOut change_prototype_txout(0, scriptChange.IsMWEB() ? CScript() : scriptChange.GetScript());

            CFeeRate discard_rate = GetDiscardRate(wallet, ::feeEstimator);

            // Get the fee rate to use effective values in coin selection
            CFeeRate nFeeRateNeeded = GetMinimumFeeRate(wallet, coin_control, ::mempool, ::feeEstimator, &feeCalc);

            CoinSelectionParams coin_selection_params; // Parameters for coin selection, init with dummy
            coin_selection_params.change_type = coin_control.m_change_type;
            coin_selection_params.change_output_size = GetSerializeSize(change_prototype_txout);

            // BnB selector is the only selector used when this is true.
            // That should only happen on the first pass through the loop.
            coin_selection_params.use_bnb = nSubtractFeeFromAmount == 0; // If we are doing subtract fee from recipient, then don't use BnB

            nFeeRet = 0;
            bool pick_new_inputs = true;
            CAmount nValueIn = 0;

            // Start with no fee and loop until there is enough fee
            while (true) {
                nChangePosInOut = nChangePosRequest;
                txNew.vin.clear();
                txNew.vout.clear();
                mweb_fee = 0;

                CAmount nValueToSelect = nValue;
                if (nSubtractFeeFromAmount == 0)
                    nValueToSelect += nFeeRet;

                if (!AddRecipientOutputs(txNew, vecSend, coin_selection_params, nFeeRet, nSubtractFeeFromAmount, strFailReason)) {
                    return false;
                }

                // Choose coins to use
                bool bnb_used = false;
                if (pick_new_inputs) {
                    nValueIn = 0;
                    setCoins.clear();
                    int change_spend_size = CalculateMaximumSignedInputSize(change_prototype_txout, &wallet);
                    // If the wallet doesn't know how to sign change output, assume p2sh-p2wpkh
                    // as lower-bound to allow BnB to do it's thing
                    if (change_spend_size == -1) {
                        coin_selection_params.change_spend_size = DUMMY_NESTED_P2WPKH_INPUT_SIZE;
                    } else {
                        coin_selection_params.change_spend_size = (size_t)change_spend_size;
                    }
                    coin_selection_params.effective_fee = nFeeRateNeeded;
                    if (!SelectCoins(wallet, vAvailableCoins, nValueToSelect, setCoins, nValueIn, coin_control, coin_selection_params, bnb_used)) {
                        strFailReason = _("Insufficient funds");
                        return false;
                    }
                }

                MWEB::TxType mweb_type = MWEB::GetTxType(vecSend, std::vector<CInputCoin>(setCoins.begin(), setCoins.end()));
                if (mweb_type != MWEB::TxType::LTC_TO_LTC) {
                    size_t mweb_outputs;
                    if (mweb_type == MWEB::TxType::MWEB_TO_MWEB) {
                        mweb_outputs = 2;
                    } else {
                        // Pegins have 1 output for the mweb receiver, but no mweb change
                        // Pegouts have 1 mweb change output, but no mweb receiver
                        mweb_outputs = 1;
                    }

                    size_t mweb_weight = 3 + (18 * mweb_outputs);
                    mweb_fee = 100'000 * mweb_weight; // Hardcoded for now

                    // We don't support multiple recipients for MWEB txs yet,
                    // so the only possible LTC outputs are pegins & change.
                    // Both of those are added after this, so clear outputs for now. 
                    txNew.vout.clear();
                    nChangePosInOut = -1;
                } else {
                    mweb_fee = 0;
                }

                change_on_mweb = IsChangeOnMWEB(mweb_type, coin_control);

                const CAmount nChange = nValueIn - nValueToSelect;
                if (nChange > 0 && !change_on_mweb) {
                    // Fill a vout to ourself
                    CTxOut newTxOut(nChange, scriptChange.IsMWEB() ? CScript() : scriptChange.GetScript());

                    // Never create dust outputs; if we would, just
                    // add the dust to the fee.
                    // The nChange when BnB is used is always going to go to fees.
                    if (IsDust(newTxOut, discard_rate) || bnb_used) {
                        nChangePosInOut = -1;
                        nFeeRet += nChange;
                    } else {
                        if (nChangePosInOut == -1) {
                            // Insert change txn at random position:
                            nChangePosInOut = GetRandInt(txNew.vout.size() + 1);
                        } else if ((unsigned int)nChangePosInOut > txNew.vout.size()) {
                            strFailReason = _("Change index out of range");
                            return false;
                        }

                        std::vector<CTxOut>::iterator position = txNew.vout.begin() + nChangePosInOut;
                        txNew.vout.insert(position, newTxOut);
                    }
                } else {
                    nChangePosInOut = -1;
                }

                if (ContainsPegIn(mweb_type, setCoins)) {
                    CScript dummy_pegin_script;
                    dummy_pegin_script << CScript::EncodeOP_N(Consensus::Mimblewimble::WITNESS_VERSION);
                    dummy_pegin_script << std::vector<uint8_t>(33);
                    txNew.vout.push_back(CTxOut(0, dummy_pegin_script));
                }

                // Dummy fill vin for maximum size estimation
                //
                for (const auto& coin : setCoins) {
                    if (!coin.IsMWEB()) {
                        txNew.vin.push_back(CTxIn(coin.outpoint, CScript()));
                    }
                }

                nBytes = CalculateMaximumSignedTxSize(CTransaction(txNew), &wallet, coin_control.fAllowWatchOnly);
                if (nBytes < 0) {
                    LogPrintf("Transaction failed to sign: %s", CTransaction(txNew).ToString().c_str());
                    strFailReason = _("Signing transaction failed");
                    return false;
                }

                nFeeNeeded = GetMinimumFee(wallet, nBytes, coin_control, ::mempool, ::feeEstimator, &feeCalc) + mweb_fee;
                if (feeCalc.reason == FeeReason::FALLBACK && !wallet.m_allow_fallback_fee) {
                    // eventually allow a fallback fee
                    strFailReason = _("Fee estimation failed. Fallbackfee is disabled. Wait a few blocks or enable -fallbackfee.");
                    return false;
                }

                // If we made it here and we aren't even able to meet the relay fee on the next pass, give up
                // because we must be at the maximum allowed fee.
                if (nFeeNeeded < ::minRelayTxFee.GetFee(nBytes)) { // MW: TODO - Also include MWEB weight in minRelayTxFee calculation
                    strFailReason = _("Transaction too large for fee policy");
                    return false;
                }

                if (nFeeRet >= nFeeNeeded) {
                    // Reduce fee to only the needed amount if possible. This
                    // prevents potential overpayment in fees if the coins
                    // selected to meet nFeeNeeded result in a transaction that
                    // requires less fee than the prior iteration.

                    // If we have no change and a big enough excess fee, then
                    // try to construct transaction again only without picking
                    // new inputs. We now know we only need the smaller fee
                    // (because of reduced tx size) and so we should add a
                    // change output. Only try this once.
                    if (nChangePosInOut == -1 && nSubtractFeeFromAmount == 0 && pick_new_inputs && (mweb_type == MWEB::TxType::LTC_TO_LTC || mweb_type == MWEB::TxType::PEGIN)) {
                        unsigned int tx_size_with_change = nBytes + coin_selection_params.change_output_size + 2; // Add 2 as a buffer in case increasing # of outputs changes compact size
                        CAmount fee_needed_with_change = GetMinimumFee(wallet, tx_size_with_change, coin_control, ::mempool, ::feeEstimator, nullptr);
                        fee_needed_with_change += mweb_fee;
                        CAmount minimum_value_for_change = GetDustThreshold(change_prototype_txout, discard_rate);
                        if (nFeeRet >= fee_needed_with_change + minimum_value_for_change) {
                            pick_new_inputs = false;
                            nFeeRet = fee_needed_with_change;
                            continue;
                        }
                    }

                    // If we have change output already, just increase it
                    if (nFeeRet > nFeeNeeded && nSubtractFeeFromAmount == 0) {
                        CAmount extraFeePaid = nFeeRet - nFeeNeeded;

                        if (nChangePosInOut != -1) {
                            std::vector<CTxOut>::iterator change_position = txNew.vout.begin() + nChangePosInOut;
                            change_position->nValue += extraFeePaid;
                            nFeeRet -= extraFeePaid;
                        } else if (change_on_mweb) {
                            nFeeRet -= extraFeePaid;
                        }
                    }

                    LogPrintf("nFeeRet: %d, nFeeNeeded: %d (MWEB: %d), nChangePosInOut: %d\n", nFeeRet, nFeeNeeded, mweb_fee, nChangePosInOut);
                    if (nFeeRet)
                    break; // Done, enough fee included.
                } else if (!pick_new_inputs) {
                    // This shouldn't happen, we should have had enough excess
                    // fee to pay for the new output and still meet nFeeNeeded
                    // Or we should have just subtracted fee from recipients and
                    // nFeeNeeded should not have changed
                    strFailReason = _("Transaction fee and change calculation failed");
                    return false;
                }

                // Try to reduce change to include necessary fee
                if (nChangePosInOut != -1 && nSubtractFeeFromAmount == 0) {
                    CAmount additionalFeeNeeded = nFeeNeeded - nFeeRet;
                    std::vector<CTxOut>::iterator change_position = txNew.vout.begin() + nChangePosInOut;
                    // Only reduce change if remaining amount is still a large enough output.
                    if (change_position->nValue >= MIN_FINAL_CHANGE + additionalFeeNeeded) {
                        change_position->nValue -= additionalFeeNeeded;
                        nFeeRet += additionalFeeNeeded;
                        break; // Done, able to increase fee from change
                    }
                }

                // If subtracting fee from recipients, we now know what fee we
                // need to subtract, we have no reason to reselect inputs
                if (nSubtractFeeFromAmount > 0) {
                    pick_new_inputs = false;
                }

                // Include more fee and try again.
                nFeeRet = nFeeNeeded;
                coin_selection_params.use_bnb = false;
                continue;
            }
        }

        if (nChangePosInOut == -1) reservekey.ReturnKey(); // Return any reserved key if we don't have change

        // Shuffle selected coins and fill in final vin
        txNew.vin.clear();
        std::vector<CInputCoin> selected_coins(setCoins.begin(), setCoins.end());
        Shuffle(selected_coins.begin(), selected_coins.end(), FastRandomContext());

        // Note how the sequence number is set to non-maxint so that
        // the nLockTime set above actually works.
        //
        // BIP125 defines opt-in RBF as any nSequence < maxint-1, so
        // we use the highest possible value in that range (maxint-2)
        // to avoid conflicting with other possible uses of nSequence,
        // and in the spirit of "smallest possible change from prior
        // behavior."
        const uint32_t nSequence = coin_control.m_signal_bip125_rbf.get_value_or(wallet.m_signal_rbf) ? MAX_BIP125_RBF_SEQUENCE : (CTxIn::SEQUENCE_FINAL - 1);
        for (const auto& coin : selected_coins) {
            if (!coin.IsMWEB()) {
                txNew.vin.push_back(CTxIn(coin.outpoint, CScript(), nSequence));
            }
        }

        if (!MWEB::Transact::CreateTx(wallet.GetMWWallet(), txNew, selected_coins, vecSend, nFeeRet, mweb_fee, change_on_mweb)) {
            strFailReason = _("Failed to create MWEB transaction");
            return false;
        }

        if (sign) {
            int nIn = 0;
            for (const auto& coin : selected_coins) {
                if (coin.IsMWEB()) {
                    continue;
                }

                const CScript& scriptPubKey = coin.txout.scriptPubKey;
                SignatureData sigdata;

                if (!ProduceSignature(wallet, MutableTransactionSignatureCreator(&txNew, nIn, coin.txout.nValue, SIGHASH_ALL), scriptPubKey, sigdata)) {
                    LogPrintf("Transaction failed to sign: %s", CTransaction(txNew).ToString().c_str());
                    strFailReason = _("Signing transaction failed");
                    return false;
                } else {
                    UpdateInput(txNew.vin.at(nIn), sigdata);
                }

                nIn++;
            }
        }

        // Return the constructed transaction data.
        tx = MakeTransactionRef(std::move(txNew));

        // Limit size
        if (GetTransactionWeight(*tx) > MAX_STANDARD_TX_WEIGHT) {
            strFailReason = _("Transaction too large");
            return false;
        }
    }

    if (nFeeRet > maxTxFee) {
        strFailReason = _("Fee exceeds maximum configured by -maxtxfee");
        return false;
    }

    if (gArgs.GetBoolArg("-walletrejectlongchains", DEFAULT_WALLET_REJECT_LONG_CHAINS)) {
        // Lastly, ensure this tx will pass the mempool's chain limits
        LockPoints lp;
        CTxMemPoolEntry entry(tx, 0, 0, 0, false, 0, lp);
        CTxMemPool::setEntries setAncestors;
        size_t nLimitAncestors = gArgs.GetArg("-limitancestorcount", DEFAULT_ANCESTOR_LIMIT);
        size_t nLimitAncestorSize = gArgs.GetArg("-limitancestorsize", DEFAULT_ANCESTOR_SIZE_LIMIT) * 1000;
        size_t nLimitDescendants = gArgs.GetArg("-limitdescendantcount", DEFAULT_DESCENDANT_LIMIT);
        size_t nLimitDescendantSize = gArgs.GetArg("-limitdescendantsize", DEFAULT_DESCENDANT_SIZE_LIMIT) * 1000;
        std::string errString;
        LOCK(::mempool.cs);
        if (!::mempool.CalculateMemPoolAncestors(entry, setAncestors, nLimitAncestors, nLimitAncestorSize, nLimitDescendants, nLimitDescendantSize, errString)) {
            strFailReason = _("Transaction has too long of a mempool chain");
            return false;
        }
    }

    wallet.WalletLogPrintf("Fee Calculation: Fee:%d Bytes:%u Needed:%d Tgt:%d (requested %d) Reason:\"%s\" Decay %.5f: Estimation: (%g - %g) %.2f%% %.1f/(%.1f %d mem %.1f out) Fail: (%g - %g) %.2f%% %.1f/(%.1f %d mem %.1f out)\n",
        nFeeRet, nBytes, nFeeNeeded, feeCalc.returnedTarget, feeCalc.desiredTarget, StringForFeeReason(feeCalc.reason), feeCalc.est.decay,
        feeCalc.est.pass.start, feeCalc.est.pass.end,
        100 * feeCalc.est.pass.withinTarget / (feeCalc.est.pass.totalConfirmed + feeCalc.est.pass.inMempool + feeCalc.est.pass.leftMempool),
        feeCalc.est.pass.withinTarget, feeCalc.est.pass.totalConfirmed, feeCalc.est.pass.inMempool, feeCalc.est.pass.leftMempool,
        feeCalc.est.fail.start, feeCalc.est.fail.end,
        100 * feeCalc.est.fail.withinTarget / (feeCalc.est.fail.totalConfirmed + feeCalc.est.fail.inMempool + feeCalc.est.fail.leftMempool),
        feeCalc.est.fail.withinTarget, feeCalc.est.fail.totalConfirmed, feeCalc.est.fail.inMempool, feeCalc.est.fail.leftMempool);
    return true;
}