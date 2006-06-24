/***************************************************************************
 * mainmodule.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
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

#include "mainmodule.h"

using namespace Kross::Api;

namespace Kross { namespace Api {

    /// \internal
    class MainModulePrivate
    {
        public:
           /**
            * The \a Exception this \a MainModule throwed or
            * NULL if we don't had an exception.
            */
            Exception::Ptr exception;
    };

}}

MainModule::MainModule(const QString& name)
    : Module(name)
    , d(new MainModulePrivate())
{
    d->exception = 0;
}

MainModule::~MainModule()
{
    delete d;
}

const QString MainModule::getClassName() const
{
    return "Kross::Api::MainModule";
}

bool MainModule::hadException()
{
    return d->exception.data() != 0;
}

Exception* MainModule::getException()
{
    return d->exception.data();
}

void MainModule::setException(Exception* exception)
{
    d->exception = Exception::Ptr(exception);
}

QtObject::Ptr MainModule::addQObject(QObject* object, const QString& name)
{
    QtObject* qtobject = new QtObject(object, name);
    if(! addChild(name, qtobject)) {
        krosswarning( QString("Failed to add QObject name='%1'").arg(object->objectName()) );
        delete qtobject;
        return QtObject::Ptr(0);
    }
    return QtObject::Ptr(qtobject);
}
