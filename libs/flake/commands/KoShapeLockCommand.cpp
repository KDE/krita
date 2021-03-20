/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeLockCommand.h"
#include "KoShape.h"

#include <klocalizedstring.h>

KoShapeLockCommand::KoShapeLockCommand(const QList<KoShape*> &shapes, const QList<bool> &oldLock, const QList<bool> &newLock, KUndo2Command *parent)
        : KUndo2Command(parent)
        , m_shapes(shapes)
        , m_oldLock(oldLock)
        , m_newLock(newLock)
{
    Q_ASSERT(m_shapes.count() == m_oldLock.count());
    Q_ASSERT(m_shapes.count() == m_newLock.count());

    setText(kundo2_i18n("Lock shapes"));
}

KoShapeLockCommand::~KoShapeLockCommand()
{
}

void KoShapeLockCommand::redo()
{
    KUndo2Command::redo();
    for (int i = 0; i < m_shapes.count(); ++i) {
        m_shapes[i]->setGeometryProtected(m_newLock[i]);
    }
}

void KoShapeLockCommand::undo()
{
    KUndo2Command::undo();
    for (int i = 0; i < m_shapes.count(); ++i) {
        m_shapes[i]->setGeometryProtected(m_oldLock[i]);
    }
}
