/*
 * This file is part of the KOffice project
 *
 * Copyright (C) 2006-2007 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoScriptingModule.h"

// qt
#include <QApplication>
// kde
//#include <kdebug.h>
//#include <klocale.h>
#include <kxmlguiwindow.h>
// koffice
#include <KoMainWindow.h>
#include <KoApplicationAdaptor.h>
#include <KoDocumentAdaptor.h>
#include <KoView.h>

//#include <kross/core/manager.h>
//#include <kross/core/model.h>
//#include <kross/core/guiclient.h>
//#include <core/actioncollection.h>

KoScriptingModule::KoScriptingModule(const QString& name)
    : QObject()
{
    setObjectName(name);
}

KoScriptingModule::~KoScriptingModule()
{
}

KoView* KoScriptingModule::view() const
{
    return m_view;
}

void KoScriptingModule::setView(KoView* view)
{
    m_view = view;
}

QObject* KoScriptingModule::application()
{
    return qApp->findChild< KoApplicationAdaptor* >();
}

QObject* KoScriptingModule::shell()
{
    return m_view ? m_view->shell() : 0;
}

QWidget* KoScriptingModule::mainWindow()
{
    return m_view ? m_view->mainWindow() : 0;
}

QObject* KoScriptingModule::document()
{
    return doc()->findChild< KoDocumentAdaptor* >();
}

#include "KoScriptingModule.moc"
