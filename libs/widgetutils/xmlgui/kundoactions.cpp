/* This file is part of the KDE project
   Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kundoactions.h"
#include "config-xmlgui.h"
#include <QAction>
#include <QKeySequence>
#include <QList>
#include <QUndoStack>

#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <klocalizedstring.h>

#include <kis_icon_utils.h>

QAction *KUndoActions::createRedoAction(QUndoStack *undoStack, KActionCollection *actionCollection, const QString &actionName)
{
    QAction *action = undoStack->createRedoAction(actionCollection);

    if (actionName.isEmpty()) {
        action->setObjectName(QLatin1String(KStandardAction::name(KStandardAction::Redo)));
    } else {
        action->setObjectName(actionName);
    }

    action->setIcon(KisIconUtils::loadIcon(QStringLiteral("edit-redo")));
    action->setIconText(i18n("Redo"));
    action->setShortcuts(KStandardShortcut::redo());

    actionCollection->addAction(action->objectName(), action);

    return action;
}

QAction *KUndoActions::createUndoAction(QUndoStack *undoStack, KActionCollection *actionCollection, const QString &actionName)
{
    QAction *action = undoStack->createUndoAction(actionCollection);

    if (actionName.isEmpty()) {
        action->setObjectName(QLatin1String(KStandardAction::name(KStandardAction::Undo)));
    } else {
        action->setObjectName(actionName);
    }

    action->setIcon(KisIconUtils::loadIcon(QStringLiteral("edit-undo")));
    action->setIconText(i18n("Undo"));
    action->setShortcuts(KStandardShortcut::undo());

    actionCollection->addAction(action->objectName(), action);

    return action;
}
