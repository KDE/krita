/***************************************************************************
 * qtobject.cpp
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

#include "qtobject.h"

#include "object.h"
#include "variant.h"
#include "event.h"
#include "proxy.h"

#include "../main/manager.h"
//#include "eventslot.h"
//#include "eventsignal.h"

#include <QObject>
//#include <qglobal.h>
//#include <qobjectdefs.h>
#include <QMetaObject>

using namespace Kross::Api;

QtObject::QtObject(QObject* object, const QString& name)
    : Class<QtObject>(name)
    , m_object(object)
{
    this->addFunction0<Kross::Api::Variant>("className", m_object, &QObject::className);

    this->addFunction0<Kross::Api::Variant>("objectName", m_object, &QObject::objectName);
    this->addFunction1<void, Kross::Api::Variant>("setObjectName", m_object, &QObject::setObjectName);

    this->addFunction1<Kross::Api::Variant, Kross::Api::Variant>("property", m_object, &QObject::property);
    this->addFunction2<Kross::Api::Variant, Kross::Api::Variant, Kross::Api::Variant>("setProperty", m_object, &QObject::setProperty);



    // Walk through the signals and slots the QObject has
    // and attach them as events to this QtObject.
///\todo implement new QMetaObject-stuff
/*
    Q3StrList slotnames = m_object->metaObject()->slotNames(false);
    for(char* c = slotnames.first(); c; c = slotnames.next()) {
        Q3CString s = c;
        addChild( new EventSlot(s, this, object, s) );
    }

    Q3StrList signalnames = m_object->metaObject()->signalNames(false);
    for(char* c = signalnames.first(); c; c = signalnames.next()) {
        Q3CString s = c;
        addChild( new EventSignal(s, this, object, s) );
    }

    // Add functions to wrap QObject methods into callable
    // Kross objects.

    addFunction("propertyNames", &QtObject::propertyNames);
    addFunction("hasProperty", &QtObject::hasProperty,
        Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String"));
    addFunction("getProperty", &QtObject::getProperty,
        Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String"));
    addFunction("setProperty", &QtObject::setProperty,
        Kross::Api::ArgumentList()
            << Kross::Api::Argument("Kross::Api::Variant::String")
            << Kross::Api::Argument("Kross::Api::Variant"));

    addFunction("slotNames", &QtObject::slotNames);
    addFunction("hasSlot", &QtObject::hasSlot,
        Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String"));
    addFunction("slot", &QtObject::callSlot,
        Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String"));

    addFunction("signalNames", &QtObject::signalNames);
    addFunction("hasSignal", &QtObject::hasSignal,
        Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String"));
    addFunction("signal", &QtObject::emitSignal,
        Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String"));

    addFunction("connect", &QtObject::connectSignal,
        Kross::Api::ArgumentList()
            << Kross::Api::Argument("Kross::Api::Variant::String")
            << Kross::Api::Argument("Kross::Api::QtObject")
            << Kross::Api::Argument("Kross::Api::Variant::String"));
    addFunction("disconnect", &QtObject::disconnectSignal,
        Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String"));
*/
}

QtObject::~QtObject()
{
}

const QString QtObject::getClassName() const
{
    return "Kross::Api::QtObject";
}

QObject* QtObject::getObject() const
{
    return m_object;
}

/*
Kross::Api::Object::Ptr QtObject::propertyNames(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant(
        QStringList::fromStrList(m_object->metaObject()->propertyNames(false)));
}

Kross::Api::Object::Ptr QtObject::hasProperty(Kross::Api::List::Ptr args)
{
    return new Kross::Api::Variant(
        m_object->metaObject()->findProperty(Kross::Api::Variant::toString(args->item(0)).toLatin1().data(), false));
}

Kross::Api::Object::Ptr QtObject::getProperty(Kross::Api::List::Ptr args)
{
    QVariant variant = m_object->property(Kross::Api::Variant::toString(args->item(0)).toLatin1().data());
    if(variant.type() == QVariant::Invalid)
        return 0;
    return new Kross::Api::Variant(variant);
}

Kross::Api::Object::Ptr QtObject::setProperty(Kross::Api::List::Ptr args)
{
    return new Kross::Api::Variant(
           m_object->setProperty(
               Kross::Api::Variant::toString(args->item(0)).toLatin1().data(),
               Kross::Api::Variant::toVariant(args->item(1))
           ));
}

Kross::Api::Object::Ptr QtObject::slotNames(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant(
           QStringList::fromStrList(m_object->metaObject()->slotNames(false)));
}

Kross::Api::Object::Ptr QtObject::hasSlot(Kross::Api::List::Ptr args)
{
    return new Kross::Api::Variant(
           bool(m_object->metaObject()->slotNames(false).find(
               Kross::Api::Variant::toString(args->item(0)).toLatin1().data()
           ) != -1));
}

Kross::Api::Object::Ptr QtObject::callSlot(Kross::Api::List::Ptr args)
{
//TODO just call the child event ?!
    QString name = Kross::Api::Variant::toString(args->item(0));
    int slotid = m_object->metaObject()->findSlot(name.toLatin1().data(), false);
    if(slotid < 0)
        throw Exception::Ptr( new Exception(QString("No such slot '%1'.").arg(name)) );

    QUObject* uo = QtObject::toQUObject(name, args);
    m_object->qt_invoke(slotid, uo);
    delete [] uo;

    return new Variant(true);
}

Kross::Api::Object::Ptr QtObject::signalNames(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant(
           QStringList::fromStrList(m_object->metaObject()->signalNames(false)));
}

Kross::Api::Object::Ptr QtObject::hasSignal(Kross::Api::List::Ptr args)
{
    return new Kross::Api::Variant(
           bool(m_object->metaObject()->signalNames(false).find(
               Kross::Api::Variant::toString(args->item(0)).toLatin1().data()
           ) != -1));
}

Kross::Api::Object::Ptr QtObject::emitSignal(Kross::Api::List::Ptr args)
{
    QString name = Kross::Api::Variant::toString(args->item(0));
    int signalid = m_object->metaObject()->findSignal(name.toLatin1().data(), false);
    if(signalid < 0)
        throw Exception::Ptr( new Exception(QString("No such signal '%1'.").arg(name)) );
    m_object->qt_invoke(signalid, 0); //TODO convert Kross::Api::List::Ptr => QUObject*
    return 0;
}

Kross::Api::Object::Ptr QtObject::connectSignal(Kross::Api::List::Ptr args)
{
    QString signalname = Kross::Api::Variant::toString(args->item(0));
    QString signalsignatur = QString("2%1").arg(signalname);
    const char* signalsig = signalsignatur.toLatin1().data();

    QtObject* obj = Kross::Api::Object::fromObject<Kross::Api::QtObject>(args->item(1));
    QObject* o = obj->getObject();
    if(! o)
        throw Exception::Ptr( new Exception(QString("No such QObject receiver in '%1'.").arg(obj->getName())) );

    QString slotname = Kross::Api::Variant::toString(args->item(2));
    QString slotsignatur = QString("1%1").arg(slotname);
    const char* slotsig = slotsignatur.toLatin1().data();

    return new Kross::Api::Variant(
           QObject::connect(m_object, signalsig, o, slotsig));
}

Kross::Api::Object::Ptr QtObject::disconnectSignal(Kross::Api::List::Ptr)
{
    //TODO
    return 0;
}
*/
