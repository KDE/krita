/***************************************************************************
 * eventslot.cpp
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

#include "eventslot.h"

#include "variant.h"
#include "qtobject.h"

#include <qmetaobject.h>
//Added by qt3to4:
#include <Q3CString>

using namespace Kross::Api;

EventSlot::EventSlot(const QString& name, Object* parent, QObject* receiver, Q3CString slot)
    : Event<EventSlot>(name, parent)
    , m_receiver(receiver)
    , m_slot(slot) //QObject::normalizeSignalSlot(slot)
{
}

EventSlot::~EventSlot()
{
}

const QString EventSlot::getClassName() const
{
    return "Kross::Api::EventSlot";
}

Object::Ptr EventSlot::call(const QString& /*name*/, List::Ptr arguments)
{
#ifdef KROSS_API_EVENTSLOT_CALL_DEBUG
    krossdebug( QString("EventSlot::call() name=%1 m_slot=%2 arguments=%3").arg(name).arg(m_slot).arg(arguments->toString()) );
#endif

///\todo implement new QMetaObject-stuff
/*
    QString n = m_slot; //TODO name; //Variant::toString(args->item(0));

    if(n.startsWith("1")) // Remove prefix of SLOT-macros
        n.remove(0,1);

    int slotid = m_receiver->metaObject()->findSlot(n.latin1(), false);
    if(slotid < 0)
        throw Exception::Ptr( new Exception(QString("No such slot '%1'.").arg(n)) );

    QUObject* uo = QtObject::toQUObject(n, arguments);
    m_receiver->qt_invoke(slotid, uo); // invoke the slot
    delete [] uo;

    return new Variant(true, "Kross::Api::EventSlot::Bool");
*/
    return Object::Ptr();
}

