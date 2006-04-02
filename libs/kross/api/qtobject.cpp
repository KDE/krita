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
//Added by qt3to4:
#include <Q3StrList>
#include <Q3CString>
#include "object.h"
#include "variant.h"
#include "event.h"

#include "../main/manager.h"
#include "eventslot.h"
#include "eventsignal.h"

#include <qobject.h>
#include <q3signal.h>
//#include <qglobal.h>
//#include <qobjectdefs.h>
#include <qmetaobject.h>
//#include <private/qucom_p.h> // for the Qt QUObject API.

using namespace Kross::Api;

QtObject::QtObject(Object::Ptr parent, QObject* object, const QString& name)
    : Kross::Api::Class<QtObject>(name.isEmpty() ? object->name() : name, parent)
    , m_object(object)
{
/*TODO
    // Walk through the signals and slots the QObject has
    // and attach them as events to this QtObject.

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

QObject* QtObject::getObject()
{
    return m_object;
}

#if 0

QUObject* QtObject::toQUObject(const QString& signature, List::Ptr arguments)
{
    int startpos = signature.find("(");
    int endpos = signature.findRev(")");
    if(startpos < 0 || startpos > endpos)
        throw Exception::Ptr( new Exception(QString("Invalid Qt signal or slot signature '%1'").arg(signature)) );

    //QString sig = signature.left(startpos);
    QString params = signature.mid(startpos + 1, endpos - startpos - 1);
    QStringList paramlist = QStringList::split(",", params); // this will fail on something like myslot(QMap<QString,QString> arg), but we don't care jet.
    uint paramcount = paramlist.size();

    // The first item in the QUObject-array is for the returnvalue
    // while everything >=1 are the passed parameters.
    QUObject* uo = new QUObject[ paramcount + 1 ];
    uo[0] = QUObject(); // empty placeholder for the returnvalue.

//QString t;
//for(int j=0; j<argcount; j++) t += "'" + Variant::toString(arguments->item(j)) + "' ";
//kDebug()<<"1 --------------------- ("<<argcount<<"): "<<t<<endl;

    // Fill parameters.
    uint argcount = arguments ? arguments->count() : 0;
    for(uint i = 0; i < paramcount; i++) {
        if(paramlist[i].find("QString") >= 0) {
            const QString s = (argcount > i) ? Variant::toString(arguments->item(i)) : QString::null;
            //kDebug()<<"EventSlot::toQUObject s="<<s<<endl;
            static_QUType_QString.set( &(uo[i + 1]), s );
        }
        //TODO handle int, long, char*, QStringList, etc.
        else {
            throw Exception::Ptr( new Exception(QString("Unknown Qt signal or slot argument '%1' in signature '%2'.").arg(paramlist[i]).arg(signature)) );
        }
    }

//kDebug()<<"2 --------------------- "<<endl;
    return uo;
}

Kross::Api::Object::Ptr QtObject::propertyNames(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant(
        QStringList::fromStrList(m_object->metaObject()->propertyNames(false)),
        "Kross::Api::QtObject::propertyNames::Variant::StringList");
}

Kross::Api::Object::Ptr QtObject::hasProperty(Kross::Api::List::Ptr args)
{
    return new Kross::Api::Variant(
        m_object->metaObject()->findProperty(Kross::Api::Variant::toString(args->item(0)).latin1(), false),
        "Kross::Api::QtObject::hasProperty::Variant::Bool");
}

Kross::Api::Object::Ptr QtObject::getProperty(Kross::Api::List::Ptr args)
{
    QVariant variant = m_object->property(Kross::Api::Variant::toString(args->item(0)).latin1());
    if(variant.type() == QVariant::Invalid)
        return 0;
    return new Kross::Api::Variant(variant, "Kross::Api::QtObject::getProperty::Variant");
}

Kross::Api::Object::Ptr QtObject::setProperty(Kross::Api::List::Ptr args)
{
    return new Kross::Api::Variant(
           m_object->setProperty(
               Kross::Api::Variant::toString(args->item(0)).latin1(),
               Kross::Api::Variant::toVariant(args->item(1))
           ),
           "Kross::Api::QtObject::setProperty::Variant::Bool");
}

Kross::Api::Object::Ptr QtObject::slotNames(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant(
           QStringList::fromStrList(m_object->metaObject()->slotNames(false)),
           "Kross::Api::QtObject::slotNames::Variant::StringList");
}

Kross::Api::Object::Ptr QtObject::hasSlot(Kross::Api::List::Ptr args)
{
    return new Kross::Api::Variant(
           bool(m_object->metaObject()->slotNames(false).find(
               Kross::Api::Variant::toString(args->item(0)).latin1()
           ) != -1),
           "Kross::Api::QtObject::hasSlot::Variant::Bool");
}

Kross::Api::Object::Ptr QtObject::callSlot(Kross::Api::List::Ptr args)
{
//TODO just call the child event ?!
    QString name = Kross::Api::Variant::toString(args->item(0));
    int slotid = m_object->metaObject()->findSlot(name.latin1(), false);
    if(slotid < 0)
        throw Exception::Ptr( new Exception(QString("No such slot '%1'.").arg(name)) );

    QUObject* uo = QtObject::toQUObject(name, args);
    m_object->qt_invoke(slotid, uo);
    delete [] uo;

    return new Variant(true, "Kross::Api::QtObject::Bool");
}

Kross::Api::Object::Ptr QtObject::signalNames(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant(
           QStringList::fromStrList(m_object->metaObject()->signalNames(false)),
           "Kross::Api::QtObject::signalNames::Variant::StringList");
}

Kross::Api::Object::Ptr QtObject::hasSignal(Kross::Api::List::Ptr args)
{
    return new Kross::Api::Variant(
           bool(m_object->metaObject()->signalNames(false).find(
               Kross::Api::Variant::toString(args->item(0)).latin1()
           ) != -1),
           "Kross::Api::QtObject::hasSignal::Variant::Bool");
}

Kross::Api::Object::Ptr QtObject::emitSignal(Kross::Api::List::Ptr args)
{
    QString name = Kross::Api::Variant::toString(args->item(0));
    int signalid = m_object->metaObject()->findSignal(name.latin1(), false);
    if(signalid < 0)
        throw Exception::Ptr( new Exception(QString("No such signal '%1'.").arg(name)) );
    m_object->qt_invoke(signalid, 0); //TODO convert Kross::Api::List::Ptr => QUObject*
    return 0;
}

Kross::Api::Object::Ptr QtObject::connectSignal(Kross::Api::List::Ptr args)
{
    QString signalname = Kross::Api::Variant::toString(args->item(0));
    QString signalsignatur = QString("2%1").arg(signalname);
    const char* signalsig = signalsignatur.latin1();

    QtObject* obj = Kross::Api::Object::fromObject<Kross::Api::QtObject>(args->item(1));
    QObject* o = obj->getObject();
    if(! o)
        throw Exception::Ptr( new Exception(QString("No such QObject receiver in '%1'.").arg(obj->getName())) );

    QString slotname = Kross::Api::Variant::toString(args->item(2));
    QString slotsignatur = QString("1%1").arg(slotname);
    const char* slotsig = slotsignatur.latin1();

    return new Kross::Api::Variant(
           QObject::connect(m_object, signalsig, o, slotsig),
           "Kross::Api::QtObject::connect::Bool");
}

Kross::Api::Object::Ptr QtObject::disconnectSignal(Kross::Api::List::Ptr)
{
    //TODO
    return 0;
}

#endif
