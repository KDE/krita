/***************************************************************************
 * pythonextension.cpp
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

#include "pythonextension.h"
#include "pythoninterpreter.h"
#include "pythonvariant.h"

#include <QWidget>
#include <QMetaMethod>
#include <QSignalSpy>
#include <QVarLengthArray>

using namespace Kross;

namespace Kross {

    /// \internal d-pointer class.
    class PythonExtension::Private
    {
        public:
            /// The QObject this PythonExtension wraps.
            QPointer<QObject> object;

            #ifdef KROSS_PYTHON_EXTENSION_CTORDTOR_DEBUG
                /// \internal string for debugging.
                QString debuginfo;
            #endif

            /// The cached list of methods.
            QHash<QByteArray, Py::Object> methods;
            /// The cached list of properties.
            QHash<QByteArray, QMetaProperty> properties;
            /// The cached list of enumerations.
            QHash<QByteArray, int> enumerations;

            /// The cached list of methodnames.
            Py::List methodnames;
            /// The cached list of membernames.
            Py::List membernames;

            /// The proxymethod which will handle all calls to our \a PythonExtension instance.
            Py::MethodDefExt<PythonExtension>* proxymethod;

    };

}

PythonExtension::PythonExtension(QObject* object)
    : Py::PythonExtension<PythonExtension>()
    , d( new Private() )
{
    d->object = object;

    #ifdef KROSS_PYTHON_EXTENSION_CTORDTOR_DEBUG
        d->debuginfo = object ? QString("%1 (%2)").arg(object->objectName()).arg(object->metaObject()->className()) : "NULL";
        krossdebug( QString("PythonExtension::Constructor object=%1").arg(d->debuginfo) );
    #endif

    behaviors().name("KrossPythonExtension");
    behaviors().doc("The KrossPythonExtension object wraps a QObject into the world of python.");
    //behaviors().supportRepr();
    behaviors().supportGetattr();
    behaviors().supportSetattr();
    behaviors().supportSequenceType();
    behaviors().supportMappingType();

    add_varargs_method("__name__", &PythonExtension::getObjectName, "Return the objectName.");
    add_varargs_method("__class__", &PythonExtension::getClassName, "Return the className.");
    //add_varargs_method("__classinfos__", &PythonExtension::getClassInfos, "Return a list of key,value-tuples of class informations.");
    add_varargs_method("__signals__", &PythonExtension::getSignalNames, "Return list of signal names.");
    add_varargs_method("__slots__", &PythonExtension::getSlotNames, "Return list of slot names.");
    add_varargs_method("__properties__", &PythonExtension::getPropertyNames, "Return list of property names.");
    //add_varargs_method("__toPointer__", &PythonExtension::toPointer, "Return the void* pointer of the QObject.");
    //add_varargs_method("__fromPointer__", &PythonExtension::fromPointer, "Set the QObject* to the passed void* pointer.");
    //add_varargs_method("__connect__", &PythonExtension::doConnect, "Connect signal with python function.");

    d->proxymethod = new Py::MethodDefExt<PythonExtension>(
        "", // methodname, not needed cause we use the method only internaly.
        0, // method that should handle the callback, not needed cause proxyhandler will handle it.
        Py::method_varargs_call_handler_t( proxyhandler ), // callback handler
        "" // documentation
    );

    if(d->object) {
        const QMetaObject* metaobject = d->object->metaObject();

        { // initialize methods.
            const int count = metaobject->methodCount();
            for(int i = 0; i < count; ++i) {
                QMetaMethod member = metaobject->method(i);
                const QString signature = member.signature();
                const QByteArray name = signature.left(signature.indexOf('(')).toLatin1();
                if(! d->methods.contains(name)) {
                    Py::Tuple self(3);
                    self[0] = Py::Object(this); // reference to this instance
                    self[1] = Py::Int(i); // the first index used for faster access
                    self[2] = Py::String(name); // the name of the method

                    d->methods.insert(name, Py::Object(PyCFunction_New( &d->proxymethod->ext_meth_def, self.ptr() ), true));
                    d->methodnames.append(self[2]);
                }
            }
        }

        { // initialize properties
            const int count = metaobject->propertyCount();
            for(int i = 0; i < count; ++i) {
                QMetaProperty prop = metaobject->property(i);
                d->properties.insert(prop.name(), prop);
                d->membernames.append( Py::String(prop.name()) );
            }
        }

        { // initialize enumerations
            const int count = metaobject->enumeratorCount();
            for(int i = 0; i < count; ++i) {
                QMetaEnum e = metaobject->enumerator(i);
                const int kc = e.keyCount();
                for(int k = 0; k < kc; ++k) {
                    const QByteArray name = /*e.name() +*/ e.key(k);
                    d->enumerations.insert(name, e.value(k));
                    d->membernames.append( Py::String(name) );
                }
            }
        }
    }
}

