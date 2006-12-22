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

#include "KoShapeSizeCommand.h"

#include <klocale.h>

KoShapeSizeCommand::KoShapeSizeCommand(const KoSelectionSet &shapes, QList<QSizeF> &previousSizes, QList<QSizeF> &newSizes,
                                        QUndoCommand *parent)
: QUndoCommand(parent)
, m_previousSizes(previousSizes)
, m_newSizes(newSizes)
{
    m_shapes = shapes.toList();
    Q_ASSERT(m_shapes.count() == m_previousSizes.count());
    Q_ASSERT(m_shapes.count() == m_newSizes.count());

    setText(i18n( "Resize shapes" ));
}

KoShapeSizeCommand::KoShapeSizeCommand(const QList<KoShape*> &shapes, QList<QSizeF> &previousSizes, QList<QSizeF> &newSizes, QUndoCommand *parent)
: QUndoCommand(parent)
, m_previousSizes(previousSizes)
, m_newSizes(newSizes)
{
    m_shapes = shapes;
    Q_ASSERT(m_shapes.count() == m_previousSizes.count());
    Q_ASSERT(m_shapes.count() == m_newSizes.count());

    setText(i18n( "Resize shapes" ));
}

void KoShapeSizeCommand::redo () {
    int i=0;
    foreach(KoShape *shape, m_shapes) {
        shape->repaint();
        shape->resize(m_newSizes[i++]);
        shape->repaint();
    }
}

void KoShapeSizeCommand::undo () {
    int i=0;
    foreach(KoShape *shape, m_shapes) {
        shape->repaint();
        shape->resize(m_previousSizes[i++]);
        shape->repaint();
    }
}
