/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