PythonExtension::~PythonExtension()
{
    #ifdef KROSS_PYTHON_EXTENSION_CTORDTOR_DEBUG
        krossdebug( QString("PythonExtension::Destructor object=%1").arg(d->debuginfo) );
    #endif
    delete d->proxymethod;
    delete d;
}

QObject* PythonExtension::object() const
{
    return d->object;
}

Py::Object PythonExtension::getattr(const char* n)
{
    #ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
        krossdebug( QString("PythonExtension::getattr name='%1'").arg(n) );
    #endif

    // handle internal methods
    if(n[0] == '_') {
        if(strcmp(n,"__methods__") == 0)
            return d->methodnames;
        if(strcmp(n,"__members__") == 0)
            return d->membernames;
        //if(strcmp(n,"__dict__") == 0)
        //    return PythonType<QStringList>::toPyObject( QStringList() );
    }

    // look if the attribute is a method
    if(d->methods.contains(n)) {
        #ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
            krossdebug( QString("PythonExtension::getattr name='%1' is a method.").arg(n) );
        #endif
        return d->methods[n];
    }

    // look if the attribute is a property
    if(d->properties.contains(n) && d->object) {
        QMetaProperty property = d->properties[n];

        #ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
            krossdebug( QString("PythonExtension::getattr name='%1' is a property: type=%2 valid=%3 readable=%4 scriptable=%5 writable=%6 usertype=%7")
                        .arg(n).arg(property.typeName()).arg(property.isValid())
                        .arg(property.isReadable()).arg(property.isScriptable(d->object)).arg(property.isWritable())
                        .arg(property.isUser(d->object)).arg(property.userType())
            );
        #endif

        if(! property.isReadable()) {
            Py::AttributeError( QString("Attribute \"%1\" is not readable.").arg(n).toLatin1().constData() );
            return Py::None();
        }

        return PythonType<QVariant>::toPyObject( property.read(d->object) );
    }

    // look if the attribute is an enumerator
    if(d->enumerations.contains(n)) {
        return Py::Int(d->enumerations[n]);
    }

    // finally redirect the unhandled attribute-request...
    //return Py::PythonExtension<PythonExtension>::getattr_methods(n);
    return Py::PythonExtension<PythonExtension>::getattr(n);
}

int PythonExtension::setattr(const char* n, const Py::Object& value)
{
    // look if the attribute is a property
    if(d->properties.contains(n) && d->object) {
        QMetaProperty property = d->properties[n];

        #ifdef KROSS_PYTHON_EXTENSION_SETATTR_DEBUG
            krossdebug( QString("PythonExtension::setattr name='%1' is a property: type=%2 valid=%3 readable=%4 scriptable=%5 writable=%6 usertype=%7")
                .arg(n).arg(property.typeName()).arg(property.isValid())
                .arg(property.isReadable()).arg(property.isScriptable(d->object)).arg(property.isWritable())
                .arg(property.isUser(d->object)).arg(property.userType())
            );
        #endif

        if(! property.isWritable()) {
            Py::AttributeError( QString("Attribute \"%1\" is not writable.").arg(n).toLatin1().constData() );
            return -1; // indicate error
        }

        QVariant v = PythonType<QVariant>::toVariant(value);
        if(! property.write(d->object, v)) {
            Py::AttributeError( QString("Setting attribute \"%1\" failed.").arg(n).toLatin1().constData() );
            return -1; // indicate error
        }
        #ifdef KROSS_PYTHON_EXTENSION_SETATTR_DEBUG
            krossdebug( QString("PythonExtension::setattr name='%1' value='%2'").arg(n).arg(v.toString()) );
        #endif
        return 0; // indicate success
    }

    // finally redirect the unhandled attribute-request...
    return Py::PythonExtension<PythonExtension>::setattr(n, value);
}

