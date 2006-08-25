/***************************************************************************
 * rubyinterpreter.cpp
 * This file is part of the KDE project
 * copyright (C)2005 by Cyrille Berger (cberger@cberger.net)
 * copyright (C)2006 by Sebastian Sauer (mail@dipe.org)
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

#include "rubyextension.h"

#include <st.h>

#include <QMap>
#include <QString>
#include <QPointer>
#include <QMetaObject>
#include <QMetaMethod>
#include <QHash>

#include "rubyconfig.h"

using namespace Kross;

namespace Kross {

    class RubyExtensionPrivate {
        friend class RubyExtension;

        /// The wrapped QObject.
        QPointer<QObject> m_object;

        /// The wrapped krossobject
        static VALUE s_krossObject;

        //static VALUE s_krossException;

        /// The cached list of methods.
        QHash<QByteArray, int>* m_methods;

        RubyExtensionPrivate(QObject* object) : m_object(object), m_methods(0) {}
        ~RubyExtensionPrivate() { delete m_methods; }

        /// Update the cached list of methods.
        void updateMethods() {
            delete m_methods;
            m_methods = new QHash<QByteArray, int>();
            if(m_object) {
                const QMetaObject* metaobject = m_object->metaObject();
                const int count = metaobject->methodCount();
                for(int i = 0; i < count; ++i) {
                    QMetaMethod member = metaobject->method(i);
                    const QString signature = member.signature();
                    const QByteArray name = signature.left(signature.indexOf('(')).toLatin1();
                    if(! m_methods->contains(name)) {
                        m_methods->insert(name, i);
                    }
                }
            }
        }
    };

}

VALUE RubyExtensionPrivate::s_krossObject = 0;
//VALUE RubyExtensionPrivate::s_krossException = 0;

RubyExtension::RubyExtension(QObject* object)
    : d(new RubyExtensionPrivate(object))
{
    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug(QString("RubyExtension Ctor QObject=%1").arg( object ? QString("%1 %2").arg(object->objectName()).arg(object->metaObject()->className()) : "NULL" ));
    #endif
}

RubyExtension::~RubyExtension()
{
    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug("RubyExtension Dtor");
    #endif

    delete d;
}

VALUE RubyExtension::method_missing(int argc, VALUE *argv, VALUE self)
{
    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug("method_missing(argc, argv, self)");
    #endif

    if(argc < 1) {
        return 0;
    }

    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug("Converting self to QObject");
    #endif

#if 0
    QObject* object = toObject( self );
    return RubyExtension::call_method(object, argc, argv);
//#else
    RubyExtension* extension = toObject( self );
    Q_ASSERT(extension);
    return RubyExtension::call_method(extension, argc, argv);
#endif
    return Qfalse;
}

VALUE RubyExtension::call_method(RubyExtension* extension, int argc, VALUE *argv)
{
    QByteArray funcname = rb_id2name(SYM2ID(argv[0]));
    const int argumentcount = argc - 1;

    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug(QString("RubyExtension::call_method method=%1 argumentcount=%2").arg(funcname.constData()).arg(argumentcount));
    #endif

    Q_ASSERT(extension);
    Q_ASSERT(extension->d->m_object);

    if(! extension->d->m_methods) {
        extension->d->updateMethods();
        Q_ASSERT(extension->d->m_methods);
    }

    if(! extension->d->m_methods->contains(funcname)) {
        krosswarning( QString("RubyExtension::call_method No such method '%1'").arg(funcname.constData()) );
        return Qfalse;
    }

    int methodindex = extension->d->m_methods->operator[](funcname);
    //int methodindex = object->metaObject()->indexOfMethod( funcname.toLatin1().constData() );
    if(methodindex < 0) {
        krosswarning(QString("No such function '%1'").arg(funcname.constData()));
        return Qfalse;
    }

    QObject* object = extension->d->m_object;
    QMetaMethod metamethod = object->metaObject()->method( methodindex );
    if(metamethod.parameterTypes().size() != argumentcount) {
        bool found = false;
        const int count = object->metaObject()->methodCount();
        for(++methodindex; methodindex < count; ++methodindex) {
            metamethod = object->metaObject()->method( methodindex );
            const QString signature = metamethod.signature();
            const QByteArray name = signature.left(signature.indexOf('(')).toLatin1();
            if(name == funcname && metamethod.parameterTypes().size() == argumentcount) {
                found = true;
                break;
            }
        }
        if(! found) {
            krosswarning(QString("The function '%1' does not expect %2 arguments.").arg(funcname.constData()).arg(argumentcount));
            return Qfalse;
        }
    }

    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug( QString("  QMetaMethod idx=%1 sig=%2 tag=%3 type=%4").arg(methodindex).arg(metamethod.signature()).arg(metamethod.tag()).arg(metamethod.typeName()) );
    #endif

    {
        QList<QByteArray> typelist = metamethod.parameterTypes();
        const int typelistcount = typelist.count();
        bool hasreturnvalue = strcmp(metamethod.typeName(),"") != 0;

        Q_ASSERT(typelistcount < 10);

        krosswarning("TODOOOOOOOOOOOOOOOO !!!!!!!!!!!!!!!!!!!!!!");
        //TODO

    }

#if 0
    QList<Api::Object::Ptr> argsList;
    for(int i = 1; i < argc; i++)
    {
        QObject* obj = toObject(argv[i]);
        if(obj) argsList.append(obj);
    }
    QObject* result;
    try { // We need a double try/catch because, the cleaning is only done at the end of the catch, so if we had only one try/catch, kross would crash after the call to rb_exc_raise
        try { // We can't let a C++ exceptions propagate in the C mechanism
            Kross::Callable* callable = dynamic_cast<Kross::Callable*>(object.data());
            if(callable && callable->hasChild(funcname)) {
                #ifdef KROSS_RUBY_EXTENSION_DEBUG
                    krossdebug( QString("Kross::Ruby::RubyExtension::method_missing name='%1' is a child object of '%2'.").arg(funcname).arg(object->getName()) );
                #endif
                result = callable->getChild(funcname)->call(QString::null, KSharedPtr<Kross::List>(new Api::List(argsList)));
            }
            else {
                #ifdef KROSS_RUBY_EXTENSION_DEBUG
                    krossdebug( QString("Kross::Ruby::RubyExtension::method_missing try to call function with name '%1' in object '%2'.").arg(funcname).arg(object->getName()) );
                #endif
                result = object->call(funcname, Api::List::Ptr(new Api::List(argsList)));
            }
        } catch(Kross::Exception::Ptr exception)
        {
            #ifdef KROSS_RUBY_EXTENSION_DEBUG
                krossdebug("c++ exception catched, raise a ruby error");
            #endif
            throw convertFromException(exception);
        }  catch(...)
        {
            Kross::Exception::Ptr e = Kross::Exception::Ptr( new Kross::Exception( "Unknow error" ) );
            throw convertFromException(e); // TODO: fix //i18n
        }
    } catch(VALUE v) {
         rb_exc_raise(v );
    }
    return toVALUE(result);
#endif
    return Qfalse;
}

void RubyExtension::delete_object(void* object)
{
    krossdebug("RubyExtension::delete_object");
    RubyExtension* extension = static_cast< RubyExtension* >(object);
    delete extension;
    extension = 0;
}

void RubyExtension::delete_exception(void* object)
{
#if 0
    Kross::Exception* exc = static_cast<Kross::Exception*>(object);
    exc->_KShared_unref();
#endif
}

int RubyExtension::convertHash_i(VALUE key, VALUE value, VALUE  vmap)
{
    krossdebug("RubyExtension::convertHash_i");
    QVariantMap* map; 
    Data_Get_Struct(vmap, QVariantMap, map);
    if (key != Qundef)
        map->insert(STR2CSTR(key), RubyExtension::toVariant( value ));
    return ST_CONTINUE;
}

#if 0
bool RubyExtension::isOfExceptionType(VALUE value)
{
    VALUE result = rb_funcall(value, rb_intern("kind_of?"), 1, RubyExtensionPrivate::s_krossException );
    return (TYPE(result) == T_TRUE);
}

bool RubyExtension::isOfObjectType(VALUE value)
{
    VALUE result = rb_funcall(value, rb_intern("kind_of?"), 1, RubyExtensionPrivate::s_krossObject );
    return (TYPE(result) == T_TRUE);
}

Kross::Exception* RubyExtension::convertToException(VALUE value)
{
    if( isOfExceptionType(value) )
    {
        Kross::Exception* exception;
        Data_Get_Struct(value, Kross::Exception, exception);
        return exception;
    }
    return 0;
}

VALUE RubyExtension::convertFromException(Kross::Exception::Ptr exc)
{
    if(RubyExtensionPrivate::s_krossException == 0)
    {
        RubyExtensionPrivate::s_krossException = rb_define_class("KrossException", rb_eRuntimeError);
    }
    //exc->_KShared_ref(); //TODO
    return Data_Wrap_Struct(RubyExtensionPrivate::s_krossException, 0, RubyExtension::delete_exception, exc.data() );
}
#endif

QVariant RubyExtension::toVariant(VALUE value)
{
    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug(QString("RubyExtension::toVariant of type %1").arg(TYPE(value)));
    #endif

    switch( TYPE( value ) )
    {
        case T_DATA:
        {
            #ifdef KROSS_RUBY_EXTENSION_DEBUG
                krossdebug("Object is a Kross Object");
            #endif
#if 0
            if( isOfObjectType(value) )
            {
                RubyExtension* objectExtension;
                Data_Get_Struct(value, RubyExtension, objectExtension);
                return objectExtension->d->m_object.data();
            } else {
                krosswarning("Cannot yet convert standard ruby type to kross object");
                return 0;
            }
#endif
            return 0;
        }
        case T_FLOAT:
            return (double) NUM2DBL(value);
        case T_STRING:
            return QString(STR2CSTR(value));
        case T_ARRAY:
        {
            QVariantList l;
            for(int i = 0; i < RARRAY(value)->len; i++)
                l.append( toVariant( rb_ary_entry( value , i ) ) );
            return l;
        }
        case T_FIXNUM:
            return (qlonglong) FIX2INT(value);
        case T_HASH:
        {
            QVariantMap map;
            VALUE vmap = Data_Wrap_Struct(rb_cObject, 0,0, &map);
            rb_hash_foreach(value, (int (*)(...))convertHash_i, vmap);
            return map;
        }
        case T_BIGNUM:
        {
            return (qlonglong) NUM2LONG(value);
        }
        case T_TRUE:
        {
            return QVariant(bool(true));
        }
        case T_FALSE:
        {
            return QVariant(bool(false));
        }
        case T_SYMBOL:
        {
            return QString(rb_id2name(SYM2ID(value)));
        }
        case T_MATCH:
        case T_OBJECT:
        case T_FILE:
        case T_STRUCT:
        case T_REGEXP:
        case T_MODULE:
        case T_ICLASS:
        case T_CLASS:
            krosswarning(QString("This ruby type '%1' cannot be converted to a Kross::Object").arg(TYPE(value)));
        default:
        case T_NIL:
            return 0;
    }
}

VALUE RubyExtension::toVALUE(const QString& s)
{
    return s.isNull() ? rb_str_new2("") : rb_str_new2(s.toLatin1().data());
}

VALUE RubyExtension::toVALUE(QStringList list)
{
    VALUE l = rb_ary_new();
    foreach(QString s, list)
        rb_ary_push(l, toVALUE(s));
    return l;
}

VALUE RubyExtension::toVALUE(QVariantMap map)
{
    VALUE h = rb_hash_new();
    for(QMap<QString, QVariant>::Iterator it = map.begin(); it != map.end(); ++it)
        rb_hash_aset(h, toVALUE(it.key()), toVALUE(it.value()) );
    return h;

}

VALUE RubyExtension::toVALUE(QVariantList list)
{
    VALUE l = rb_ary_new();
    foreach(QVariant v, list)
        rb_ary_push(l, toVALUE(v));
    return l;
}

VALUE RubyExtension::toVALUE(const QVariant& variant)
{
    switch(variant.type()) {
        case QVariant::Invalid:
            return Qnil;
        case QVariant::Bool:
            return (variant.toBool()) ? Qtrue : Qfalse;
        case QVariant::Int:
            return INT2FIX(variant.toInt());
        case QVariant::UInt:
            return UINT2NUM(variant.toUInt());
        case QVariant::Double:
            return rb_float_new(variant.toDouble());
        case QVariant::Date:
        case QVariant::Time:
        case QVariant::DateTime:
        case QVariant::ByteArray:
        case QVariant::BitArray:
        //case QVariant::CString:
        case QVariant::String:
            return toVALUE(variant.toString());
        case QVariant::StringList:
            return toVALUE(variant.toStringList());
        case QVariant::Map:
            return toVALUE(variant.toMap());
        case QVariant::List:
            return toVALUE(variant.toList());

        // To handle following both cases is a bit difficult
        // cause Python doesn't spend an easy possibility
        // for such large numbers (TODO maybe BigInt?). So,
        // we risk overflows here, but well...
        case QVariant::LongLong: {
            return INT2NUM((long)variant.toLongLong());
        }
        case QVariant::ULongLong:
            return UINT2NUM((unsigned long)variant.toULongLong());
        default: {
            krosswarning( QString("Kross::Ruby::RubyExtension::toVALUE(QVariant) Not possible to convert the QVariant type '%1' to a VALUE.").arg(variant.typeName()) );
            return Qundef;
        }
    }
}

VALUE RubyExtension::toVALUE(RubyExtension* extension)
{
    QObject* object = extension->d->m_object;

    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug( QString("RubyExtension::toVALUE RubyExtension.QObject=%1").arg( object ? QString("%1 %2").arg(object->objectName()).arg(object->metaObject()->className()) : "NULL" ) );
    #endif

    if(! object) {
        return 0;
    }

    if(RubyExtensionPrivate::s_krossObject == 0) {
        RubyExtensionPrivate::s_krossObject = rb_define_class("KrossObject", rb_cObject);
        rb_define_method(RubyExtensionPrivate::s_krossObject, "method_missing",  (VALUE (*)(...))RubyExtension::method_missing, -1);
    }

    return Data_Wrap_Struct(RubyExtensionPrivate::s_krossObject, 0, RubyExtension::delete_object, extension);
    //return Data_Wrap_Struct(RubyExtensionPrivate::s_krossObject, 0, RubyExtension::delete_object, new RubyExtension(extension));
}

#if 0
VALUE RubyExtension::toVALUE(QObject* object)
{
    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug( QString("RubyExtension::toVALUE QObject=%1").arg( object ? QString("%1 %2").arg(object->objectName()).arg(object->metaObject()->className()) : "NULL" ) );
    #endif

    if(! object) {
        return 0;
    }

    if(RubyExtensionPrivate::s_krossObject == 0) {
        RubyExtensionPrivate::s_krossObject = rb_define_class("KrossObject", rb_cObject);
        rb_define_method(RubyExtensionPrivate::s_krossObject, "method_missing",  (VALUE (*)(...))RubyExtension::method_missing, -1);
    }

    return Data_Wrap_Struct(RubyExtensionPrivate::s_krossObject, 0, RubyExtension::delete_object, new RubyExtension(object) );

#if 0
    {
        Kross::Variant* variant = dynamic_cast<Kross::Variant*>( object.data() );
        if(variant)
            return toVALUE( variant->getValue() );
    }
    {
        Kross::List* list = dynamic_cast<Kross::List*>( object.data() );
        if(list)
            return toVALUE( Kross::List::Ptr(list) );
    }
    {
        Kross::Dict* dict = dynamic_cast<Kross::Dict*>( object.data() );
        if(dict)
            return toVALUE( Kross::Dict::Ptr(dict) );
    }
    if(RubyExtensionPrivate::s_krossObject == 0)
    {
        RubyExtensionPrivate::s_krossObject = rb_define_class("KrossObject", rb_cObject);
        rb_define_method(RubyExtensionPrivate::s_krossObject, "method_missing",  (VALUE (*)(...))RubyExtension::method_missing, -1);
    }
    return Data_Wrap_Struct(RubyExtensionPrivate::s_krossObject, 0, RubyExtension::delete_object, new RubyExtension(object) );
#endif
}
#endif

#if 0
VALUE RubyExtension::toVALUE(Kross::List::Ptr list)
{
    VALUE l = rb_ary_new();
    uint count = list ? list->count() : 0;
    for(uint i = 0; i < count; i++)
        rb_ary_push(l, toVALUE(list->item(i)));
    return l;

}
#endif
