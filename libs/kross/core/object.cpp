/***************************************************************************
 * object.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
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

#include "object.h"
#include "metaobject.h"

//#include <QMetaObject>
//#include <QMetaMethod>
#include <QPointer>

using namespace Kross;

namespace Kross {

    /**
    * \internal static QMetaObject implementation for the \a Object class.
    *
    * The relaySlot slot has local ID 0 (we use this when calling QMetaObject::connect) it also gets called with the void** array
    *
    * signature;
    *     e.g. "relay(QObject*,const QMetaObject*,int,QVariantList)"
    * parameters;
    *     e.g. ",,," or "obj,metaobject,sid,args"
    * type;
    *     the return value
    * tag;
    *     afaik unused yet
    * flags;
    *     enum MethodFlags {
    *         AccessPrivate = 0x00,
    *         AccessProtected = 0x01,
    *         AccessPublic = 0x02,
    *         MethodMethod = 0x00,
    *         MethodSignal = 0x04,
    *         MethodSlot = 0x08,
    *         MethodCompatibility = 0x10,
    *         MethodCloned = 0x20,
    *         MethodScriptable = 0x40
    *     };
    */
    static const uint qt_meta_data_Kross__Object[] = {
        // content:
        1,       // revision
        0,       // classname
        0,    0, // classinfo
        1,   10, // methods
        0,    0, // properties
        0,    0, // enums/sets
        // slots: signature, parameters, type, tag, flags
       19,   15,   14,   14, 0x0a,
       0        // eod
    };
    static const char qt_meta_stringdata_Kross__Object[] = {
        "Kross::Object\0\0,,,\0"
        "relay(QObject*,const QMetaObject*,int,QVariantList)\0"
    };
    const QMetaObject Object::staticMetaObject = {
        { &QObject::staticMetaObject, qt_meta_stringdata_Kross__Object, qt_meta_data_Kross__Object, 0 }
    };

    /// \internal d-pointer class for \a Object .
    class Object::Private
    {
        public:

            /**
             * The QMetaObject implementation that provides us a dynamic way to
             * deal with Qt's meta-functionality like signals, slots and properties.
             */
            MetaObject* metaobject;

            /**
             * The wrapped QObject instance or NULL if no QObject should be wrapped.
             * The pointer is guarded, what means it will be automaticly set to NULL
             * if the object got destroyed.
             */
            QPointer<QObject> object;

            Private(QObject* const obj) : metaobject(0), object(obj) {}
    };

}

Object::Object(QObject* const object, const QString& name)
    : QObject()
    , KShared()
    , d(new Private(object))
{
    setObjectName( (object && name.isNull()) ? object->objectName() : name );
    krossdebug(QString("Object::Constructor name=%1").arg(objectName()));
}

Object::~Object()
{
    krossdebug(QString("Object::Destructor name=%1").arg(objectName()));
    delete d;
}

QObject* Object::object() const
{
    return d->object;
}

const QMetaObject* Object::metaObject() const
{
    if(! d->metaobject)
        d->metaobject = new MetaObject(this);
    return d->metaobject;
}

/*
void *Kross::Object::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Kross__Object))
	  return static_cast<void*>(const_cast<Object*>(this));
    if (!strcmp(_clname, "KShared"))
	  return static_cast<KShared*>(const_cast<Object*>(this));
    return QObject::qt_metacast(_clname);
}

int Kross::Object::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: relay((*reinterpret_cast< QObject*(*)>(_a[1])),(*reinterpret_cast< const QMetaObject*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< QVariantList(*)>(_a[4]))); break;
        }
        _id -= 1;
    }
    return _id;
}
*/

void* Object::qt_metacast(const char* classname)
{
    krossdebug( QString("Object::qt_metacast classname=%1 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$").arg(classname) );
    if(! classname)
        return 0;
    if(strcmp(classname, qt_meta_stringdata_Kross__Object) == 0)
        return static_cast< void* >( const_cast< Object* >(this) );
    if(strcmp(classname, "KShared") == 0)
        return static_cast< KShared* >( const_cast< Object* >(this) );
    //if(strcmp(classname, d->object->metaObject()->className()) == 0)
    //    return static_cast< void* >( const_cast< QObject* >( d->object ) );
    return QObject::qt_metacast(classname);
}

