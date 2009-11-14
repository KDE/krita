/* This file is part of the KDE libraries
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

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

#include "KoStandardAction.h"

#include <KActionCollection>
#include <KToggleAction>
#include <KLocale>

KAction *KoStandardAction::create(StandardAction id, const QObject *recvr, const char *slot, QObject *parent)
{
    KAction *newAction = 0;

    switch (id) {
    case ShowGuides: {
        KToggleAction *toggle = new KToggleAction(KIcon("guides"), i18n("Show Guides"), parent);
        toggle->setCheckedState(KGuiItem(i18n("Hide Guides")));
        toggle->setToolTip(i18n("Shows or hides guides"));
        newAction = toggle;
        break;
    }
    case ActionNone:
        return 0;
    }

    Q_ASSERT(newAction);
    newAction->setObjectName(name(id));

    if (recvr && slot)
        QObject::connect(newAction, SIGNAL(triggered(bool)), recvr, slot);

    KActionCollection *collection = qobject_cast<KActionCollection *>(parent);
    if (collection)
        collection->addAction(newAction->objectName(), newAction);

    return newAction;
}

const char* KoStandardAction::name(StandardAction id)
{
    switch (id) {
    case ShowGuides:
        return "view_show_guides";
    default:
        return 0;
    };
}

KToggleAction *KoStandardAction::showGuides(const QObject *receiver, const char *slot, QObject *parent)
{
    return static_cast<KToggleAction*>(create(ShowGuides, receiver, slot, parent));
}