Py::Object PythonExtension::getObjectName(const Py::Tuple&)
{
    return PythonType<QString>::toPyObject( d->object->objectName() );
}

Py::Object PythonExtension::getClassName(const Py::Tuple&)
{
    return PythonType<QString>::toPyObject( d->object->metaObject()->className() );
}

Py::Object PythonExtension::getSignalNames(const Py::Tuple&)
{
    Py::List list;
    const QMetaObject* metaobject = d->object->metaObject();
    const int count = metaobject->methodCount();
    for(int i = 0; i < count; ++i) {
        QMetaMethod m = metaobject->method(i);
        if( m.methodType() == QMetaMethod::Signal)
            list.append( Py::String(m.signature()) );
    }
    return list;
}

Py::Object PythonExtension::getSlotNames(const Py::Tuple&)
{
    Py::List list;
    const QMetaObject* metaobject = d->object->metaObject();
    const int count = metaobject->methodCount();
    for(int i = 0; i < count; ++i) {
        QMetaMethod m = metaobject->method(i);
        if( m.methodType() == QMetaMethod::Slot)
            list.append( Py::String(m.signature()) );
    }
    return list;
}

Py::Object PythonExtension::getPropertyNames(const Py::Tuple&)
{
    Py::List list;
    const QMetaObject* metaobject = d->object->metaObject();
    const int count = metaobject->propertyCount();
    for(int i = 0; i < count; ++i)
        list.append( Py::String(metaobject->property(i).name()) );
    return list;
}

/*
Py::Object PythonExtension::toPointer(const Py::Tuple&)
{
    PyObject* qobjectptr = PyLong_FromVoidPtr( (void*) d->object.data() );
    //PyObject* o = Py_BuildValue ("N", mw);
    return Py::asObject( qobjectptr );
    //PythonPyQtExtension* pyqtextension = new PythonPyQtExtension(self, args);
    //return pyqtextension;
}

Py::Object PythonExtension::toPointer(fromPointer(const Py::Tuple&)
{
    QObject* object = dynamic_cast< QObject* >(PyLong_AsVoidPtr( args[0] ));
}
*/

#if 0
Py::Object PythonExtension::doConnect(const Py::Tuple& args)
{
    if( args.size() < 2 ) {
        Py::AttributeError("The connect-method expects at least 2 arguments.");
        return PythonType<bool>::toPyObject(false);
    }

    QByteArray signalname = PythonType<QByteArray>::toVariant( args[0] );
    krossdebug( QString("====================================> signalname=%1 typeName=%2").arg(signalname).arg(args[1].type().as_string().c_str()) );

    if( args[1].isCallable() ) {
        Py::Callable funcobject(args[1]);

QVariantList args = d->signalspy.takeFirst();
Py::Object result = d->callable.apply( PythonType<QVariantList,Py::Tuple>::toPyObject(args) );

//connect(d->object, "2" + signalname, , SIGNAL(handleEmitted));
//QSignalSpy spy(d->object, signalname);
/*
COMPARE(spy.count(), 1); // make sure the signal was emitted exactly one time
QVariantList arguments = spy.takeFirst(); // take the first signal
VERIFY(arguments.at(0).toBool() == true); // verify the first argument
*/

//TODO

    }
    else {
        Py::AttributeError("The connect-method expects as second argument something that is callable (e.g. a function).");
        return PythonType<bool>::toPyObject(false);
    }

    return PythonType<bool>::toPyObject(true);
}
#endif

