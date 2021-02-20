/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
