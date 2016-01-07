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

#include "FilterStackSetCommand.h"
#include "KoShape.h"
#include "KoFilterEffectStack.h"

#include <klocalizedstring.h>

FilterStackSetCommand::FilterStackSetCommand(KoFilterEffectStack *newStack, KoShape *shape, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_newFilterStack(newStack)
    , m_shape(shape)
{
    Q_ASSERT(m_shape);
    m_oldFilterStack = m_shape->filterEffectStack();
    if (m_newFilterStack) {
        m_newFilterStack->ref();
    }
    if (m_oldFilterStack) {
        m_oldFilterStack->ref();
    }

    setText(kundo2_i18n("Set filter stack"));
}

FilterStackSetCommand::~FilterStackSetCommand()
{
    if (m_newFilterStack && !m_newFilterStack->deref()) {
        delete m_newFilterStack;
    }
    if (m_oldFilterStack && !m_oldFilterStack->deref()) {
        delete m_oldFilterStack;
    }
}

void FilterStackSetCommand::redo()
{
    KUndo2Command::redo();

    m_shape->update();
    m_shape->setFilterEffectStack(m_newFilterStack);
    m_shape->update();
}

void FilterStackSetCommand::undo()
{
    m_shape->update();
    m_shape->setFilterEffectStack(m_oldFilterStack);
    m_shape->update();

    KUndo2Command::undo();
}
