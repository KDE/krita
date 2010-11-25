/*
 * This file is part of the KDE project
 * Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include "AutoResizeCommand.h"
#include "TextShape.h"

#include <klocale.h>
//#include <KAction>

AutoResizeCommand::AutoResizeCommand(TextShape *textShape, KoTextDocument::ResizeMethod resizeMethod, bool enabled)
    : QUndoCommand()
    , m_textShape(textShape)
    , m_resizeMethod(resizeMethod)
    , m_enabled(enabled)
    , m_first(true)
    , m_prevResizeMethod(KoTextDocument::NoResize)
{
    Q_ASSERT(m_textShape);
    const QString s = m_enabled ? i18nc("Enable Shrink To Fit", "Enable") : i18nc("Disable Shrink To Fit", "Disable");
    switch (m_resizeMethod) {
        case KoTextDocument::AutoGrowWidth:
            setText(i18nc("Enable/Disable Grow To Fit Width", "%1 Grow To Fit Width", s));
            break;
        case KoTextDocument::AutoGrowHeight:
            setText(i18nc("Enable/Disable Grow To Fit Height", "%1 Grow To Fit Height", s));
            break;
        case KoTextDocument::ShrinkToFitResize:
            setText(i18nc("Enable/Disable Shrink To Fit", "%1 Shrink To Fit", s));
            break;
        default:
            Q_ASSERT_X(false, __FUNCTION__, QString("The resize-method '%1' is unsupported by this command").arg(resizeMethod).toUtf8());
            break;
    }
}

void AutoResizeCommand::undo()
{
    m_textShape->setResizeMethod(m_prevResizeMethod);
}

void AutoResizeCommand::redo()
{
    if (m_first) {
        m_first = false;
        m_prevResizeMethod = m_textShape->resizeMethod();
    }
    KoTextDocument::ResizeMethod resize = m_enabled ? m_resizeMethod : KoTextDocument::NoResize;
    if (m_resizeMethod == KoTextDocument::AutoGrowWidth || m_resizeMethod == KoTextDocument::AutoGrowHeight) {
        if (m_enabled) {
            if ((m_textShape->resizeMethod() == KoTextDocument::AutoGrowWidth || m_textShape->resizeMethod() == KoTextDocument::AutoGrowHeight) && m_resizeMethod != m_textShape->resizeMethod())
                resize = KoTextDocument::AutoGrowWidthAndHeight;
        } else {
            if (m_textShape->resizeMethod() == KoTextDocument::AutoGrowWidthAndHeight)
                resize = m_resizeMethod == KoTextDocument::AutoGrowWidth ? KoTextDocument::AutoGrowHeight : KoTextDocument::AutoGrowWidth;
        }
    }
    m_textShape->setResizeMethod(resize);
}
