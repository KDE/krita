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

//#include <qglobal.h>
//#include <qobjectdefs.h>
#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaEnum>

using namespace Kross::Api;

QtObject::QtObject(QObject* object, const QString& name)
    : Class<QtObject>(name)
    , m_object(object)
{
    this->addFunction0<Kross::Api::Variant>("className", this, &QtObject::className);
    this->addFunction0<Kross::Api::Variant>("objectName", this, &QtObject::objectName);
    this->addFunction1<void, Kross::Api::Variant>("setObjectName", this, &QtObject::setObjectName);

    this->addFunction1<Kross::Api::Variant, Kross::Api::Variant>("property", this, &QtObject::property);
    this->addFunction2<Kross::Api::Variant, Kross::Api::Variant, Kross::Api::Variant>("setProperty", this, &QtObject::setProperty);
    this->addFunction0<Kross::Api::Variant>("propertyNames", this, &QtObject::propertyNames);
    this->addFunction1<Kross::Api::Variant, Kross::Api::Variant>("propertyTypeName", this, &QtObject::propertyTypeName);

    this->addFunction1<Kross::Api::Variant, Kross::Api::Variant>("enumeratorName", this, &QtObject::enumeratorName);
    this->addFunction0<Kross::Api::Variant>("enumeratorCount", this, &QtObject::enumeratorCount);
    this->addFunction0<Kross::Api::Variant>("enumeratorNames", this, &QtObject::enumeratorNames);
    this->addFunction1<Kross::Api::Variant, Kross::Api::Variant>("enumeratorIndex", this, &QtObject::enumeratorIndex);

    this->addFunction0<Kross::Api::Variant>("methodNames", this, &QtObject::methodNames);
    //TODO this->addFunction1<Kross::Api::Variant, Kross::Api::Variant>("invokeMethod", this, &QtObject::invokeMethod);

    /*
    QStringList slotnames = m_object->metaObject()->slotNames(false);
    for(char* c = slotnames.first(); c; c = slotnames.next())
        addChild( new EventSlot(c, this, object, c) );
    QStringList signalnames = m_object->metaObject()->signalNames(false);
    for(char* c = signalnames.first(); c; c = signalnames.next())
        addChild( new EventSignal(c, this, object, c) );
    addFunction("slotNames", &QtObject::slotNames);
    addFunction("hasSlot", &QtObject::hasSlot);
    addFunction("slot", &QtObject::callSlot);
    addFunction("signalNames", &QtObject::signalNames);
    addFunction("hasSignal", &QtObject::hasSignal);
    addFunction("signal", &QtObject::emitSignal);
    addFunction("connect", &QtObject::connectSignal);
    addFunction("disconnect", &QtObject::disconnectSignal);
    */
}

QtObject::~QtObject() {
}

QObject* QtObject::getObject() const {
    return m_object;
}

const QString QtObject::className() const {
    return m_object->metaObject()->className();
}

const QString QtObject::objectName() const {
    return m_object->objectName();
}

void QtObject::setObjectName(const QString& name) {
    m_object->setObjectName(name);
}

QVariant QtObject::property(const char* name) {
    return m_object->property(name);
}

bool QtObject::setProperty(const char* name, const QVariant& value) {
    return m_object->setProperty(name, value);
}

QStringList QtObject::propertyNames() const {
    QStringList list;
    const int count = m_object->metaObject()->propertyCount(); 
    for(int i = 0; i < count; ++i)
        list.append( m_object->metaObject()->property(i).name() );
    return list;
}

QString QtObject::propertyTypeName(const char* name) { 
    int index = m_object->metaObject()->indexOfProperty(name);
    return index >= 0 ? m_object->metaObject()->property(index).typeName() : QVariant::typeToName(QVariant::Invalid);
}

QString QtObject::enumeratorName(int index) const {
    return m_object->metaObject()->enumerator(index).name();
}

int QtObject::enumeratorCount() const {
    return m_object->metaObject()->enumeratorCount();
}

QStringList QtObject::enumeratorNames() const {
    QStringList list;
    const int count = m_object->metaObject()->enumeratorCount();
    for(int i = 0; i < count; ++i)
        list.append( m_object->metaObject()->enumerator(i).name() );
    return list;
}

int QtObject::enumeratorIndex(const char* name) const {
    const int i = m_object->metaObject()->indexOfEnumerator(name);
    return m_object->metaObject()->enumerator(i).value(i);
}

QStringList QtObject::methodNames() const {
    QStringList list;
    const int count = m_object->metaObject()->methodCount();
    for(int i = 0; i < count; ++i)
        list.append( m_object->metaObject()->method(i).signature() );
    return list;
}

/*TODO
bool QtObject::invokeMethod(const char* name) {
    Q_RETURN_ARG()
    Q_ARG()
    QMetaObject::invokeMethod(m_object, name, )
}
*/

/*
Kross::Api::Object::Ptr QtObject::callSlot(Kross::Api::List::Ptr args) {
    QString name = Kross::Api::Variant::toString(args->item(0));
    int slotid = m_object->metaObject()->findSlot(name.toLatin1().data(), false);
    if(slotid < 0) throw Exception::Ptr( new Exception(QString("No such slot '%1'.").arg(name)) );
    QUObject* uo = QtObject::toQUObject(name, args);
    m_object->qt_invoke(slotid, uo);
    delete [] uo;
    return new Variant( QVariant(true,0) );
}
Kross::Api::Object::Ptr QtObject::emitSignal(Kross::Api::List::Ptr args) {
    QString name = Kross::Api::Variant::toString(args->item(0));
    int signalid = m_object->metaObject()->findSignal(name.toLatin1().data(), false);
    if(signalid < 0) throw Exception::Ptr( new Exception(QString("No such signal '%1'.").arg(name)) );
    m_object->qt_invoke(signalid, 0); //convert Kross::Api::List::Ptr => QUObject*
    return 0;
}
Kross::Api::Object::Ptr QtObject::connectSignal(Kross::Api::List::Ptr args) {
    QString signalname = Kross::Api::Variant::toString(args->item(0));
    QString signalsignatur = QString("2%1").arg(signalname);
    const char* signalsig = signalsignatur.toLatin1().data();
    QtObject* obj = Kross::Api::Object::fromObject<Kross::Api::QtObject>(args->item(1));
    QObject* o = obj->getObject();
    if(! o) throw Exception::Ptr( new Exception(QString("No such QObject receiver in '%1'.").arg(obj->getName())) );
    QString slotname = Kross::Api::Variant::toString(args->item(2));
    QString slotsignatur = QString("1%1").arg(slotname);
    const char* slotsig = slotsignatur.toLatin1().data();
    return new Kross::Api::Variant(QObject::connect(m_object, signalsig, o, slotsig));
}
Kross::Api::Object::Ptr QtObject::disconnectSignal(Kross::Api::List::Ptr) {
    return 0;
}
*/
