/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only

*/

#ifndef KODIALOG_P_H
#define KODIALOG_P_H

#include "KoDialog.h"
#include <QPointer>
#include <KisSignalMapper.h>
#include <QSize>
#include <QHash>

class QBoxLayout;
class QPushButton;
class KUrlLabel;
class KSeparator;
class QDialogButtonBox;

class KoDialogPrivate
{
    Q_DECLARE_PUBLIC(KoDialog)
protected:
    KoDialogPrivate()
        : mDetailsVisible(false), mSettingDetails(false), mDeferredDelete(false),
          mDetailsWidget(0),
          mTopLayout(0), mMainWidget(0), mUrlHelp(0), mActionSeparator(0),
          mButtonOrientation(Qt::Horizontal),
          mDefaultButton(KoDialog::NoDefault),
          mButtonBox(0)
    {
    }

    virtual ~KoDialogPrivate() {}

    KoDialog *q_ptr {nullptr};

    void setupLayout();
    void appendButton(KoDialog::ButtonCode code, const KGuiItem &item);

    bool mDetailsVisible {false};
    bool mSettingDetails {false};
    bool mDeferredDelete {false};
    QWidget *mDetailsWidget {nullptr};
    QSize mIncSize;
    QSize mMinSize;
    QString mDetailsButtonText;

    QBoxLayout *mTopLayout {nullptr};
    QPointer<QWidget> mMainWidget;
    KUrlLabel *mUrlHelp {nullptr};
    KSeparator *mActionSeparator {nullptr};

    QString mAnchor;
    QString mHelpApp;
    QString mHelpLinkText;

    Qt::Orientation mButtonOrientation;
    KoDialog::ButtonCode mDefaultButton {KoDialog::NoDefault};
    KoDialog::ButtonCode mEscapeButton {KoDialog::None};

    QDialogButtonBox *mButtonBox {nullptr};
    QHash<int, QPushButton *> mButtonList;

protected Q_SLOTS:
    void queuedLayoutUpdate();
    void helpLinkClicked();

private:
    void init(KoDialog *);
    bool dirty {false};
};

#endif // KDEUI_KDIALOG_P_H
