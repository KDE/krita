/* This file is part of the KDE project
 * Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "FilterRegionChangeCommand.h"
#include "KoFilterEffect.h"
#include "KoShape.h"

FilterRegionChangeCommand::FilterRegionChangeCommand(KoFilterEffect *effect, const QRectF &filterRegion, KoShape *shape, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_effect(effect)
    , m_newRegion(filterRegion)
    , m_shape(shape)
{
    Q_ASSERT(m_effect);
    m_oldRegion = m_effect->filterRect();
}

void FilterRegionChangeCommand::redo()
{
    if (m_shape) {
        m_shape->update();
    }

    m_effect->setFilterRect(m_newRegion);

    if (m_shape) {
        m_shape->update();
        m_shape->notifyChanged();
    }

    KUndo2Command::redo();
}

void FilterRegionChangeCommand::undo()
{
    if (m_shape) {
        m_shape->update();
    }

    m_effect->setFilterRect(m_oldRegion);

    if (m_shape) {
        m_shape->update();
        m_shape->notifyChanged();
    }

    KUndo2Command::undo();
}
