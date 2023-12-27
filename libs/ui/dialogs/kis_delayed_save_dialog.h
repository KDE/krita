/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DELAYED_SAVE_DIALOG_H
#define KIS_DELAYED_SAVE_DIALOG_H

#include <QScopedPointer>

#include <KoDialog.h>
#include <kis_types.h>

class WdgDelayedSaveDialog;

class KisDelayedSaveDialog : public KoDialog
{
    Q_OBJECT

public:
    enum ResultType {
        Rejected = QDialog::Rejected,
        Accepted = QDialog::Accepted,
        Ignored = 2
    };

    enum Type {
        SaveDialog,
        GeneralDialog,
        ForcedDialog
    };

public:
    explicit KisDelayedSaveDialog(KisImageSP image, Type type, int busyWait, QWidget *parent = nullptr);
    ~KisDelayedSaveDialog() override;

    void blockIfImageIsBusy();

private Q_SLOTS:
    void slotTimerTimeout();
    void slotCancelRequested();
    void slotIgnoreRequested();

private:
    WdgDelayedSaveDialog *ui;
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_DELAYED_SAVE_DIALOG_H
