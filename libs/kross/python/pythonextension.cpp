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
#include "pythonobject.h"

#include "../api/variant.h"
#include "../api/dict.h"
#include "../api/exception.h"
//Added by qt3to4:
#include <Q3ValueList>

using namespace Kross::Python;

PythonExtension::PythonExtension(Kross::Api::Object* object)
    : Py::PythonExtension<PythonExtension>()
    , m_object(object)
{
#ifdef KROSS_PYTHON_EXTENSION_CTOR_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::Constructor objectname='%1' objectclass='%2'").arg(m_object->getName()).arg(m_object->getClassName()) );
#endif

    behaviors().name("KrossPythonExtension");
    /*
    behaviors().doc(
        "The common KrossPythonExtension object enables passing "
        "of Kross::Api::Object's from C/C++ to Python and "
        "backwards in a transparent way."
    );
    */
    behaviors().supportGetattr();

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
    krossdebug( QString("Kross::Python::PythonExtension::Destructor objectname='%1' objectclass='%2'").arg(m_object->getName()).arg(m_object->getClassName()) );
#endif
    delete m_proxymethod;
}

Py::Object PythonExtension::str()
{
    QString s = m_object->getName();
    return toPyObject(s.isEmpty() ? m_object->getClassName() : s);
}

Py::Object PythonExtension::repr()
{
    return toPyObject( m_object->toString() );
}

Py::Object PythonExtension::getattr(const char* n)
{
#ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::getattr name='%1'").arg(n) );
#endif

    if(n[0] == '_') {
        if(n == "__methods__") {
            Py::List methods;
            QStringList calls = m_object->getCalls();
            for(QStringList::Iterator it = calls.begin(); it != calls.end(); ++it) {
#ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
                krossdebug( QString("Kross::Python::PythonExtension::getattr name='%1' callable='%2'").arg(n).arg(*it) );
#endif
                methods.append(Py::String( (*it).latin1() ));
            }
            return methods;
        }

        if(n == "__members__") {
            Py::List members;
            QMap<QString, Kross::Api::Object::Ptr> children = m_object->getChildren();
            QMap<QString, Kross::Api::Object::Ptr>::Iterator it( children.begin() );
            for(; it != children.end(); ++it) {
#ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
                krossdebug( QString("Kross::Python::PythonExtension::getattr n='%1' child='%2'").arg(n).arg(it.key()) );
#endif
                members.append(Py::String( it.key().latin1() ));
            }
            return members;
        }

        //if(n == "__dict__") { krosswarning( QString("PythonExtension::getattr(%1) __dict__").arg(n) ); return Py::None(); }
        //if(n == "__class__") { krosswarning( QString("PythonExtension::getattr(%1) __class__").arg(n) ); return Py::None(); }

#ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
        krossdebug( QString("Kross::Python::PythonExtension::getattr name='%1' is a internal name.").arg(n) );
#endif
        return Py::PythonExtension<PythonExtension>::getattr_methods(n);
    }

    // Redirect the call to our static proxy method which will take care
    // of handling the call.
    Py::Tuple self(2);
    self[0] = Py::Object(this);
    self[1] = Py::String(n);
    return Py::Object(PyCFunction_New( &m_proxymethod->ext_meth_def, self.ptr() ), true);
}

/*
Py::Object PythonExtension::getattr_methods(const char* n)
{
#ifdef KROSS_PYTHON_EXTENSION_GETATTRMETHOD_DEBUG
    krossdebug( QString("PythonExtension::getattr_methods name=%1").arg(n) );
#endif
    return Py::PythonExtension<PythonExtension>::getattr_methods(n);
}

int PythonExtension::setattr(const char* name, const Py::Object& value)
{
#ifdef KROSS_PYTHON_EXTENSION_SETATTR_DEBUG
    krossdebug( QString("PythonExtension::setattr name=%1 value=%2").arg(name).arg(value.as_string().c_str()) );
#endif
    return Py::PythonExtension<PythonExtension>::setattr(name, value);
}
*/