PyObject* PythonExtension::proxyhandler(PyObject *_self_and_name_tuple, PyObject *args)
{
    try {
        Py::Tuple selftuple(_self_and_name_tuple);
        PythonExtension *self = static_cast<PythonExtension*>( selftuple[0].ptr() );

        int methodindex = Py::Int(selftuple[1]);

        QByteArray ba = Py::String(selftuple[2]).as_string().c_str();
        const char* methodname = ba.constData();

        #ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
            krossdebug( QString("PythonExtension::proxyhandler methodname=%1 methodindex=%2").arg(methodname).arg(methodindex) );
        #endif

        Py::Tuple argstuple(args);
        const int argssize = int( argstuple.size() );
        QMetaMethod metamethod = self->d->object->metaObject()->method( methodindex );
        if(metamethod.parameterTypes().size() != argssize) {
            bool found = false;
            const int count = self->d->object->metaObject()->methodCount();
            for(++methodindex; methodindex < count; ++methodindex) {
                metamethod = self->d->object->metaObject()->method( methodindex );
                const QString signature = metamethod.signature();
                const QByteArray name = signature.left(signature.indexOf('(')).toLatin1();
                if(name == methodname) {
                    if(metamethod.parameterTypes().size() == argssize) {
                        found = true;
                        break;
                    }
                }
            }
            if(! found) {
                krosswarning( QString("PythonExtension::proxyhandler The method '%1' does not expect %2 arguments.").arg(methodname).arg(argssize) );
                throw Py::TypeError( QString("Invalid number of arguments for the method %1").arg(methodname).toLatin1().constData() );
            }
        }

        #ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
            krossdebug( QString("PythonExtension::proxyhandler QMetaMethod idx=%1 sig=%2 tag=%3 type=%4").arg(methodindex).arg(metamethod.signature()).arg(metamethod.tag()).arg(metamethod.typeName()) );
            for(int i = 0; i < argssize; ++i) {
                QVariant v = PythonType<QVariant>::toVariant( argstuple[i] );
                krossdebug( QString("  Argument index=%1 variant.toString=%2 variant.typeName=%3").arg(i).arg(v.toString()).arg(v.typeName()) );
            }
        #endif

        Py::Object pyresult;
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
                    #ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
                        krossdebug( QString("PythonExtension::proxyhandler typeName=%1 variant.typeid=%2").arg(metamethod.typeName()).arg(typeId) );
                    #endif
                    returntype = new MetaTypeVariant< QVariant >( QVariant( (QVariant::Type) typeId ) );
                }
                else {
                    typeId = QMetaType::type( metamethod.typeName() );
                    if(typeId == QMetaType::Void) {
                        #ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
                            krossdebug( QString("PythonExtension::proxyhandler typeName=%1 metatype.typeid is QMetaType::Void").arg(metamethod.typeName()) );
                        #endif
                        returntype = new MetaTypeVariant< QVariant >( QVariant() );
                    }
                    else {
                        #ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
                            krossdebug( QString("PythonExtension::proxyhandler typeName=%1 metatype.typeid=%2").arg(metamethod.typeName()).arg(typeId) );
                        #endif
                        //if (id != -1) {
                        void* myClassPtr = QMetaType::construct(typeId, 0);
                        //QMetaType::destroy(id, myClassPtr);
                        returntype = new MetaTypeVoidStar( typeId, myClassPtr );
                    }
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
            try {
                for(; idx <= typelistcount; ++idx) {
                    variantargs[idx] = PythonMetaTypeFactory::create(typelist[idx - 1].constData(), argstuple[idx - 1]);
                    voidstarargs[idx] = variantargs[idx]->toVoidStar();
                }
            }
            catch(Py::Exception& e) {
                // Seems PythonMetaTypeFactory::create raised an exception
                // up. Clean all already allocated MetaType instances.
                for(int i = 0; i < idx; ++i)
                    delete variantargs[i];
                throw e; // re-throw exception
            }

            // call the method now
            int r = self->d->object->qt_metacall(QMetaObject::InvokeMetaMethod, methodindex,
                    &voidstarargs[0]);
            #ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
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
                QVariant v(tp, variantargs[0]->toVoidStar());
                pyresult = PythonType<QVariant>::toPyObject(v);
                #ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
                    krossdebug( QString("Returnvalue id=%1 metamethod.typename=%2 variant.toString=%3 variant.typeName=%4 pyobject=%5").arg(tp).arg(metamethod.typeName()).arg(v.toString()).arg(v.typeName()).arg(pyresult.as_string().c_str()) );
                #endif
            }

            // finally free the PythonVariable instances
            for(int i = 0; i <= typelistcount; ++i)
                delete variantargs[i];
        }

        pyresult.increment_reference_count(); // don't destroy PyObject* if pyresult got destroyed.
        return pyresult.ptr();
    }
    catch(Py::Exception& e) {
        QStringList trace;
        int lineno;
        PythonInterpreter::extractException(trace, lineno);
        krosswarning( QString("PythonExtension::proxyhandler Had exception on line %1:\n%2 \n%3").arg(lineno).arg(Py::value(e).as_string().c_str()).arg(trace.join("\n")) );
    }

    return Py_None;
}

