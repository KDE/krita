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
#include "rubyvariant.h"
#include "../core/metatype.h"

#include <st.h>

#include <QMap>
#include <QString>
#include <QPointer>
#include <QMetaObject>
#include <QMetaMethod>
#include <QHash>
#include <QVarLengthArray>

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

QObject* RubyExtension::object() const
{
    return d->m_object;
}

VALUE RubyExtension::method_missing(int argc, VALUE *argv, VALUE self)
{
    if(argc < 1) {
        return 0;
    }

    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug("RubyExtension::method_missing Converting self to RubyExtension");
    #endif

    RubyExtension* extension;
    Data_Get_Struct(self, RubyExtension, extension);
    Q_ASSERT(extension);
    return RubyExtension::call_method(extension, argc, argv);
}

VALUE RubyExtension::call_method(RubyExtension* extension, int argc, VALUE *argv)
{
    QByteArray funcname = rb_id2name(SYM2ID(argv[0]));
    const int argumentcount = argc - 1;

    #ifdef KROSS_RUBY_EXTENSION_DEBUG
        krossdebug(QString("RubyExtension::call_method method=%1 argumentcount=%2").arg(funcname.constData()).arg(argumentcount));
        for(int i = 1; i < argc; i++) {
            QVariant v = RubyType<QVariant>::toVariant(argv[i]);
            krossdebug(QString("  argument #%1: variant.toString=%2 variant.typeName=%3").arg(i).arg(v.toString()).arg(v.typeName()));
        }
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
        krossdebug( QString("QMetaMethod idx=%1 sig=%2 tag=%3 type=%4").arg(methodindex).arg(metamethod.signature()).arg(metamethod.tag()).arg(metamethod.typeName()) );
    #endif

    QVariant result;
    {
        QList<QByteArray> typelist = metamethod.parameterTypes();
        const int typelistcount = typelist.count();
        bool hasreturnvalue = strcmp(metamethod.typeName(),"") != 0;

        // exact 1 returnvalue + 0..9 arguments
        Q_ASSERT(typelistcount <= 10);
        QVarLengthArray<MetaType*> variantargs( typelistcount + 1 );
        QVarLengthArray<void*> voidstarargs( typelistcount + 1 );

        // set the return value
        if(hasreturnvalue) {
            MetaType* returntype;
            int typeId = QVariant::nameToType( metamethod.typeName() );
            if(typeId != QVariant::Invalid) {
                #ifdef KROSS_RUBY_EXTENSION_DEBUG
                    krossdebug( QString("RubyExtension::call_method typeName=%1 variant.typeid=%2").arg(metamethod.typeName()).arg(typeId) );
                #endif
                returntype = new MetaTypeVariant< QVariant >( QVariant( (QVariant::Type) typeId ) );
            }
            else {
                // crashes on shared containers like e.g. QStringList and QList which are handled above already
                typeId = QMetaType::type( metamethod.typeName() );
                //Q_ASSERT(typeId != QMetaType::Void);
                #ifdef KROSS_RUBY_EXTENSION_DEBUG
                    krossdebug( QString("RubyExtension::call_method typeName=%1 metatype.typeid=%2").arg(metamethod.typeName()).arg(typeId) );
                #endif
                //if (id != -1) {
                void* myClassPtr = QMetaType::construct(typeId, 0);
                //QMetaType::destroy(id, myClassPtr);
                returntype = new MetaTypeVoidStar( typeId, myClassPtr );
            }

            variantargs[0] = returntype;
            voidstarargs[0] = returntype->toVoidStar();
        }
        else {
            variantargs[0] = 0;
            voidstarargs[0] = (void*)0;
        }

        // set the arguments
        int idx = 1;
        for(; idx <= typelistcount; ++idx) {
            MetaType* metatype = RubyMetaTypeFactory::create(typelist[idx - 1].constData(), argv[idx]);
            if(! metatype) {
                // Seems RubyMetaTypeFactory::create returned an invalid RubyType.
                krosswarning( QString("RubyExtension::call_method Aborting cause RubyMetaTypeFactory::create returned NULL.") );
                for(int i = 0; i < idx; ++i) // Clear already allocated instances.
                    delete variantargs[i];
                return Qfalse; // abort execution.
            }
            variantargs[idx] = metatype;
            voidstarargs[idx] = metatype->toVoidStar();
        }

        // call the method now
        int r = object->qt_metacall(QMetaObject::InvokeMetaMethod, methodindex, &voidstarargs[0]);
        #ifdef KROSS_RUBY_EXTENSION_DEBUG
            krossdebug( QString("RESULT nr=%1").arg(r) );
        #else
            Q_UNUSED(r);
        #endif

        // eval the return-value
        if(hasreturnvalue) {
            int tp = QVariant::nameToType( metamethod.typeName() );
            if(tp == QVariant::UserType /*|| tp == QVariant::Invalid*/) {
                tp = QMetaType::type( metamethod.typeName() );
                //QObject* obj = (*reinterpret_cast< QObject*(*)>( variantargs[0]->toVoidStar() ));
            }
            result = QVariant(tp, variantargs[0]->toVoidStar());
            #ifdef KROSS_RUBY_EXTENSION_DEBUG
                krossdebug( QString("Returnvalue id=%1 metamethod.typename=%2 variant.toString=%3 variant.typeName=%4").arg(tp).arg(metamethod.typeName()).arg(result.toString()).arg(result.typeName()) );
            #endif
        }

        // finally free the PythonVariable instances
        for(int i = 0; i <= typelistcount; ++i)
            delete variantargs[i];
    }

    return result.isNull() ? 0 : RubyType<QVariant>::toVALUE(result);

#if 0
    QList<Api::Object::Ptr> argsList;
    for(int i = 1; i < argc; i++) {
        QObject* obj = toObject(argv[i]);
        if(obj) argsList.append(obj);
    }
    QObject* result;
    try { // We need a double try/catch because, the cleaning is only done at the end of the catch, so if we had only one try/catch, kross would crash after the call to rb_exc_raise
        try { // We can't let a C++ exceptions propagate in the C mechanism
            Kross::Callable* callable = dynamic_cast<Kross::Callable*>(object.data());
            if(callable && callable->hasChild(funcname)) {
                #ifdef KROSS_RUBY_EXTENSION_DEBUG
                    krossdebug( QString("RubyExtension::method_missing name='%1' is a child object of '%2'.").arg(funcname).arg(object->getName()) );
                #endif
                result = callable->getChild(funcname)->call(QString::null, KSharedPtr<Kross::List>(new Api::List(argsList)));
            }
            else {
                #ifdef KROSS_RUBY_EXTENSION_DEBUG
                    krossdebug( QString("RubyExtension::method_missing try to call function with name '%1' in object '%2'.").arg(funcname).arg(object->getName()) );
                #endif
                result = object->call(funcname, Api::List::Ptr(new Api::List(argsList)));
            }
        } catch(Kross::Exception::Ptr exception) {
            #ifdef KROSS_RUBY_EXTENSION_DEBUG
                krossdebug("c++ exception catched, raise a ruby error");
            #endif
            throw convertFromException(exception);
        }  catch(...) {
            Kross::Exception::Ptr e = Kross::Exception::Ptr( new Kross::Exception( "Unknow error" ) );
            throw convertFromException(e); // TODO: fix //i18n
        }
    } catch(VALUE v) {
         rb_exc_raise(v );
    }
    return toVALUE(result);
    return Qfalse;
#endif
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
}
