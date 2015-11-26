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

#include "FilterRemoveCommand.h"
#include "KoShape.h"
#include "KoFilterEffect.h"
#include "KoFilterEffectStack.h"

#include <klocalizedstring.h>

FilterRemoveCommand::FilterRemoveCommand(int filterEffectIndex, KoFilterEffectStack *filterStack, KoShape *shape, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_filterEffect(0)
    , m_filterStack(filterStack)
    , m_shape(shape)
    , m_isRemoved(false)
    , m_filterEffectIndex(filterEffectIndex)
{
    Q_ASSERT(filterStack);
    setText(kundo2_i18n("Remove filter effect"));
}

FilterRemoveCommand::~FilterRemoveCommand()
{
    if (m_isRemoved) {
        delete m_filterEffect;
    }
}

void FilterRemoveCommand::redo()
{
    KUndo2Command::redo();

    if (m_shape) {
        m_shape->update();
    }

    m_filterEffect = m_filterStack->takeFilterEffect(m_filterEffectIndex);
    m_isRemoved = true;

    if (m_shape) {
        m_shape->update();
    }
}

void FilterRemoveCommand::undo()
{
    if (m_shape) {
        m_shape->update();
    }

    m_filterStack->insertFilterEffect(m_filterEffectIndex, m_filterEffect);
    m_isRemoved = false;

    if (m_shape) {
        m_shape->update();
    }

    KUndo2Command::undo();
}
