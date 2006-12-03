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

#include "rubycallcache.h"
#include "rubyvariant.h"
#include <kross/core/metatype.h>

//#include <st.h>

#include <QMap>
#include <QString>
#include <QPointer>
#include <QMetaObject>
#include <QMetaMethod>
#include <QHash>
#include <QVarLengthArray>

using namespace Kross;

namespace Kross {

    /// @internal d-pointer class.
    class RubyExtensionPrivate {
        friend class RubyExtension;

        /// The wrapped QObject.
        QPointer<QObject> m_object;

        /// The wrapped krossobject VALUE type.
        static VALUE s_krossObject;
        //static VALUE s_krossException;

        /// The cached list of methods.
        QHash<QByteArray, int> m_methods;
        /// The cached list of properties.
        QHash<QByteArray, int> m_properties;
        /// The cached list of enumerations.
        QHash<QByteArray, int> m_enumerations;
    };

}

VALUE RubyExtensionPrivate::s_krossObject = 0;
//VALUE RubyExtensionPrivate::s_krossException = 0;

RubyExtension::RubyExtension(QObject* object)
    : d(new RubyExtensionPrivate())
{
    d->m_object = object;

    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug(QString("RubyExtension Ctor QObject=%1").arg( object ? QString("%1 %2").arg(object->objectName()).arg(object->metaObject()->className()) : "NULL" ));
    #endif

    if(d->m_object) {
        const QMetaObject* metaobject = d->m_object->metaObject();

        { // initialize methods.
            const int count = metaobject->methodCount();
            for(int i = 0; i < count; ++i) {
                QMetaMethod member = metaobject->method(i);
                const QString signature = member.signature();
                const QByteArray name = signature.left(signature.indexOf('(')).toLatin1();
                if(! d->m_methods.contains(name))
                    d->m_methods.insert(name, i);
            }
        }

        { // initialize properties
            const int count = metaobject->propertyCount();
            for(int i = 0; i < count; ++i) {
                QMetaProperty prop = metaobject->property(i);
                d->m_properties.insert(prop.name(), i);
                if(prop.isWritable())
                    d->m_properties.insert(QByteArray(prop.name()).append('='), i);
            }
        }

        { // initialize enumerations
            const int count = metaobject->enumeratorCount();
            for(int i = 0; i < count; ++i) {
                QMetaEnum e = metaobject->enumerator(i);
                const int kc = e.keyCount();
                for(int k = 0; k < kc; ++k) {
                    const QByteArray name = /*e.name() +*/ e.key(k);
                    d->m_enumerations.insert(name, e.value(k));
                }
            }
        }
    }
}

RubyExtension::~RubyExtension()
{
    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug("RubyExtension Dtor");
    #endif
    delete d;
}

QObject* RubyExtension::object() const
{
    return d->m_object;
}

VALUE RubyExtension::method_missing(int argc, VALUE *argv, VALUE self)
{
    if( argc < 1 )
        return 0;

    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug("RubyExtension::method_missing Converting self to RubyExtension");
    #endif

    RubyExtension* extension;
    Data_Get_Struct(self, RubyExtension, extension);
    Q_ASSERT(extension);
    return RubyExtension::call_method_missing(extension, argc, argv, self);
}

VALUE RubyExtension::callMetaMethod(const QByteArray& funcname, int argc, VALUE *argv, VALUE self)
{
    const int argumentcount = argc - 1;

    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug(QString("RubyExtension::callMetaMethod method=%1 argumentcount=%2").arg(funcname.constData()).arg(argumentcount));
        for(int i = 1; i < argc; i++) {
            QVariant v = RubyType<QVariant>::toVariant(argv[i]);
            krossdebug(QString("  argument #%1: variant.toString=%2 variant.typeName=%3").arg(i).arg(v.toString()).arg(v.typeName()));
        }
    #endif

    int methodindex = d->m_methods[funcname];
    if(methodindex < 0) {
        krosswarning(QString("No such function '%1'").arg(funcname.constData()));
        return Qfalse;
    }

    QObject* object = d->m_object;
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
        krossdebug( QString("QMetaMethod idx=%1 sig=%2 tag=%3 type=%4").arg(methodindex).arg(metamethod.signature()).arg(metamethod.tag()).arg(metamethod.typeName()) );
    #endif
    
    QList<QByteArray> typelist = metamethod.parameterTypes();
    const int typelistcount = typelist.count();
    bool hasreturnvalue = strcmp(metamethod.typeName(),"") != 0;

    // exact 1 returnvalue + 0..9 arguments
    Q_ASSERT(typelistcount <= 10);
    QVarLengthArray<MetaType*> variantargs( typelistcount + 1 );
    QVarLengthArray<void*> voidstarargs( typelistcount + 1 );
    QVarLengthArray<int> varianttypes( typelistcount + 1 );
    
    // set the return type
    int returnTypeId;
    int returnMetaTypeId;
    if(hasreturnvalue) {
        returnTypeId = QVariant::nameToType( metamethod.typeName() );
        if(returnTypeId != QVariant::Invalid) {
            if(returnTypeId == QVariant::UserType)
            {
                returnMetaTypeId = QMetaType::type( metamethod.typeName() );
            }
            #ifdef KROSS_RUBY_EXTENSION_DEBUG
                krossdebug( QString("RubyExtension::callMetaMethod typeName=%1 variant.typeid=%2").arg(metamethod.typeName()).arg(returnTypeId) );
            #endif
        }
        else {
            returnMetaTypeId = QMetaType::type( metamethod.typeName() );
            if(returnMetaTypeId == QMetaType::Void) {
                #ifdef KROSS_RUBY_EXTENSION_DEBUG
                    krossdebug( QString("RubyExtension::callMetaMethod typeName=%1 metatype.typeid is QMetaType::Void").arg(metamethod.typeName()) );
                #endif
            }
            else {
                #ifdef KROSS_RUBY_EXTENSION_DEBUG
                    krossdebug( QString("RubyExtension::callMetaMethod typeName=%1 metatype.typeid=%2").arg(metamethod.typeName()).arg(returnMetaTypeId) );
                #endif
            }
        }
    }
    // set the arguments types
    int idx = 1;
    for(; idx <= typelistcount; ++idx) {
        varianttypes[idx ] = QVariant::nameToType(typelist[idx - 1].constData());
    }

    // Create a cache of the function call
    RubyCallCache* callobj = new RubyCallCache(object, methodindex, hasreturnvalue, returnTypeId, returnMetaTypeId, varianttypes);
    QByteArray varcallcache = QByteArray("@callcache") + funcname;
    rb_iv_set(self, varcallcache , callobj->toValue() );
    rb_define_variable("$krossinternallastclass", &self);
    rb_eval_string("def $krossinternallastclass." + funcname + "(*args)\n "+ varcallcache +".cacheexec(args) \nend");
    return callobj->execfunction(argc, argv);
}

