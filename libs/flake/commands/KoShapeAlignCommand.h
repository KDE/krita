/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSHAPEALIGNCOMMAND_H
#define KOSHAPEALIGNCOMMAND_H

#include "flake_export.h"

#include <QRectF>
#include <QUndoCommand>
#include <QList>

class KoShape;

/// The undo / redo command for aligning shapes
class FLAKE_EXPORT KoShapeAlignCommand : public QUndoCommand
{
public:
    /// The different alignment options for this command
    enum Align {
        HorizontalLeftAlignment,    ///< Align left
        HorizontalCenterAlignment,  ///< Align Centered horizontally
        HorizontalRightAlignment,   ///< Align Right
        VerticalBottomAlignment,    ///< Align bottom
        VerticalCenterAlignment,    ///< Align centered vertically
        VerticalTopAlignment        ///< Align top
    };
    /**
     * Command to align a set of shapes in a rect
     * @param shapes a set of all the shapes that should be aligned
     * @param align the aligment type
     * @param boundingRect the rect the shape will be aligned in
     * @param parent the parent command used for macro commands
     */
    KoShapeAlignCommand(const QList<KoShape*> &shapes, Align align, const QRectF &boundingRect, QUndoCommand *parent = 0);
    virtual ~KoShapeAlignCommand();
    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();
private:
    class Private;
    Private * const d;
};

#endif
