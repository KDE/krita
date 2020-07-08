/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoAddRemoveShapeCommands.h"

#include <KoShapeContainer.h>
#include <kis_assert.h>


KoAddRemoveShapeCommandImpl::KoAddRemoveShapeCommandImpl(KoShape *shape, KoShapeContainer *parent, KisCommandUtils::FlipFlopCommand::State state, KUndo2Command *parentCommand)
    : KisCommandUtils::FlipFlopCommand(state, parentCommand),
      m_shape(shape),
      m_parent(parent)
{
}

KoAddRemoveShapeCommandImpl::~KoAddRemoveShapeCommandImpl() {
    if (m_ownShape) {
        delete m_shape;
    }
}

void KoAddRemoveShapeCommandImpl::partB()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_shape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_parent);

    m_parent->removeShape(m_shape);
    m_ownShape = true;
}

void KoAddRemoveShapeCommandImpl::partA()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_shape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_parent);

    m_parent->addShape(m_shape);
    m_ownShape = false;
}

KoAddShapeCommand::KoAddShapeCommand(KoShape *shape, KoShapeContainer *parent, KUndo2Command *parentCommand)
    : KoAddRemoveShapeCommandImpl(shape, parent, INITIALIZING, parentCommand)
{
}

KoRemoveShapeCommand::KoRemoveShapeCommand(KoShape *shape, KoShapeContainer *parent, KUndo2Command *parentCommand)
    : KoAddRemoveShapeCommandImpl(shape, parent, FINALIZING, parentCommand)
{
}
