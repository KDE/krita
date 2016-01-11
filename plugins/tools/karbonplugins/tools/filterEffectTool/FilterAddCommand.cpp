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

#include "FilterAddCommand.h"
#include "KoShape.h"
#include "KoFilterEffect.h"
#include "KoFilterEffectStack.h"

#include <klocalizedstring.h>

FilterAddCommand::FilterAddCommand(KoFilterEffect *filterEffect, KoShape *shape, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_filterEffect(filterEffect)
    , m_shape(shape)
    , m_isAdded(false)
{
    Q_ASSERT(m_shape);
    setText(kundo2_i18n("Add filter effect"));
}

FilterAddCommand::~FilterAddCommand()
{
    if (!m_isAdded) {
        delete m_filterEffect;
    }
}

void FilterAddCommand::redo()
{
    KUndo2Command::redo();

    if (m_shape->filterEffectStack()) {
        m_shape->update();
        m_shape->filterEffectStack()->appendFilterEffect(m_filterEffect);
        m_shape->update();
        m_isAdded = true;
    }
}

void FilterAddCommand::undo()
{
    if (m_shape->filterEffectStack()) {
        int index = m_shape->filterEffectStack()->filterEffects().indexOf(m_filterEffect);
        if (index >= 0) {
            m_shape->update();
            m_shape->filterEffectStack()->takeFilterEffect(index);
            m_shape->update();
        }
        m_isAdded = false;
    }
    KUndo2Command::undo();
}
