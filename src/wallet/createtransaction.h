#pragma once

#include <wallet/wallet.h>
#include <wallet/coincontrol.h>

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
    bool sign
);