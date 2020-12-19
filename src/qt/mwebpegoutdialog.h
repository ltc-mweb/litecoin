// Copyright (c) 2020-2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_MWEBPEGOUTDIALOG_H
#define BITCOIN_QT_MWEBPEGOUTDIALOG_H

#include <amount.h>

#include <QAbstractButton>
#include <QDialog>

class PlatformStyle;
class WalletModel;

namespace Ui {
    class MWEBPegOutDialog;
}

class MWEBPegOutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MWEBPegOutDialog(const PlatformStyle *platformStyle, QWidget *parent = nullptr);
    ~MWEBPegOutDialog();

    void setModel(WalletModel *model);

Q_SIGNALS:
    // Fired when a message should be reported to the user
    void message(const QString &title, const QString &message, unsigned int style);

private:
    Ui::MWEBPegOutDialog *ui;
    WalletModel *model;

    const PlatformStyle *platformStyle;

private Q_SLOTS:
    void generateAddressClicked();
    void buttonBoxClicked(QAbstractButton* button);
};

#endif // BITCOIN_QT_MWEBPEGOUTDIALOG_H
