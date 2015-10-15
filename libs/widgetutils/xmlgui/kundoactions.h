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
