/***************************************************************************
 * pythonvariant.cpp
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

#include "pythonvariant.h"
#include "pythonextension.h"

#include <QWidget>

using namespace Kross;

Py::Object PythonType<QVariant>::toPyObject(const QVariant& v)
{
    #ifdef KROSS_PYTHON_VARIANT_DEBUG
        krossdebug( QString("PythonType<QVariant>::toPyObject variant.toString=%1 variant.typeid=%2 variant.typeName=%3").arg(v.toString()).arg(v.type()).arg(v.typeName()) );
    #endif

    switch( v.type() ) {
        case QVariant::Int:
            return PythonType<int>::toPyObject(v.toInt());
        case QVariant::UInt:
            return PythonType<uint>::toPyObject(v.toUInt());
        case QVariant::Double:
            return PythonType<double>::toPyObject(v.toDouble());
        case QVariant::ByteArray:
            return PythonType<QByteArray>::toPyObject(v.toByteArray());
        case QVariant::String:
            return PythonType<QString>::toPyObject(v.toString());
        case QVariant::Bool:
            return PythonType<bool>::toPyObject(v.toBool());
        case QVariant::StringList:
            return PythonType<QStringList>::toPyObject(v.toStringList());
        case QVariant::Map:
            return PythonType<QVariantMap>::toPyObject(v.toMap());
        case QVariant::List:
            return PythonType<QVariantList>::toPyObject(v.toList());
        case QVariant::LongLong:
            return PythonType<qlonglong>::toPyObject(v.toLongLong());
        case QVariant::ULongLong:
            return PythonType<qlonglong>::toPyObject(v.toULongLong());

        case QVariant::Invalid: {
            #ifdef KROSS_PYTHON_VARIANT_DEBUG
                krossdebug( QString("PythonType<QVariant>::toPyObject variant=%1 is QVariant::Invalid. Returning Py:None.").arg(v.toString()) );
            #endif
            //return Py::None();
        } // fall through

        case QVariant::UserType: {
            #ifdef KROSS_PYTHON_VARIANT_DEBUG
                krossdebug( QString("PythonType<QVariant>::toPyObject variant=%1 is QVariant::UserType. Trying to cast now.").arg(v.toString()) );
            #endif
        } // fall through

        default: {
            if( strcmp(v.typeName(),"float") == 0 ) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("PythonType<QVariant>::toPyObject Casting '%1' to double").arg(v.typeName()) );
                #endif
                return PythonType<double>::toPyObject(v.toDouble());
            }

            if( qVariantCanConvert< QWidget* >(v) ) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("PythonType<QVariant>::toPyObject Casting '%1' to QWidget").arg(v.typeName()) );
                #endif
                QWidget* widget = qvariant_cast< QWidget* >(v);
                if(! widget) {
                    #ifdef KROSS_PYTHON_VARIANT_DEBUG
                        krossdebug( QString("PythonType<QVariant>::toPyObject To QWidget casted '%1' is NULL").arg(v.typeName()) );
                    #endif
                    return Py::None();
                }
                return Py::asObject(new PythonExtension(widget));
            }

            if( qVariantCanConvert< QObject* >(v) ) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("PythonType<QVariant>::toPyObject Casting '%1' to QObject").arg(v.typeName()) );
                #endif
                QObject* obj = qvariant_cast< QObject* >(v);
                if(! obj) {
                    #ifdef KROSS_PYTHON_VARIANT_DEBUG
                        krossdebug( QString("PythonType<QVariant>::toPyObject To QObject casted '%1' is NULL").arg(v.typeName()) );
                    #endif
                    return Py::None();
                }
                return Py::asObject(new PythonExtension(obj));
            }

            //QObject* obj = (*reinterpret_cast< QObject*(*)>( variantargs[0]->toVoidStar() ));
            //PyObject* qobjectptr = PyLong_FromVoidPtr( (void*) variantargs[0]->toVoidStar() );

            //if(v.type() == QVariant::Invalid) return Py::None();

            #ifdef KROSS_PYTHON_VARIANT_DEBUG
                krosswarning( QString("PythonType<QVariant>::toPyObject Not possible to convert the QVariant '%1' with type '%2' (%3) to a Py::Object.").arg(v.toString()).arg(v.typeName()).arg(v.type()) );
            #endif
            throw Py::TypeError( QString("Variant of type %1 can not be casted to a python object.").arg(v.typeName()).toLatin1().constData() );
        }
    }
}

QVariant PythonType<QVariant>::toVariant(const Py::Object& obj)
{
    if(obj == Py::None())
        return QVariant();

    PyTypeObject *type = (PyTypeObject*) obj.type().ptr();

    if(type == &PyInt_Type)
        return PythonType<int>::toVariant(obj);
    if(type == &PyLong_Type)
        return PythonType<qlonglong>::toVariant(obj);
    if(type == &PyFloat_Type)
        return PythonType<double>::toVariant(obj);
    if(type == &PyBool_Type)
        return PythonType<bool>::toVariant(obj);

    if(PyType_IsSubtype(type,&PyString_Type))
        return PythonType<QString>::toVariant(obj);

    if(type == &PyTuple_Type)
        return PythonType<QVariantList,Py::Tuple>::toVariant(Py::Tuple(obj));
    if(type == &PyList_Type)
        return PythonType<QVariantList,Py::List>::toVariant(Py::List(obj));
    if(type == &PyDict_Type)
        return PythonType<QVariantMap,Py::Dict>::toVariant(Py::Dict(obj.ptr()));

    if(obj.isInstance()) {
        #ifdef KROSS_PYTHON_VARIANT_DEBUG
            krossdebug( QString("PythonType<QVariant>::toVariant IsInstance=TRUE") );
        #endif
        //return new PythonType(object);
    }

    Py::ExtensionObject<PythonExtension> extobj(obj);
    PythonExtension* extension = extobj.extensionObject();
    if(! extension) {
        #ifdef KROSS_PYTHON_VARIANT_DEBUG
            krosswarning( QString("PythonType<QVariant>::toVariant Failed to determinate PythonExtension for object=%1").arg(obj.as_string().c_str()) );
        #endif
        throw Py::RuntimeError( QString("Failed to determinate PythonExtension object.").toLatin1().constData() );
    }

    const QVariant variant = qVariantFromValue( extension->object() );
    #ifdef KROSS_PYTHON_VARIANT_DEBUG
        if(extension->object())
            krossdebug( QString("PythonType<QVariant>::toVariant KrossObject.objectName=%1 KrossObject.className=%2 QVariant.toString=%3 QVariant.typeName=%4").arg(extension->object()->objectName()).arg(extension->object()->metaObject()->className()).arg(variant.toString()).arg(variant.typeName()) );
        else
            krossdebug( QString("PythonType<QVariant>::toVariant The PythonExtension object does not have a valid QObject") );
    #endif
    return variant;
}

MetaType* PythonMetaTypeFactory::create(const char* typeName, const Py::Object& object)
{
    int typeId = QVariant::nameToType(typeName);

    #ifdef KROSS_PYTHON_VARIANT_DEBUG
        krossdebug( QString("PythonMetaTypeFactory::create object=%1 typeName=%2 metatype.id=%3 variant.id=%4").arg(object.as_string().c_str()).arg(typeName).arg(QMetaType::type(typeName)).arg(typeId) );
    #endif

    switch(typeId) {
        case QVariant::Int:
            return new PythonMetaTypeVariant<int>(object);
        case QVariant::UInt:
            return new PythonMetaTypeVariant<uint>(object);
        case QVariant::Double:
            return new PythonMetaTypeVariant<double>(object);
        case QVariant::Bool:
            return new PythonMetaTypeVariant<bool>(object);

        case QVariant::ByteArray:
            return new PythonMetaTypeVariant<QByteArray>(object);
        case QVariant::String:
            return new PythonMetaTypeVariant<QString>(object);

        case QVariant::StringList:
            return new PythonMetaTypeVariant<QStringList>(object);
        case QVariant::Map:
            return new PythonMetaTypeVariant<QVariantMap>(object);
        case QVariant::List:
            return new PythonMetaTypeVariant<QVariantList>(object);

        case QVariant::LongLong:
            return new PythonMetaTypeVariant<qlonglong>(object);
        case QVariant::ULongLong:
            return new PythonMetaTypeVariant<qulonglong>(object);

        case QVariant::Invalid: // fall through
        case QVariant::UserType: // fall through
        default: {
            //int metaid = QMetaType::type(typeName);

            if(Py::PythonExtension<PythonExtension>::check( object )) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("PythonMetaTypeFactory::create Py::Object '%1' with typename '%2' is a PythonExtension object").arg(object.as_string().c_str()).arg(typeName) );
                #endif

                Py::ExtensionObject<PythonExtension> extobj(object);
                PythonExtension* extension = extobj.extensionObject();
                Q_ASSERT( extension->object() );
                return new MetaTypeVoidStar( typeId, extension->object() );
            }

            //QVariant v = PythonType<QVariant>::toVariant(object);
            //krossdebug( QString("PythonVariant::create Converted Py::Object '%1' with type '%2 %3' to QVariant with type '%4 %5'").arg(object.as_string().c_str()).arg(typeName).arg(typeId).arg(v.toString()).arg(v.typeName()) );
            //if(typeId == QVariant::Invalid) return new PythonVariantImpl<void>();
            //return new PythonVariantImpl<QVariant>(v);

            #ifdef KROSS_PYTHON_VARIANT_DEBUG
                krosswarning( QString("PythonMetaTypeFactory::create Not possible to convert the Py::Object '%1' to QVariant with '%2' and metaid '%3'").arg(object.as_string().c_str()).arg(typeName).arg(typeId) );
            #endif
            throw Py::TypeError( QString("Invalid typename %1").arg(typeName).toLatin1().constData() );
        } break;
    }
}

