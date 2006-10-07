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
#include "pythonvariant.h"

#include <QWidget>
#include <QMetaMethod>
#include <qvarlengtharray.h>

using namespace Kross;

PythonExtension::PythonExtension(QObject* object)
    : Py::PythonExtension<PythonExtension>()
    , m_object(object)
    , m_debuginfo(object ? QString("%1(%2)").arg(object->objectName()).arg(object->metaObject()->className()) : "NULL")
    , m_methods(0)
{
    #ifdef KROSS_PYTHON_EXTENSION_CTOR_DEBUG
        krossdebug( QString("PythonExtension::Constructor object=%1").arg(m_debuginfo) );
    #endif

    behaviors().name("KrossPythonExtension");
    /*
    behaviors().doc(
        "The common KrossPythonExtension object enables passing "
        "of Kross::Object's from C/C++ to Python and "
        "backwards in a transparent way."
    );
    */
    //behaviors().supportRepr();
    behaviors().supportGetattr();
    behaviors().supportSequenceType();
    behaviors().supportMappingType();

    m_proxymethod = new Py::MethodDefExt<PythonExtension>(
        "", // methodname, not needed cause we use the method only internaly.
        0, // method that should handle the callback, not needed cause proxyhandler will handle it.
        Py::method_varargs_call_handler_t( proxyhandler ), // callback handler
        "" // documentation
    );
}

PythonExtension::~PythonExtension()
{
    #ifdef KROSS_PYTHON_EXTENSION_DTOR_DEBUG
        krossdebug( QString("PythonExtension::Destructor object=%1").arg(m_debuginfo) );
    #endif
    delete m_proxymethod;
    delete m_methods;
}

QObject* PythonExtension::object() const
{
    return m_object;
}

Py::List PythonExtension::updateMethods()
{
    delete m_methods;
    m_methods = new QHash<QByteArray, Py::Object>();

    Py::List list;
    if(m_object) {
        //list.append(Py::String("toPyQt")); // for PythonPyQtExtension
        const QMetaObject* metaobject = m_object->metaObject();
        const int count = metaobject->methodCount();
        for(int i = 0; i < count; ++i) {
            QMetaMethod member = metaobject->method(i);
            const QString signature = member.signature();
            const QByteArray name = signature.left(signature.indexOf('(')).toLatin1();
            if(! m_methods->contains(name)) {
                Py::Tuple self(3);
                self[0] = Py::Object(this); // reference to this instance
                self[1] = Py::Int(i); // the first index used for faster access
                self[2] = Py::String(name); // the name of the method

                m_methods->insert(name, Py::Object(PyCFunction_New( &m_proxymethod->ext_meth_def, self.ptr() ), true));
                list.append(self[2]);
            }
        }
    }
    return list;
}

Py::Object PythonExtension::getattr(const char* n)
{
    #ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
        krossdebug( QString("PythonExtension::getattr name='%1'").arg(n) );
    #endif

    // handle internal methods
    if(n[0] == '_') {
        if(strcmp(n,"__methods__") == 0) {
            return updateMethods();
        }

        /*
        if(strcmp(n,"__members__") == 0) {
            return PythonType<QStringList>::toPyObject( QStringList() << "__name__" );
        }
        if(strcmp(n,"__dict__") == 0) {
            return PythonType<QStringList>::toPyObject( QStringList() );
        }
        */

        if(strcmp(n,"__name__") == 0) {
            return PythonType<QString>::toPyObject( m_object->objectName() );
        }
        if(strcmp(n,"__class__") == 0) {
            return PythonType<QString>::toPyObject( m_object->metaObject()->className() );
        }

        #ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
            krossdebug( QString("PythonExtension::getattr name='%1' is internal.").arg(n) );
        #endif
        return Py::PythonExtension<PythonExtension>::getattr_methods(n);
    }

    // look if the attribute is a method
    if(! m_methods)
        updateMethods();

    if(m_methods->contains(n)) {
        #ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
            krossdebug( QString("PythonExtension::getattr name='%1' is a method.").arg(n) );
        #endif
        return m_methods->operator[]( n );
    }

    // look if the attribute is a property
    //TODO

    /*
    if(strcmp(methodname,"toPointer") == 0) {
        PyObject* qobjectptr = PyLong_FromVoidPtr( (void*) m_object.data() );
        //PyObject* o = Py_BuildValue ("N", mw);
        return Py::asObject( qobjectptr );
        //PythonPyQtExtension* pyqtextension = new PythonPyQtExtension(self, args);
        //return pyqtextension;
    }
    if(strcmp(methodname,"fromPointer") == 0) {
        QObject* object = dynamic_cast< QObject* >(PyLong_AsVoidPtr( args[0] ));
    }
    */

    krosswarning( QString("PythonExtension::getattr name='%1' TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!").arg(n) );
    return Py::None();
}

