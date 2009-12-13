/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoParameterToPathCommand.h"
#include "KoParameterShape.h"
#include <klocale.h>

KoParameterToPathCommand::KoParameterToPathCommand(KoParameterShape *shape, QUndoCommand *parent)
        : QUndoCommand(parent)
{
    m_shapes.append(shape);
    initialize();
    setText(i18n("Convert to Path"));
}

KoParameterToPathCommand::KoParameterToPathCommand(const QList<KoParameterShape*> &shapes, QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_shapes(shapes)
{
    initialize();
    setText(i18n("Convert to Path"));
}

KoParameterToPathCommand::~KoParameterToPathCommand()
{
    qDeleteAll(m_copies);
}

void KoParameterToPathCommand::redo()
{
    QUndoCommand::redo();
    for (int i = 0; i < m_shapes.size(); ++i) {
        KoParameterShape * parameterShape = m_shapes.at(i);
        parameterShape->update();
        parameterShape->setModified(true);
        parameterShape->update();
    }
}

void KoParameterToPathCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < m_shapes.size(); ++i) {
        KoParameterShape * parameterShape = m_shapes.at(i);
        parameterShape->update();
        parameterShape->setModified(false);
        copyPath(parameterShape, m_copies[i]);
        parameterShape->update();
    }
}

void KoParameterToPathCommand::initialize()
{
    foreach(KoParameterShape *shape, m_shapes) {
        KoPathShape * p = new KoPathShape();
        copyPath(p, shape);
        m_copies.append(p);
    }
}

void KoParameterToPathCommand::copyPath(KoPathShape * dst, KoPathShape * src)
{
    dst->clear();

    int subpathCount = src->subpathCount();
    for (int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex) {
        int pointCount = src->subpathPointCount(subpathIndex);
        if (! pointCount)
            continue;

        KoSubpath * subpath = new KoSubpath;
        for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
            KoPathPoint * p = src->pointByIndex(KoPathPointIndex(subpathIndex, pointIndex));
            KoPathPoint * c = new KoPathPoint(*p);
            c->setParent(dst);
            subpath->append(c);
        }
        dst->addSubpath(subpath, subpathIndex);
    }
    dst->setTransformation(src->transformation());
}