int PythonExtension::sequence_length()
{
    return d->object->children().count();
}

Py::Object PythonExtension::sequence_concat(const Py::Object& obj)
{
    throw Py::RuntimeError( QString("Unsupported: PythonExtension::sequence_concat %1").arg(obj.as_string().c_str()).toLatin1().constData() );
}

Py::Object PythonExtension::sequence_repeat(int index)
{
    throw Py::RuntimeError( QString("Unsupported: PythonExtension::sequence_repeat %1").arg(index).toLatin1().constData() );
}

Py::Object PythonExtension::sequence_item(int index)
{
    if(index < d->object->children().count())
        return Py::asObject(new PythonExtension( d->object->children().at(index) ));
    return Py::asObject( Py::new_reference_to( NULL ) );
}

Py::Object PythonExtension::sequence_slice(int from, int to)
{
    Py::List list;
    if(from >= 0) {
        const int count = d->object->children().count();
        for(int i = from; i <= to && i < count; ++i)
            list.append( Py::asObject(new PythonExtension( d->object->children().at(i) )) );
    }
    return list;
}

int PythonExtension::sequence_ass_item(int index, const Py::Object& obj)
{
    throw Py::RuntimeError( QString("Unsupported: PythonExtension::sequence_ass_item %1 %2").arg(index).arg(obj.as_string().c_str()).toLatin1().constData() );
}

int PythonExtension::sequence_ass_slice(int from, int to, const Py::Object& obj)
{
    throw Py::RuntimeError( QString("Unsupported: PythonExtension::sequence_ass_slice %1 %2 %3").arg(from).arg(to).arg(obj.as_string().c_str()).toLatin1().constData() );
}

int PythonExtension::mapping_length()
{
    return d->object->children().count();
}

Py::Object PythonExtension::mapping_subscript(const Py::Object& obj)
{
    QString name = Py::String(obj).as_string().c_str();
    QObject* object = d->object->findChild< QObject* >( name );
    if(! object) {
        foreach(QObject* o, d->object->children()) {
            if(name == o->metaObject()->className()) {
                object = o;
                break;
            }
        }
    }
    if(object)
        return Py::asObject(new PythonExtension(object));
    return Py::asObject( Py::new_reference_to( NULL ) );
}

int PythonExtension::mapping_ass_subscript(const Py::Object& obj1, const Py::Object& obj2)
{
    throw Py::RuntimeError( QString("Unsupported: PythonExtension::mapping_ass_subscript %1 %2").arg(obj1.as_string().c_str()).arg(obj2.as_string().c_str()).toLatin1().constData() );
}