int Object::qt_metacall(QMetaObject::Call call, int id, void** args)
{
    Q_ASSERT( d->metaobject );

    switch(call) {

        case QMetaObject::InvokeMetaMethod: {
            const QMetaMethod method = d->metaobject->method(id);

            #ifdef KROSS_OBJECT_METACALL_DEBUG
                krossdebug( QString("Object::qt_metacall InvokeMetaMethod id=%1 methodOffset=%2 methodCount=%3").arg(id).arg(d->metaobject->methodOffset()).arg(d->metaobject->methodCount()) );
                for(int i = 0; i < d->metaobject->methodCount(); ++i)
                    krossdebug( QString("  method index=%1 sig=%2").arg(i).arg( d->metaobject->method(i).signature() ) );
                krossdebug( QString("Object::qt_metacall id=%1 signature=%2 tag=%3 typeName=%4 paramsize=%5").arg(id).arg(method.signature()).arg(method.tag()).arg(method.typeName()).arg(method.parameterTypes().size()) );
                foreach(QByteArray t, method.parameterTypes())
                    krossdebug( QString("  param type=%1").arg(t.constData()) );
            #endif

            // Now we need to determinate if the method is within the wrapped object
            if(d->object) {
                const QMetaObject* mo = d->object->metaObject();
                const int mid = mo->indexOfMethod( method.signature() ); //TODO compute matching id to gain more performance
                if(mid >= 0) {
                    const QMetaMethod m = mo->method(mid);
                    #ifdef KROSS_OBJECT_METACALL_DEBUG
                        krossdebug( QString("Object::qt_metacall redirect call to wrapped object=%1 classname=%2 id=%3 signature=%4").arg(d->object->objectName()).arg(mo->className()).arg(mid).arg(m.signature()) );
                        foreach(QByteArray t, m.parameterTypes())
                            krossdebug( QString("  param type=%1").arg(t.constData()) );
                    #endif

                    krossdebug("Object::qt_metacall CALL BEGIN");
                    int resultid = d->object->qt_metacall(QMetaObject::InvokeMetaMethod, mid, args);
                    krossdebug("Object::qt_metacall CALL END");
                    return resultid;

                }
            }

            //TODO implement dynamic methods
            krossdebug("TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

            /*
            const int methodoffset = d->metaobject->methodOffset();
            if(id <= methodoffset) { // method is a builtin method.
                krossdebug( QString("Object::qt_metacall method id=%1 with signature=%2 is a builtin method").arg(id).arg(method.signature()) );
                const int objid = QObject::qt_metacall(call, id, args); // try to redirect the call
                if(objid < 0) return objid; // if QObject handled the call successfully
                krosswarning( QString("Object::qt_metacall method id=%1 with signature=%2 is unhandled!").arg(id).arg(method.signature()) );
            }
            else { // the method is a wrapped method.
                krossdebug( QString("Object::qt_metacall id=%1 with signature=%2 is a wrapped method").arg(id).arg(method.signature()) );
            const QMetaObject* mo = d->object->metaObject();
            krossdebug( QString("Object::qt_metacall id=%1 methodOffset=%2 methodCount=%3").arg(id).arg(mo->methodOffset()).arg(mo->methodCount()) );
            for(int i = 0; i < mo->methodCount(); ++i)
                krossdebug( QString("  method index=%1 sig=%2").arg(i).arg( mo->method(i).signature() ) );
            //return d->object->qt_metacall(call,id,args);
            //QMetaObject::invokeMethod(object  );
            */
        } break;

        //case QMetaObject::ReadProperty:
        //case QMetaObject::WriteProperty:
        //case QMetaObject::ResetProperty:
        //case QMetaObject::QueryPropertyDesignable:
        //case QMetaObject::QueryPropertyScriptable:
        //case QMetaObject::QueryPropertyStored:
        //case QMetaObject::QueryPropertyEditable:
        //case QMetaObject::QueryPropertyUser:
        default: {
            krosswarning( QString("Object::qt_metacall call=%1 for id=%2 not supported!").arg(call).arg(id) );
        } break;
    }

    return 0; // failed...
}

void Object::relay(QObject* object, const QMetaObject* metaobject, int index, QVariantList args)
{
    //TODO implement functionality to redirect/relay QMetaObject's...
    Q_UNUSED(metaobject);
    QString s;
    foreach(QVariant v, args) s += v.toString() + ",";
    krossdebug( QString("Object::relay called objectName=%1 className=%2 index=%3 args=%4").arg(object ? object->objectName() : "NULL").arg(object ? object->metaObject()->className() : "NULL").arg(index).arg(s) );
}
