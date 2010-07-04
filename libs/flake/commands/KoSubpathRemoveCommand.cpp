/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoSubpathRemoveCommand.h"
#include <klocale.h>

KoSubpathRemoveCommand::KoSubpathRemoveCommand(KoPathShape *pathShape, int subpathIndex, QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_pathShape(pathShape)
        , m_subpathIndex(subpathIndex)
        , m_subpath(0)
{
    setText(i18n("Remove subpath"));
}

KoSubpathRemoveCommand::~KoSubpathRemoveCommand()
{
    if (m_subpath) {
        qDeleteAll(*m_subpath);
        delete m_subpath;
    }
}

void KoSubpathRemoveCommand::redo()
{
    QUndoCommand::redo();
    m_pathShape->update();
    m_subpath = m_pathShape->removeSubpath(m_subpathIndex);
    if (m_subpath) {
        QPointF offset = m_pathShape->normalize();

        QTransform matrix;
        matrix.translate(-offset.x(), -offset.y());
        foreach(KoPathPoint *point, *m_subpath) {
            point->map(matrix);
        }
        m_pathShape->update();
    }
}

void KoSubpathRemoveCommand::undo()
{
    QUndoCommand::undo();
    if (m_subpath) {
        m_pathShape->addSubpath(m_subpath, m_subpathIndex);
        m_pathShape->normalize();
        m_pathShape->update();
        m_subpath = 0;
    }
}