Kross::Api::List* PythonExtension::toObject(const Py::Tuple& tuple)
{
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toObject(Py::Tuple)") );
#endif

    Q3ValueList<Kross::Api::Object::Ptr> l;
    uint size = tuple.size();
    for(uint i = 0; i < size; i++)
        l.append( Kross::Api::Object::Ptr(toObject(tuple[i])) );
    return new Kross::Api::List(l);
}

Kross::Api::List* PythonExtension::toObject(const Py::List& list)
{
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toObject(Py::List)") );
#endif

    Q3ValueList<Kross::Api::Object::Ptr> l;
    uint length = list.length();
    for(uint i = 0; i < length; i++)
        l.append( Kross::Api::Object::Ptr(toObject(list[i])) );
    return new Kross::Api::List(l);
}

Kross::Api::Dict* PythonExtension::toObject(const Py::Dict& dict)
{
    QMap<QString, Kross::Api::Object::Ptr> map;
    Py::List l = dict.keys();
    uint length = l.length();
    for(Py::List::size_type i = 0; i < length; ++i) {
        const char* n = l[i].str().as_string().c_str();
        map.replace(n, Kross::Api::Object::Ptr(toObject(dict[n])));
    }
    return new Kross::Api::Dict(map);
}

Kross::Api::Object* PythonExtension::toObject(const Py::Object& object)
{
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toObject(Py::Object) object='%1'").arg(object.as_string().c_str()) );
#endif
    if(object == Py::None())
        return 0;
    PyTypeObject *type = (PyTypeObject*) object.type().ptr();
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toObject(Py::Object) type='%1'").arg(type->tp_name) );
#endif
    if(type == &PyInt_Type)
        return new Kross::Api::Variant(int(Py::Int(object)));
    if(type == &PyBool_Type)
        return new Kross::Api::Variant(QVariant(object.isTrue(),0));
    if(type == &PyLong_Type)
        return new Kross::Api::Variant(qlonglong(long(Py::Long(object))));
    if(type == &PyFloat_Type)
        return new Kross::Api::Variant(double(Py::Float(object)));

    if( PyType_IsSubtype(type,&PyString_Type) ) {
#ifdef Py_USING_UNICODE
	/*
        if(type == &PyUnicode_Type) {
            Py::unicodestring u = Py::String(object).as_unicodestring();
            std::string s;
            std::copy(u.begin(), u.end(), std::back_inserter(s));
            return new Kross::Api::Variant(s.c_str());
        }
	*/
#endif
        return new Kross::Api::Variant(object.as_string().c_str());
    }

    if(type == &PyTuple_Type)
        return toObject(Py::Tuple(object));
    if(type == &PyList_Type)
        return toObject(Py::List(object));
    if(type == &PyDict_Type)
        return toObject(Py::Dict(object.ptr()));

    if(object.isInstance())
        return new PythonObject(object);

    Py::ExtensionObject<PythonExtension> extobj(object);
    PythonExtension* extension = extobj.extensionObject();
    if(! extension) {
        krosswarning("EXCEPTION in PythonExtension::toObject(): Failed to determinate PythonExtension object.");
        throw Py::Exception("Failed to determinate PythonExtension object.");
    }
    if(! extension->m_object) {
        krosswarning("EXCEPTION in PythonExtension::toObject(): Failed to convert the PythonExtension object into a Kross::Api::Object.");
        throw Py::Exception("Failed to convert the PythonExtension object into a Kross::Api::Object.");
    }

#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
    krossdebug( "Kross::Python::PythonExtension::toObject(Py::Object) successfully converted into Kross::Api::Object." );
#endif
    return extension->m_object.data();
}

const Py::Object PythonExtension::toPyObject(const QString& s)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toPyObject(QString)") );
#endif
    return s.isNull() ? Py::String() : Py::String(s.latin1());
}

