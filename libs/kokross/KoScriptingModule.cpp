/***************************************************************************
 * KoScriptingModule.cpp
 * This file is part of the KDE project
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
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
 ***************************************************************************/

#include "KoScriptingModule.h"

// qt
#include <QApplication>
#include <QPointer>
// kde
//#include <kdebug.h>
//#include <klocale.h>
//#include <kxmlguiwindow.h>
// koffice
#include <KoMainWindow.h>
#include <KoApplicationAdaptor.h>
#include <KoDocumentAdaptor.h>
#include <KoView.h>

#include <kross/core/manager.h>
//#include <kross/core/interpreter.h>
#include <kross/core/action.h>
//#include <kross/core/model.h>
//#include <kross/core/guiclient.h>
//#include <core/actioncollection.h>

/// \internal d-pointer class.
class KoScriptingModule::Private
{
    public:
        KoScriptingModule* module;
        KoView* view;
};

KoScriptingModule::KoScriptingModule(QObject* parent, const QString& name)
    : QObject(parent)
    , d(new Private())
{
    setObjectName(name);

    d->view = dynamic_cast< KoView* >(parent);
    //if( d->view ) KoMainWindow* mainwindow = d->view->shell();
}

KoScriptingModule::~KoScriptingModule()
{
    delete d;
}

KoView* KoScriptingModule::view() const
{
    return d->view;
}

/*
void KoScriptingModule::setView(KoView* view)
{
    d->view = view;
}
*/

QObject* KoScriptingModule::application()
{
    return qApp->findChild< KoApplicationAdaptor* >();
}

QObject* KoScriptingModule::shell()
{
    return d->view ? d->view->shell() : 0;
}

QWidget* KoScriptingModule::mainWindow()
{
    return d->view ? d->view->mainWindow() : 0;
}

QObject* KoScriptingModule::document()
{
    KoDocument* kdoc = doc();
    return kdoc ? kdoc->findChild< KoDocumentAdaptor* >() : 0;
}

#include "KoScriptingModule.moc"