/*
const Py::Object PythonExtension::toPyObject(Kross::Object* object)
{
    if(! object) {
        #ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
            krossdebug("PythonExtension::toPyObject(Kross::Object) is NULL => Py::None");
        #endif
        return Py::None();
    }
    {
        Kross::Variant* variant = dynamic_cast<Kross::Variant*>( object );
        if(variant) return toPyObject( variant->getValue() );
    }
    {
        Kross::List* list = dynamic_cast<Kross::List*>( object );
        if(list) {
            Py::List pylist;
            foreach(QVariant v, list->getValue()) {
                Kross::Object::Ptr obj = qVariantValue< Kross::Object::Ptr >(v);
                pylist.append( toPyObject( obj.data() ) ); // recursive
            }
            return pylist;
        }
    }
    {
        Kross::Dict* dict = dynamic_cast<Kross::Dict*>( object );
        if(dict) {
            Py::Dict pydict;
            QMap<QString, QVariant> valuedict = dict->getValue();
            for(QMap<QString, QVariant>::Iterator it = valuedict.begin(); it != valuedict.end(); ++it) {
                const char* n = it.key().toLatin1().data();
                Kross::Object::Ptr obj = qVariantValue< Kross::Object::Ptr >( it.value() );
                pydict[ n ] = toPyObject( obj.data() ); // recursive
            }
            return pydict;
        }
    }
    #ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
        krossdebug( QString("Trying to handle PythonExtension::toPyObject() as PythonExtension") );
    #endif
    return Py::asObject( new PythonExtension(object) );
}
*/

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
        QMetaMethod metamethod = self->m_object->metaObject()->method( methodindex );
        if(metamethod.parameterTypes().size() != argssize) {
            bool found = false;
            const int count = self->m_object->metaObject()->methodCount();
            for(++methodindex; methodindex < count; ++methodindex) {
                metamethod = self->m_object->metaObject()->method( methodindex );
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
                    // crashes on shared containers like e.g. QStringList and QList
                    typeId = QMetaType::type( metamethod.typeName() );
                    //Q_ASSERT(typeId != QMetaType::Void);
                    #ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
                        krossdebug( QString("PythonExtension::proxyhandler typeName=%1 metatype.typeid=%2").arg(metamethod.typeName()).arg(typeId) );
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
            int r = self->m_object->qt_metacall(QMetaObject::InvokeMetaMethod, methodindex,
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
        krosswarning( QString("PythonExtension::proxyhandler Had exception: %1").arg(Py::value(e).as_string().c_str()) );
    }

    return Py_None;
}

int PythonExtension::sequence_length()
{
    return m_object->children().count();
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
    if(index < m_object->children().count())
        return Py::asObject(new PythonExtension( m_object->children().at(index) ));
    return Py::asObject( Py::new_reference_to( NULL ) );
}

Py::Object PythonExtension::sequence_slice(int from, int to)
{
    Py::List list;
    if(from >= 0) {
        const int count = m_object->children().count();
        for(int i = from; i <= to && i < count; ++i)
            list.append( Py::asObject(new PythonExtension( m_object->children().at(i) )) );
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
    return m_object->children().count();
}

Py::Object PythonExtension::mapping_subscript(const Py::Object& obj)
{
    QString name = Py::String(obj).as_string().c_str();
    QObject* object = m_object->findChild< QObject* >( name );
    if(! object) {
        foreach(QObject* o, m_object->children()) {
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

