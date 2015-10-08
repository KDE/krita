/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KODIALOG_P_H
#define KODIALOG_P_H

#include "KoDialog.h"
#include <QtCore/QPointer>
#include <QtCore/QSignalMapper>
#include <QtCore/QSize>
#include <QtCore/QHash>

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

    KoDialog *q_ptr;

    void setupLayout();
    void appendButton(KoDialog::ButtonCode code, const KGuiItem &item);

    bool mDetailsVisible;
    bool mSettingDetails;
    bool mDeferredDelete;
    QWidget *mDetailsWidget;
    QSize mIncSize;
    QSize mMinSize;
    QString mDetailsButtonText;

    QBoxLayout *mTopLayout;
    QPointer<QWidget> mMainWidget;
    KUrlLabel *mUrlHelp;
    KSeparator *mActionSeparator;

    QString mAnchor;
    QString mHelpApp;
    QString mHelpLinkText;

    Qt::Orientation mButtonOrientation;
    KoDialog::ButtonCode mDefaultButton;
    KoDialog::ButtonCode mEscapeButton;

    QDialogButtonBox *mButtonBox;
    QHash<int, QPushButton *> mButtonList;
    QSignalMapper mButtonSignalMapper;

protected Q_SLOTS:
    void queuedLayoutUpdate();
    void helpLinkClicked();

private:
    void init(KoDialog *);
    bool dirty: 1;
};

#endif // KDEUI_KDIALOG_P_H
