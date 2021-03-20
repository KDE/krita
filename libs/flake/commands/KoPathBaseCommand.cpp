/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    Q_FOREACH (KoPathShape *shape, m_shapes) {
        if (normalizeShapes)
            shape->normalize();
        shape->update();
    }
}