const Py::List PythonExtension::toPyObject(const QStringList& list)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toPyObject(QStringList)") );
#endif
    Py::List l;
    for(QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
        l.append(toPyObject(*it));
    return l;
}

const Py::Dict PythonExtension::toPyObject(const QMap<QString, QVariant>& map)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toPyObject(QMap<QString,QVariant>)") );
#endif
    Py::Dict d;
    for(QMap<QString, QVariant>::ConstIterator it = map.constBegin(); it != map.constEnd(); ++it)
        d.setItem(it.key().latin1(), toPyObject(it.data()));
    return d;
}

const Py::List PythonExtension::toPyObject(const QList<QVariant>& list)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toPyObject(QValueList<QVariant>)") );
#endif
    Py::List l;
    for(QList<QVariant>::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
        l.append(toPyObject(*it));
    return l;
}

const Py::List PythonExtension::toPyObject(const Q3ValueList<QVariant>& list)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toPyObject(QValueList<QVariant>)") );
#endif
    Py::List l;
    for(Q3ValueList<QVariant>::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
        l.append(toPyObject(*it));
    return l;
}

const Py::Object PythonExtension::toPyObject(const QVariant& variant)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toPyObject(QVariant) typename='%1'").arg(variant.typeName()) );
#endif

    switch(variant.type()) {
        case QVariant::Invalid:
            return Py::None();
        case QVariant::Bool:
            return Py::Int(variant.toBool());
        case QVariant::Int:
            return Py::Int(variant.toInt());
        case QVariant::UInt:
            return Py::Long((unsigned long)variant.toUInt());
        case QVariant::Double:
            return Py::Float(variant.toDouble());
        case QVariant::Date:
        case QVariant::Time:
        case QVariant::DateTime:
        case QVariant::ByteArray:
        case QVariant::BitArray:
        //case QVariant::CString:
        case QVariant::String:
            return toPyObject(variant.toString());
        case QVariant::StringList:
            return toPyObject(variant.toStringList());
        case QVariant::Map:
            return toPyObject(variant.toMap());
        case QVariant::List:
            return toPyObject(variant.toList());

        // To handle following both cases is a bit difficult
        // cause Python doesn't spend an easy possibility
        // for such large numbers (TODO maybe BigInt?). So,
        // we risk overflows here, but well...
        case QVariant::LongLong: {
            qlonglong l = variant.toLongLong();
            //return (l < 0) ? Py::Long((long)l) : Py::Long((unsigned long)l);
            return Py::Long((long)l);
            //return Py::Long(PyLong_FromLong( (long)l ), true);
        } break;
        case QVariant::ULongLong: {
            return Py::Long((unsigned long)variant.toULongLong());
        } break;

        default: {
            krosswarning( QString("Kross::Python::PythonExtension::toPyObject(QVariant) Not possible to convert the QVariant type '%1' to a Py::Object.").arg(variant.typeName()) );
            return Py::None();
        }
    }
}

