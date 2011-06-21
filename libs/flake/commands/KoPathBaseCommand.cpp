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

#include "KoPathBaseCommand.h"
#include "KoPathShape.h"

KoPathBaseCommand::KoPathBaseCommand(KUndo2Command *parent)
        : KUndo2Command(parent)
{
}

KoPathBaseCommand::KoPathBaseCommand(KoPathShape *shape, KUndo2Command *parent)
        : KUndo2Command(parent)
{
    m_shapes.insert(shape);
}

void KoPathBaseCommand::repaint(bool normalizeShapes)
{
    foreach(KoPathShape *shape, m_shapes) {
        if (normalizeShapes)
            shape->normalize();
        shape->update();
    }
}