VALUE RubyExtension::call_method_missing(RubyExtension* extension, int argc, VALUE *argv, VALUE self)
{
    QByteArray name = rb_id2name(SYM2ID(argv[0]));

    // look if the name is a method
    if( extension->d->m_methods.contains(name) ) {
        return extension->callMetaMethod(name, argc, argv, self);
    }

    // look if the name is a property
    if( extension->d->m_properties.contains(name) /* && extension->d->m_object */ ) {
        const QMetaObject* metaobject = extension->d->m_object->metaObject();
        QMetaProperty property = metaobject->property( extension->d->m_properties[name] );
        if( name.endsWith('=') ) { // setter
            if(argc < 2) {
                rb_raise(rb_eNameError, QString("Expected value-argument for \"%1\" setter.").arg(name.constData()).toLatin1().constData());
                return Qnil;
            }
            QVariant v = RubyType<QVariant>::toVariant(argv[1]);
            if(! property.write(extension->d->m_object, v)) {
                rb_raise(rb_eNameError, QString("Setting attribute \"%1\" failed.").arg(name.constData()).toLatin1().constData());
                return Qnil;
            }
            return Qnil;
        }
        else { // getter
            if(! property.isReadable()) {
                rb_raise(rb_eNameError, QString("Attribute \"%1\" is not readable.").arg(name.constData()).toLatin1().constData());
                return Qnil;
            }
            return RubyType<QVariant>::toVALUE( property.read(extension->d->m_object) );
        }
    }

    // look if the name is an enumeration
    if( extension->d->m_enumerations.contains(name) ) {
        return RubyType<int>::toVALUE( extension->d->m_enumerations[name] );
    }

    rb_raise(rb_eNameError, QString("No such method or variable \"%1\".").arg(name.constData()).toLatin1().constData());
    return Qnil;
}

void RubyExtension::delete_object(void* object)
{
    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug("RubyExtension::delete_object");
    #endif
    RubyExtension* extension = static_cast< RubyExtension* >(object);
    delete extension;
    extension = 0;
}

#if 0
void RubyExtension::delete_exception(void* object)
{
    Kross::Exception* exc = static_cast<Kross::Exception*>(object);
    exc->_KShared_unref();
}
#endif

bool RubyExtension::isRubyExtension(VALUE value)
{
    VALUE result = rb_funcall(value, rb_intern("kind_of?"), 1, RubyExtensionPrivate::s_krossObject );
    return (TYPE(result) == T_TRUE);
}

#if 0
bool RubyExtension::isOfExceptionType(VALUE value)
{
    VALUE result = rb_funcall(value, rb_intern("kind_of?"), 1, RubyExtensionPrivate::s_krossException );
    return (TYPE(result) == T_TRUE);
}
Kross::Exception* RubyExtension::convertToException(VALUE value)
{
    if( isOfExceptionType(value) ) {
        Kross::Exception* exception;
        Data_Get_Struct(value, Kross::Exception, exception);
        return exception;
    }
    return 0;
}
VALUE RubyExtension::convertFromException(Kross::Exception::Ptr exc)
{
    if(RubyExtensionPrivate::s_krossException == 0)
        RubyExtensionPrivate::s_krossException = rb_define_class("KrossException", rb_eRuntimeError);
    //exc->_KShared_ref(); //TODO
    return Data_Wrap_Struct(RubyExtensionPrivate::s_krossException, 0, RubyExtension::delete_exception, exc.data() );
}
#endif

VALUE RubyExtension::toVALUE(RubyExtension* extension)
{
    QObject* object = extension->d->m_object;

    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug( QString("RubyExtension::toVALUE RubyExtension.QObject=%1").arg( object ? QString("%1 %2").arg(object->objectName()).arg(object->metaObject()->className()) : "NULL" ) );
    #endif

    if( ! object )
        return 0;

    if( RubyExtensionPrivate::s_krossObject == 0 ) {
        RubyExtensionPrivate::s_krossObject = rb_define_class("KrossObject", rb_cObject);
        rb_define_method(RubyExtensionPrivate::s_krossObject, "method_missing",  (VALUE (*)(...))RubyExtension::method_missing, -1);
    }

    return Data_Wrap_Struct(RubyExtensionPrivate::s_krossObject, 0, RubyExtension::delete_object, extension);
}

