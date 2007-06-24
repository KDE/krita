/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSHAPETRANSFORMCOMMAND_H
#define KOSHAPETRANSFORMCOMMAND_H

#include <flake_export.h>
#include <QtGui/QUndoCommand>
#include <QtGui/QMatrix>
#include <QtCore/QList>

class KoShape;

class FLAKE_EXPORT KoShapeTransformCommand : public QUndoCommand
{
public:
    /**
     * A command to transform a selection of shapes with the same transformation.
     * @param shapes all the shapes that should be transformed
     * @param transformations the transformation to apply to all shapes
     */
    KoShapeTransformCommand( const QList<KoShape*> &shapes, const QMatrix &transformation, QUndoCommand *parent = 0 );

    /**
     * A command to transform a selection of shapes with the same transformation.
     * @param shapes all the shapes that should be transformed
     * @param transformations the transformation to apply to all shapes
     */
    KoShapeTransformCommand( const QList<KoShape*> &shapes, const QList<QMatrix> &transformations, QUndoCommand * parent = 0 );

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    QList<KoShape*> m_shapes;
    QList<QMatrix> m_transformations;
};

#endif // KOSHAPETRANSFORMCOMMAND_H
