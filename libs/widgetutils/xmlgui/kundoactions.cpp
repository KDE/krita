/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
