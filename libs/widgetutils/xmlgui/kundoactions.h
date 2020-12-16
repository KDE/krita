/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUNDOACTIONS_H
#define KUNDOACTIONS_H

#include <kritawidgetutils_export.h>

#include <QString>

class KActionCollection;
class QAction;
class QUndoStack;

/**
 * Provides functions that creates undo/redo actions for a QUndoStack with KDE's default icons and shortcuts.
 * See QUndoStack for more information.
 *
 * @since 5.0
 */
namespace KUndoActions
{
/**
 * Creates an redo action with the default shortcut and icon and adds it to @p actionCollection
 * @param undoStack the QUndoStack the action triggers the redo on
 * @param actionCollection the KActionCollection that should be the parent of the action
 * @param actionName the created action's object name, empty string will set it to the KDE default
 * @return the created action.
 */
KRITAWIDGETUTILS_EXPORT QAction *createRedoAction(QUndoStack *undoStack, KActionCollection *actionCollection, const QString &actionName = QString());

/**
 * Creates an undo action with the default KDE shortcut and icon and adds it to @p actionCollection
 * @param undoStack the QUndoStack the action triggers the undo on
 * @param actionCollection the KActionCollection that should be the parent of the action
 * @param actionName the created action's object name, empty string will set it to the KDE default
 * @return the created action.
 */
KRITAWIDGETUTILS_EXPORT QAction *createUndoAction(QUndoStack *undoStack, KActionCollection *actionCollection, const QString &actionName = QString());
}

#endif
