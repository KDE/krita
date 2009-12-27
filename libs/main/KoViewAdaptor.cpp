/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2006 Fredrik Edemar <f_edemar@linux.se>

   $Id: KoViewAdaptor.cc 529520 2006-04-13 16:41:44Z mpfeiffer $

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoViewAdaptor.h"

#include "KoView.h"

#include <kactioncollection.h>
#include <kaction.h>
#include <QList>

KoViewAdaptor::KoViewAdaptor(KoView *view)
        : QDBusAbstractAdaptor(view)
{
    setAutoRelaySignals(true);
    m_pView = view;
}

KoViewAdaptor::~KoViewAdaptor()
{
}


QStringList KoViewAdaptor::actions()
{
    QStringList tmp_actions;
    QList<QAction*> lst = m_pView->actionCollection()->actions();
    foreach(QAction* it, lst) {
        if (it->isEnabled())
            tmp_actions.append(it->objectName());
    }
    return tmp_actions;
}

#include <KoViewAdaptor.moc>