const Py::Object PythonExtension::toPyObject(Kross::Api::Object* object)
{
    if(! object) {
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
        krossdebug("Kross::Python::PythonExtension::toPyObject(Kross::Api::Object) is NULL => Py::None");
#endif
        return Py::None();
    }

    const QString classname = object->getClassName();
    if(classname == "Kross::Api::Variant") {
        QVariant v = static_cast<Kross::Api::Variant*>( object )->getValue();
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
        krossdebug( QString("Kross::Python::PythonExtension::toPyObject(Kross::Api::Object) is Kross::Api::Variant %1").arg(v.toString()) );
#endif
        return toPyObject(v);
    }

    if(classname == "Kross::Api::List") {
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
        krossdebug("Kross::Python::PythonExtension::toPyObject(Kross::Api::Object) is Kross::Api::List");
#endif
        Py::List pylist;
        Kross::Api::List* list = static_cast<Kross::Api::List*>( object );
        Q3ValueList<Kross::Api::Object::Ptr> valuelist = list->getValue();
        for(Q3ValueList<Kross::Api::Object::Ptr>::Iterator it = valuelist.begin(); it != valuelist.end(); ++it)
            pylist.append( toPyObject( (*it).data() ) ); // recursive
        return pylist;
    }

    if(classname == "Kross::Api::Dict") {
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
        krossdebug("Kross::Python::PythonExtension::toPyObject(Kross::Api::Object) is Kross::Api::Dict");
#endif
        Py::Dict pydict;
        Kross::Api::Dict* dict = static_cast<Kross::Api::Dict*>( object );
        QMap<QString, Kross::Api::Object::Ptr> valuedict = dict->getValue();
        for(QMap<QString, Kross::Api::Object::Ptr>::Iterator it = valuedict.begin(); it != valuedict.end(); ++it) {
            const char* n = it.key().latin1();
            pydict[ n ] = toPyObject( it.data().data() ); // recursive
        }
        return pydict;
    }

#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    krossdebug( QString("Trying to handle PythonExtension::toPyObject(%1) as PythonExtension").arg(object->getClassName()) );
#endif
    return Py::asObject( new PythonExtension(object) );
}

const Py::Tuple PythonExtension::toPyTuple(Kross::Api::List* list)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    krossdebug( QString("Kross::Python::PythonExtension::toPyTuple(Kross::Api::List) name='%1'").arg(list ? list->getName() : "NULL") );
#endif
    uint count = list ? list->count() : 0;
    Py::Tuple tuple(count);
    for(uint i = 0; i < count; i++)
        tuple.setItem(i, toPyObject(list->item(i)));
    return tuple;
}

PyObject* PythonExtension::proxyhandler(PyObject *_self_and_name_tuple, PyObject *args)
{
    Py::Tuple tuple(_self_and_name_tuple);
    PythonExtension *self = static_cast<PythonExtension*>( tuple[0].ptr() );
    QString methodname = Py::String(tuple[1]).as_string().c_str();

    try {
        Kross::Api::List::Ptr arguments = Kross::Api::List::Ptr( toObject( Py::Tuple(args) ) );

#ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
        krossdebug( QString("Kross::Python::PythonExtension::proxyhandler methodname='%1' arguments='%2'").arg(methodname).arg(arguments->toString()) );
#endif

        if(self->m_object->hasChild(methodname)) {
#ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
            krossdebug( QString("Kross::Python::PythonExtension::proxyhandler methodname='%1' is a child object of '%2'.").arg(methodname).arg(self->m_object->getName()) );
#endif
            Kross::Api::Object::Ptr res = self->m_object->getChild(methodname)->call(QString::null, arguments);
            Py::Object result = toPyObject(res.data());
            result.increment_reference_count();
            return result.ptr();
        }
#ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
        krossdebug( QString("Kross::Python::PythonExtension::proxyhandler try to call function with methodname '%1' in object '%2'.").arg(methodname).arg(self->m_object->getName()) );
#endif
        Kross::Api::Object::Ptr res = self->m_object->call(methodname, arguments);
        Py::Object result = toPyObject(res.data());
        result.increment_reference_count();
        return result.ptr();
    }
    catch(Py::Exception& e) {
        const QString err = Py::value(e).as_string().c_str();
        krosswarning( QString("Py::Exception in Kross::Python::PythonExtension::proxyhandler %1").arg(err) );
        //throw e;
    }
    catch(Kross::Api::Exception::Ptr e) {
        const QString err = e->toString();
        krosswarning( QString("Kross::Api::Exception in Kross::Python::PythonExtension::proxyhandler %1").arg(err) );
        // Don't throw here cause it will end in a crash depp in python. The
        // error is already handled anyway.
        //throw Py::Exception( (char*) e->toString().latin1() );
    }

    return Py_None;
}
