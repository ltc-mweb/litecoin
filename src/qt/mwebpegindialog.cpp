// Copyright (c) 2020-2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <qt/mwebpegindialog.h>
#include <qt/forms/ui_mwebpegindialog.h>

#include <qt/bitcoinunits.h>
#include <qt/coincontroldialog.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/walletmodel.h>

#include <wallet/coincontrol.h>
#include <wallet/wallet.h>

#include <QAbstractButton>
#include <QDialogButtonBox>

MWEBPegInDialog::MWEBPegInDialog(const PlatformStyle *_platformStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MWEBPegInDialog),
    model(nullptr),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);

    // ok button
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &MWEBPegInDialog::buttonBoxClicked);
}

MWEBPegInDialog::~MWEBPegInDialog()
{
    delete ui;
}

void MWEBPegInDialog::setModel(WalletModel *_model)
{
    this->model = _model;
}

// ok button
void MWEBPegInDialog::buttonBoxClicked(QAbstractButton* button)
{
    if (ui->buttonBox->buttonRole(button) != QDialogButtonBox::AcceptRole) {
        return;
    }
    done(QDialog::Accepted);

    if (!model || !model->getOptionsModel()) {
        return;
    }

    WalletModel::UnlockContext ctx(model->requestUnlock());
    if (!ctx.isValid()) {
        // Unlock wallet was cancelled
        return;
    }

    // create MWEB tx and generate recipient
    CAmount nAmount = ui->amount->value();
    auto pegin_tx = libmw::wallet::CreatePegInTx(model->wallet().GetMWWallet(), nAmount);
    mapValue_t mapValue;
    mapValue["commitment"] = HexStr(std::vector<uint8_t>(pegin_tx.second.commitment.cbegin(), pegin_tx.second.commitment.cend()), false);
    CScript pegin_script;
    pegin_script << CScript::EncodeOP_N(Consensus::Mimblewimble::WITNESS_VERSION);
    pegin_script << std::vector<uint8_t>(pegin_tx.second.commitment.cbegin(), pegin_tx.second.commitment.cend());
    std::vector<CRecipient> vecSend;
    vecSend.push_back({pegin_script, nAmount, false});

    // Always use a CCoinControl instance, use the CoinControlDialog instance if CoinControl has been enabled
    CCoinControl ctrl;
    if (model->getOptionsModel()->getCoinControlFeatures()) {
        ctrl = *CoinControlDialog::coinControl();
    }

    CAmount nBalance = model->wallet().getAvailableBalance(ctrl);
    if (nAmount > nBalance) {
        Q_EMIT message(tr("Peg-In LTC"), tr("The amount exceeds your balance."), CClientUIInterface::MSG_ERROR);
        return;
    }

    CAmount nFeeRequired = 0;
    int nChangePosRet = -1;
    std::string strFailReason;

    auto newTx = model->wallet().createTransaction(vecSend, ctrl, true /* sign */, nChangePosRet, nFeeRequired, strFailReason, CMWTx(pegin_tx.first));
    if (!newTx) {
        auto msg = QString::fromStdString(strFailReason);
        if (nAmount + nFeeRequired > nBalance) {
            auto msgFee = BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), nFeeRequired);
            msg = tr("The total exceeds your balance when the %1 transaction fee is included.").arg(msgFee);
        }
        Q_EMIT message(tr("Peg-In LTC"), msg, CClientUIInterface::MSG_ERROR);
        return;
    }

    // now send the prepared transaction
    std::string rejectReason;
    if (!newTx->commit(mapValue, {}, rejectReason)) {
        auto msg = tr("The transaction was rejected with the following reason: %1").arg(QString::fromStdString(rejectReason));
        Q_EMIT message(tr("Peg-In LTC"), msg, CClientUIInterface::MSG_ERROR);
        return;
    }

    CoinControlDialog::coinControl()->UnSelectAll();
}
