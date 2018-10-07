/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "FilterInputChangeCommand.h"
#include "KoFilterEffect.h"
#include "KoShape.h"

FilterInputChangeCommand::FilterInputChangeCommand(const InputChangeData &data, KoShape *shape, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
{
    m_data.append(data);
}

FilterInputChangeCommand::FilterInputChangeCommand(const QList<InputChangeData> &data, KoShape *shape, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_data(data)
    , m_shape(shape)
{
}

void FilterInputChangeCommand::redo()
{
    if (m_shape) {
        m_shape->update();
    }

    Q_FOREACH (const InputChangeData &data, m_data) {
        data.filterEffect->setInput(data.inputIndex, data.newInput);
    }

    if (m_shape) {
        m_shape->update();
    }

    KUndo2Command::redo();
}

void FilterInputChangeCommand::undo()
{
    if (m_shape) {
        m_shape->update();
    }

    Q_FOREACH (const InputChangeData &data, m_data) {
        data.filterEffect->setInput(data.inputIndex, data.oldInput);
    }

    if (m_shape) {
        m_shape->update();
    }

    KUndo2Command::undo();
}
