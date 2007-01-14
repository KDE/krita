/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "InsertVariableAction_p.h"
#include "KoTextSelectionHandler.h"
#include "KoVariable.h"

#include <KoCanvasBase.h>
#include <KoToolProxy.h>
#include "KoInlineObjectFactory.h"

#include <kdebug.h>

InsertVariableAction::InsertVariableAction(KoCanvasBase *base, KoInlineObjectFactory *factory)
    : KAction(KIcon(factory->icon()), factory->name(), base->canvasWidget()),
    m_canvas(base),
    m_factory(factory)
{
    Q_ASSERT(factory->type() == KoInlineObjectFactory::TextVariable);

    connect(this, SIGNAL(triggered(bool)), this, SLOT(activated()));
}

void InsertVariableAction::activated() {
    Q_ASSERT(m_canvas->toolProxy());
    KoTextSelectionHandler *handler = qobject_cast<KoTextSelectionHandler*> (m_canvas->toolProxy()->selection());
    if(handler) {
        KoVariable *variable = static_cast<KoVariable*> (m_factory->createInlineObject());
        handler->insertVariable(variable);
    }
    else
        kWarning(32500) << "InsertVariableAction: No texttool selected while trying to insert variable\n";
}


#include "InsertVariableAction_p.moc"
