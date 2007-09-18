/*
 * This file is part of Krita
 *
 * Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_script_dock.h"

#include <QDockWidget>
#include <QVariant>

#include <kross/core/action.h>

KisScriptDockFactory::KisScriptDockFactory(Kross::Action* act) : m_action(act)
{
    m_action->addObject(this, "KritaDockFactory", Kross::ChildrenInterface::AutoConnectSignals);
}

QString KisScriptDockFactory::id() const
{
    return m_action->property("id").toString();
}

QDockWidget* KisScriptDockFactory::createDockWidget()
{
    QVariant v = m_action->callFunction("createDockWidget");
    QDockWidget* qdw = dynamic_cast<QDockWidget*>(v.value<QObject*>());
    if(qdw)
        return qdw;
    kDebug() << "The script didn't delivered its promise of providing a QDockWidget";
    return new QDockWidget();
}
