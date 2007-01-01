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

#include "KoShapeRotateCommand.h"

#include <klocale.h>

KoShapeRotateCommand::KoShapeRotateCommand(const QList<KoShape*> &shapes, QList<double> &previousAngles, QList<double> &newAngles, QUndoCommand *parent)
: QUndoCommand(parent)
, m_previousAngles(previousAngles)
, m_newAngles(newAngles)
{
    m_shapes = shapes;
    Q_ASSERT(m_shapes.count() == m_previousAngles.count());
    Q_ASSERT(m_shapes.count() == m_newAngles.count());

    setText( i18n( "Rotate shapes" ) );
}

void KoShapeRotateCommand::redo() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->rotate( m_newAngles.at(i) );
        m_shapes.at(i)->repaint();
    }
}

void KoShapeRotateCommand::undo() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->rotate( m_previousAngles.at(i) );
        m_shapes.at(i)->repaint();
    }
}
