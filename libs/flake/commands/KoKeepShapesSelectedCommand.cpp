/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoKeepShapesSelectedCommand.h"

#include <KoShape.h>
#include <KoSelection.h>
#include <KoSelectedShapesProxy.h>


KoKeepShapesSelectedCommand::KoKeepShapesSelectedCommand(const QList<KoShape*> &selectedBefore,
                                                         const QList<KoShape*> &selectedAfter,
                                                         KoSelectedShapesProxy *selectionProxy,
                                                         bool isFinalizing,
                                                         KUndo2Command *parent)
    : KisCommandUtils::FlipFlopCommand(isFinalizing, parent),
      m_selectedBefore(selectedBefore),
      m_selectedAfter(selectedAfter),
      m_selectionProxy(selectionProxy)
{

}

void KoKeepShapesSelectedCommand::partB()
{
    KoSelection *selection = m_selectionProxy->selection();

    selection->deselectAll();

    const QList<KoShape*> newSelectedShapes =
        getState() == FlipFlopCommand::State::FINALIZING ? m_selectedAfter : m_selectedBefore;

    Q_FOREACH (KoShape *shape, newSelectedShapes) {
        selection->select(shape);
    }
}

