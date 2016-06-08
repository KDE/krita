/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "InsertTextLocator_p.h"
#include "KoTextEditor.h"

#include <KoCanvasBase.h>


InsertTextLocator::InsertTextLocator(KoCanvasBase *canvas)
        : InsertInlineObjectActionBase(canvas, i18n("Index Reference"))
{
}

KoInlineObject *InsertTextLocator::createInlineObject()
{
    Q_ASSERT(m_canvas);
    KoTextEditor *editor = KoTextEditor::getTextEditorFromCanvas(m_canvas);
    if (editor) {
        KoInlineObject *obj = editor->insertIndexMarker();
        Q_UNUSED(obj); // intentionally unused: if we return it, it gets inserted again
    }
    return 0;
}
