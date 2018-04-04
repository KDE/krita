/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

void KoKeepShapesSelectedCommand::end()
{
    KoSelection *selection = m_selectionProxy->selection();

    selection->deselectAll();

    const QList<KoShape*> newSelectedShapes =
        isFinalizing() ? m_selectedAfter : m_selectedBefore;

    Q_FOREACH (KoShape *shape, newSelectedShapes) {
        selection->select(shape);
    }
}

