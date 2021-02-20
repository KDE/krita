/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktoggletoolbaraction.h"

#include <QByteArray>
#include <QEvent>
#include <QPointer>

#include <klocalizedstring.h>

#include "kmainwindow.h"
#include "ktoolbar.h"

class KToggleToolBarAction::Private
{
public:
    Private()
        : toolBarName(0), toolBar(0), beingToggled(false)
    {
    }

    QByteArray toolBarName;
    QPointer<KToolBar> toolBar;
    bool beingToggled;
};

KToggleToolBarAction::KToggleToolBarAction(const char *toolBarName, const QString &text, QObject *parent)
    : KToggleAction(text, parent),
      d(new Private)
{
    d->toolBarName = toolBarName;
}

KToggleToolBarAction::KToggleToolBarAction(KToolBar *toolBar, const QString &text, QObject *parent)
    : KToggleAction(text, parent),
      d(new Private)
{
    d->toolBar = toolBar;
    d->toolBar->installEventFilter(this);

    d->beingToggled = true;
    setChecked(d->toolBar->isVisible());
    d->beingToggled = false;
}

KToggleToolBarAction::~KToggleToolBarAction()
{
    delete d;
}

bool KToggleToolBarAction::eventFilter(QObject *watched, QEvent *event)
{
    if (d->beingToggled) {
        return false;
    }

    d->beingToggled = true;

    if (watched == d->toolBar) {
        switch (event->type()) {
        case QEvent::Hide:
            if (isChecked()) {
                setChecked(false);
            }
            break;

        case QEvent::Show:
            if (!isChecked()) {
                setChecked(true);
            }
            break;

        default:
            break;
        }
    }

    d->beingToggled = false;

    return false;
}

KToolBar *KToggleToolBarAction::toolBar()
{
    return d->toolBar;
}

void KToggleToolBarAction::slotToggled(bool checked)
{
    if (!d->beingToggled && d->toolBar && checked != d->toolBar->isVisible()) {
        d->beingToggled = true;
        d->toolBar->setVisible(checked);
        d->beingToggled = false;

        QMainWindow *mw = d->toolBar->mainWindow();
        if (mw && qobject_cast<KMainWindow *>(mw)) {
            static_cast<KMainWindow *>(mw)->setSettingsDirty();
        }
    }

    KToggleAction::slotToggled(checked);
}

