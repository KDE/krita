/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SELECTION_TRANSFORM_COMMAND_H
#define SELECTION_TRANSFORM_COMMAND_H

#include <kundo2command.h>
#include <QTransform>

class KoSelection;
class KoShape;

class SelectionTransformCommand : public KUndo2Command
{
public:
    SelectionTransformCommand(KoSelection *selection, const QTransform &oldTransformation, const QTransform &newTransformation, KUndo2Command *parent = 0);

    /// reimplemented from KUndo2Command
    virtual void redo();
    /// reimplemented from KUndo2Command
    virtual void undo();
private:
    KoSelection *m_selection;
    QList<KoShape *> m_selectedShapes;
    QTransform m_oldTransformation;
    QTransform m_newTransformation;
};

#endif // SELECTION_TRANSFORM_COMMAND_H
