// Copyright (c) 2020-2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <qt/mwebpegoutdialog.h>
#include <qt/forms/ui_mwebpegoutdialog.h>

#include <qt/bitcoinunits.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/walletmodel.h>

#include <bech32.h>
#include <key_io.h>
#include <wallet/wallet.h>

#include <QAbstractButton>
#include <QDialogButtonBox>

MWEBPegOutDialog::MWEBPegOutDialog(const PlatformStyle *_platformStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MWEBPegOutDialog),
    model(nullptr),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);

    // ok button
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &MWEBPegOutDialog::buttonBoxClicked);
    connect(ui->generateAddress, &QPushButton::clicked, this, &MWEBPegOutDialog::generateAddressClicked);
}

MWEBPegOutDialog::~MWEBPegOutDialog()
{
    delete ui;
}

void MWEBPegOutDialog::setModel(WalletModel *_model)
{
    this->model = _model;
}

void MWEBPegOutDialog::generateAddressClicked()
{
    if (!model || !model->getOptionsModel()) {
        return;
    }

    WalletModel::UnlockContext ctx(model->requestUnlock());
    if (!ctx.isValid()) {
        // Unlock wallet was cancelled
        return;
    }

    if (!model->wallet().canGetAddresses()) {
        return;
    }

    CPubKey newKey;
    if (!model->wallet().getKeyFromPool(false, newKey)) {
        return;
    }

    auto output_type = OutputType::BECH32;
    model->wallet().learnRelatedScripts(newKey, output_type);
    CTxDestination dest = GetDestinationForKey(newKey, output_type);

    model->wallet().setAddressBook(dest, "mweb", "receive");

    ui->address->setText(QString::fromStdString(EncodeDestination(dest)));
}

// ok button
void MWEBPegOutDialog::buttonBoxClicked(QAbstractButton* button)
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

    CAmount nAmount = ui->amount->value();
    auto address = ui->address->text().toStdString();

    if (nAmount <= 0) {
        Q_EMIT message(tr("Peg-Out LTC"), tr("The amount to pay must be larger than 0."), CClientUIInterface::MSG_ERROR);
        return;
    }

    auto balances = model->wallet().getBalances();
    if (nAmount > balances.mweb_balance) {
        Q_EMIT message(tr("Peg-Out LTC"), tr("The amount exceeds your balance."), CClientUIInterface::MSG_ERROR);
        return;
    }

    auto bech = bech32::Decode(address);
    if (bech.second.size() == 0 || bech.first != Params().Bech32HRP()) {
        Q_EMIT message(tr("Peg-Out LTC"), tr("The recipient address is not valid. Please recheck."), CClientUIInterface::MSG_ERROR);
        return;
    }

    // create MWEB tx
    auto pegout_tx = libmw::wallet::CreatePegOutTx(model->wallet().GetMWWallet(), nAmount, 100'000, address);
    CMutableTransaction txNew;
    txNew.m_mwtx = CMWTx(pegout_tx.first);
    CTransactionRef tx = MakeTransactionRef(txNew);

    // now send the prepared transaction
    mapValue_t mapValue;
    std::string rejectReason;
    if (!model->wallet().commitTransaction(tx, mapValue, {}, rejectReason)) {
        auto msg = tr("The transaction was rejected with the following reason: %1").arg(QString::fromStdString(rejectReason));
        Q_EMIT message(tr("Peg-Out LTC"), msg, CClientUIInterface::MSG_ERROR);
        return;
    }
}