/*
QCString EventSlot::getSlot(const QCString& signal)
{
    QString signature = QString(signal).mid(1);
    int startpos = signature.find("(");
    int endpos = signature.findRev(")");
    if(startpos < 0 || startpos > endpos) {
        krosswarning( QString("EventSlot::getSlot(%1) Invalid signal.").arg(signal) );
        return QCString();
    }
    QString signalname = signature.left(startpos);
    QString params = signature.mid(startpos + 1, endpos - startpos - 1);
    //QStringList paramlist = QStringList::split(",", params);
    QCString slot = QString("callback(" + params + ")").latin1(); //normalizeSignalSlot();

    QMetaObject* mo = metaObject();
    int slotid = mo->findSlot(slot, false);
    if(slotid < 0) {
        krossdebug( QString("EventSlot::getSlot(%1) No such slot '%2' avaiable.").arg(signal).arg(slot) );
        return QCString();
    }

    const QMetaData* md = mo->slot(slotid, false);
    if(md->access != QMetaData::Public) {
        krossdebug( QString("EventSlot::getSlot(%1) The slot '%2' is not public.").arg(signal).arg(slot) );
        return QCString();
    }

//QMember* member = md->member;
//const QUMethod *method = md->method;

    krossdebug( QString("signal=%1 slot=%2 slotid=%3 params=%4 mdname=%5")
        .arg(signal).arg(slot).arg(slotid).arg(params).arg(md->name) );
    return QCString("1" + slot); // Emulate the SLOT(...) macro by adding as first char a "1".
}

bool EventSlot::connect(EventManager* eventmanager, QObject* senderobj, const QCString& signal, QString function, const QCString& slot)
{
    if(m_sender && ! disconnect())
        return false;

    const QCString& myslot = slot.isEmpty() ? getSlot(signal) : slot;
    if(! myslot)
        return false;

    if(! m_eventmanager) {
        EventSlot* eventslot = create(eventmanager);
        eventslot->connect(eventmanager, senderobj, signal, function, slot);
        m_slots.append(eventslot);
        krossdebug( QString("EventSlot::connect(%1, %2, %3) added child EventSlot !!!").arg(senderobj->name()).arg(signal).arg(function) );
    }
    else {
        m_sender = senderobj;
        m_signal = signal;
        m_function = function;
        m_slot = myslot;
        if(! QObject::connect((QObject*)senderobj, signal, this, myslot)) {
            krossdebug( QString("EventSlot::connect(%1, %2, %3) failed.").arg(senderobj->name()).arg(signal).arg(function) );
            return false;
        }
        krossdebug( QString("EventSlot::connect(%1, %2, %3) successfully connected.").arg(senderobj->name()).arg(signal).arg(function) );
    }
    return true;
}

bool EventSlot::disconnect()
{
    if(! m_sender) return false;
    QObject::disconnect((QObject*)m_sender, m_signal, this, m_slot);
    m_sender = 0;
    m_signal = 0;
    m_slot = 0;
    m_function = QString::null;
    return true;
}

void EventSlot::call(const QVariant& variant)
{
    krossdebug( QString("EventSlot::call() sender='%1' signal='%2' function='%3'")
                 .arg(m_sender->name()).arg(m_signal).arg(m_function) );

    Kross::Api::List* arglist = 0;

    QValueList<Kross::Api::Object*> args;
    if(variant.isValid()) {
        args.append(Kross::Api::Variant::create(variant));
        arglist = Kross::Api::List::create(args);
    }

    try {
        m_eventmanager->m_scriptcontainer->callFunction(m_function, arglist);
    }
    catch(Exception& e) {
        //TODO add hadError(), getError() and setError()
        krossdebug( QString("EXCEPTION in EventSlot::call('%1') type='%2' description='%3'").arg(variant.toString()).arg(e.type()).arg(e.description()) );
    }
}

void EventSlot::callback() {
    call(QVariant()); }
void EventSlot::callback(short s) {
    call(QVariant(s)); }
void EventSlot::callback(int i) {
    call(QVariant(i)); }
void EventSlot::callback(int i1, int i2) {
    call(QVariant( QValueList<QVariant>() << i1 << i2 )); }
void EventSlot::callback(int i1, int i2, int i3) {
    call(QVariant( QValueList<QVariant>() << i1 << i2 << i3 )); }
void EventSlot::callback(int i1, int i2, int i3, int i4) {
    call(QVariant( QValueList<QVariant>() << i1 << i2 << i3 << i4 )); }
void EventSlot::callback(int i1, int i2, int i3, int i4, int i5) {
    call(QVariant( QValueList<QVariant>() << i1 << i2 << i3 << i4 << i5 )); }
void EventSlot::callback(int i1, int i2, int i3, int i4, bool b) {
    call(QVariant( QValueList<QVariant>() << i1 << i2 << i3 << i4 << b )); }
void EventSlot::callback(int i1, bool b) {
    call(QVariant( QValueList<QVariant>() << i1 << b )); }
void EventSlot::callback(int i1, int i2, bool b) {
    call(QVariant( QValueList<QVariant>() << i1 << i2 << b )); }
void EventSlot::callback(int i1, int i2, const QString& s) {
    call(QVariant( QValueList<QVariant>() << i1 << i2 << s )); }
void EventSlot::callback(uint i) {
    call(QVariant(i)); }
void EventSlot::callback(long l) {
    call(QVariant((Q_LLONG)l)); }
void EventSlot::callback(ulong l) {
    call(QVariant((Q_ULLONG)l)); }
void EventSlot::callback(double d) {
    call(QVariant(d)); }
void EventSlot::callback(const char* c) {
    call(QVariant(c)); }
void EventSlot::callback(bool b) {
    call(QVariant(b)); }
void EventSlot::callback(const QString& s) {
    call(QVariant(s)); }
void EventSlot::callback(const QString& s, int i) {
    call(QVariant( QValueList<QVariant>() << s << i )); }
void EventSlot::callback(const QString& s, int i1, int i2) {
    call(QVariant( QValueList<QVariant>() << s << i1 << i2 )); }
void EventSlot::callback(const QString& s, uint i) {
    call(QVariant( QValueList<QVariant>() << s << i )); }
void EventSlot::callback(const QString& s, bool b) {
    call(QVariant( QValueList<QVariant>() << s << b )); }
void EventSlot::callback(const QString& s, bool b1, bool b2) {
    call(QVariant( QValueList<QVariant>() << s << b1 << b2 )); }
void EventSlot::callback(const QString& s, bool b, int i) {
    call(QVariant( QValueList<QVariant>() << s << b << i )); }
void EventSlot::callback(const QString& s1, const QString& s2) {
    call(QVariant( QValueList<QVariant>() << s1 << s2 )); }
void EventSlot::callback(const QString& s1, const QString& s2, const QString& s3) {
    call(QVariant( QValueList<QVariant>() << s1 << s2 << s3 )); }
void EventSlot::callback(const QStringList& sl) {
    call(QVariant(sl)); }
void EventSlot::callback(const QVariant& variant) {
    call(variant); }
*/
