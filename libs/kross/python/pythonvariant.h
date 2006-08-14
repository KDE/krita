/***************************************************************************
 * pythonvariant.h
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

#ifndef KROSS_PYTHONVARIANT_H
#define KROSS_PYTHONVARIANT_H

#include "pythonconfig.h"
#include "../core/object.h"

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMetaType>

#include <typeinfo>





#include <QDate>
#include <QTime>
#include <QDateTime>

namespace Kross {

    /**********************************************************************
     * The PythonType helper classes used to cast between QVariant
     * and Py::Object values.
     *
     * Following QVariant::Type's are unimplemented yet (do we need them anyways?);
     *   QVariant::BitArray
     *   QVariant::Date
     *   QVariant::Time
     *   QVariant::DateTime
     *   QVariant::Bitmap
     *   QVariant::Brush
     *   QVariant::Char
     *   QVariant::Color
     *   QVariant::Cursor
     *   QVariant::Font
     *   QVariant::Icon
     *   QVariant::Image
     *   QVariant::KeySequence
     *   QVariant::Line
     *   QVariant::LineF
     *   QVariant::Locale
     *   QVariant::Palette
     *   QVariant::Pen
     *   QVariant::Pixmap
     *   QVariant::Point
     *   QVariant::PointArray
     *   QVariant::PointF
     *   QVariant::Polygon
     *   QVariant::Rect
     *   QVariant::RectF
     *   QVariant::RegExp
     *   QVariant::Region
     *   QVariant::Size
     *   QVariant::SizeF
     *   QVariant::SizePolicy
     *   QVariant::TextFormat
     *   QVariant::TextLength
     *   QVariant::Url
     */

    template<typename VARIANTTYPE, typename PYTYPE = Py::Object>
    struct PythonType
    {
        // template-specialisations need to implement following both static
        // functions to translate between QVariant and Py::Object objects.

        //inline static PYTYPE toPyObject(const VARIANTTYPE&) { return Py::None(); }
        //inline static QVARIANTTYPE toVariant(const VARIANTTYPE&) { return QVariant(); }
    };

    template<>
    struct PythonType<QVariant>
    {
        static Py::Object toPyObject(const QVariant& v);
        static QVariant toVariant(const Py::Object& obj);
    };

    template<>
    struct PythonType<int>
    {
        inline static Py::Object toPyObject(int i) {
            return Py::Int(i);
        }
        inline static int toVariant(const Py::Object& obj) {
            return int(Py::Int(obj));
        }
    };

    template<>
    struct PythonType<uint>
    {
        inline static Py::Object toPyObject(uint i) {
            return Py::Long( (unsigned long)i );
        }
        inline static uint toVariant(const Py::Object& obj) {
            return uint( (unsigned long)Py::Long(obj) );
        }
    };

    template<>
    struct PythonType<double>
    {
        inline static Py::Object toPyObject(double d) {
            return Py::Float(d);
        }
        inline static double toVariant(const Py::Object& obj) {
            return double(Py::Float(obj));
        }
    };

    template<>
    struct PythonType<bool>
    {
        inline static Py::Object toPyObject(bool b) {
            return Py::Int(b);
        }
        inline static double toVariant(const Py::Object& obj) {
            return bool(Py::Int(obj));
        }
    };

    template<>
    struct PythonType<qlonglong>
    {
        inline static Py::Object toPyObject(qlonglong l) {
            return Py::Long( (long)l );
        }
        inline static qlonglong toVariant(const Py::Object& obj) {
            return (long) Py::Long(obj);
        }
    };

    template<>
    struct PythonType<qulonglong>
    {
        inline static Py::Object toPyObject(qulonglong l) {
            return Py::Long( (unsigned long)l );
        }
        inline static qulonglong toVariant(const Py::Object& obj) {
            return (unsigned long) Py::Long(obj);
        }
    };

    template<>
    struct PythonType<QByteArray>
    {
        inline static Py::Object toPyObject(const QByteArray& ba) {
            return Py::String(ba.constData());
        }
        inline static QByteArray toVariant(const Py::Object& obj) {
            return QByteArray( Py::String(obj).as_string().c_str() );
        }
    };

    template<>
    struct PythonType<QString>
    {
        inline static Py::Object toPyObject(const QString& s) {
            return s.isNull() ? Py::String() : Py::String(s.toLatin1().data());
        }
        inline static QString toVariant(const Py::Object& obj) {
            /*
            #ifdef Py_USING_UNICODE
                PyTypeObject *type = (PyTypeObject*) object.type().ptr();
                if(type == &PyUnicode_Type) {
                    Py::unicodestring u = Py::String(object).as_unicodestring();
                    std::string s;
                    std::copy(u.begin(), u.end(), std::back_inserter(s));
                    return s.c_str();
                }
            #endif
            */
            return Py::String(obj).as_string().c_str();
        }
    };

    template<>
    struct PythonType<QStringList>
    {
        inline static Py::Object toPyObject(const QStringList& sl) {
            Py::List l;
            foreach(QString s, sl)
                l.append( PythonType<QString>::toPyObject(s) );
            return l;
        }
        inline static QStringList toVariant(const Py::Object& obj) {
            Py::List list(obj);
            QStringList l;
            const uint length = list.length();
            for(uint i = 0; i < length; i++)
                l.append( Py::String(list[i]).as_string().c_str() );
            return l;
        }
    };

    template<>
    struct PythonType<QVariantList,Py::List>
    {
        inline static Py::List toPyObject(const QVariantList& list) {
            Py::List l;
            foreach(QVariant v, list)
                l.append( PythonType<QVariant>::toPyObject(v) );
            return l;
        }
        inline static QVariantList toVariant(const Py::List& list) {
            QVariantList l;
            const uint length = list.length();
            for(uint i = 0; i < length; i++)
                l.append( PythonType<QVariant>::toVariant(list[i]) );
            return l;
        }
    };

    template<>
    struct PythonType<QVariantList>
    {
        inline static Py::Object toPyObject(const QVariantList& list) {
            return PythonType<QVariantList,Py::List>::toPyObject(list);
        }
        inline static QVariantList toVariant(const Py::Object& obj) {
            return PythonType<QVariantList,Py::List>::toVariant(Py::List(obj));
        }
    };

    template<>
    struct PythonType<QVariantList,Py::Tuple>
    {
        inline static Py::Tuple toPyObject(const QVariantList& list) {
            uint count = list.count();
            Py::Tuple tuple(count);
            for(uint i = 0; i < count; ++i)
                tuple.setItem(i, PythonType<QVariant>::toPyObject(list[i]));
            return tuple;
        }
        inline static QVariantList toVariant(const Py::Tuple& tuple) {
            QVariantList l;
            const uint size = tuple.size();
            for(uint i = 0; i < size; i++)
                l.append( PythonType<QVariant>::toVariant(tuple[i]) );
            return l;
        }
    };

    template<>
    struct PythonType<QVariantMap,Py::Dict>
    {
        inline static Py::Dict toPyObject(const QVariantMap& map) {
            Py::Dict d;
            for(QMap<QString, QVariant>::ConstIterator it = map.constBegin(); it != map.constEnd(); ++it)
                d.setItem( it.key().toLatin1().data(), PythonType<QVariant>::toPyObject(it.value()) );
            return d;
        }
        inline static QVariantMap toVariant(const Py::Dict& dict) {
            QVariantMap map;
            Py::List l = dict.keys();
            const uint length = l.length();
            for(Py::List::size_type i = 0; i < length; ++i) {
                const char* n = l[i].str().as_string().c_str();
                map.insert( n, PythonType<QVariant>::toVariant(dict[n]) );
            }
            return map;
        }
    };

    template<>
    struct PythonType<QVariantMap>
    {
        inline static Py::Object toPyObject(const QVariantMap& map) {
            return PythonType<QVariantMap,Py::Dict>::toPyObject(map);
        }
        inline static QVariantMap toVariant(const Py::Object& obj) {
            return PythonType<QVariantMap,Py::Dict>::toVariant( Py::Dict(obj.ptr()) );
        }
    };

    /*
    template<>
    struct PythonType<QDate>
    {
        inline static Py::Object toPyObject(const QDate& d) {
            Py::Module mod( PyImport_ImportModule((char*)"datetime"), true );
            PyObject* result = PyObject_CallMethod(mod.ptr(),(char*)"date","iii",d.year(),d.month(),d.day());
            return Py::Object(result, true);
        }
        inline static QDate toVariant(const Py::Object& obj) {
            return QDate( Py::Int(obj.getAttr("year")), Py::Int(obj.getAttr("month")), Py::Int(obj.getAttr("day")) );
        }
    };

    template<>
    struct PythonType<QTime>
    {
        inline static Py::Object toPyObject(const QTime& t) {
            Py::Module mod( PyImport_ImportModule((char*)"datetime"), true );
            PyObject* result = PyObject_CallMethod(mod.ptr(),(char*)"time",(char*)"iii",t.hour(),t.minute(),t.second());
            return Py::Object(result, true);
        }
        inline static QTime toVariant(const Py::Object& obj) {
            return QTime( Py::Int(obj.getAttr("hour")), Py::Int(obj.getAttr("minute")), Py::Int(obj.getAttr("second")) );
        }
    };

    template<>
    struct PythonType<QDateTime>
    {
        inline static Py::Object toPyObject(const QDateTime& dt) {
            QDate d = dt.date();
            QTime t = dt.time();
            Py::Module mod( PyImport_ImportModule((char*)"datetime"), true );
            PyObject* result = PyObject_CallMethod(mod.ptr(),(char*)"datetime",(char*)"iiiiii",d.year(),d.month(),d.day(),t.hour(),t.minute(),t.second());
            return Py::Object(result, true);
        }
        inline static QDateTime toVariant(const Py::Object& obj) {
            return QDateTime( PythonType<QDate>::toVariant(obj), PythonType<QTime>::toVariant(obj) );
        }
    };
    */

    /*
    template<>
    struct PythonType<QObject>
    {
        inline static Py::Object toPyObject(QObject* obj) {
            return Py::asObject( new PythonExtension(obj) );
        }
        inline static QVariant toVariant(int typeId, const Py::Object& obj) {
            if(Py::PythonExtension<PythonExtension>::check( obj )) {
                Py::ExtensionObject<PythonExtension> extobj(object);
                PythonExtension* extension = extobj.extensionObject();
                return QVariant(typeId, extension->object());
            }
            return QVariant();
        }
        inline static QVariant toVariant(const Py::Object& obj) {
            if(Py::PythonExtension<PythonExtension>::check( obj )) {
                Py::ExtensionObject<PythonExtension> extobj(object);
                PythonExtension* extension = extobj.extensionObject();
                QObject* o = extension->object();
                int typeId = QMetaType::type( o->metaObject()->className() );
                return QVariant(typeId, o);
            }
            return QVariant();
        }
    };
    */

    /*
    template<>
    struct PythonType< void* >
    {
        inline static Py::Object toPyObject(void* p) {
            PyObject* pyobj = PyLong_FromVoidPtr(p);
            return Py::asObject(pyobj);
        }
        inline static QVariant toVariant(int varianttypeid, const Py::Long& obj) {
            void* p = PyLong_AsVoidPtr(obj.ptr());
            return QVariant(varianttypeid, p);
        }
    };
    */

    /**********************************************************************
     * The PythonVariant helper classes used as temp objects within
     * PythonExtension to translate an argument into a void* needed
     * for QGenericArgument's data pointer.
     */

    struct PythonVariant
    {
        virtual ~PythonVariant() {}
        virtual void* toVoidStar() = 0;
        //virtual QVariant toVariant(int type) = 0;
        //virtual Py::Object toPyObject() = 0;

        static PythonVariant* create(const char* typeName);
        static PythonVariant* create(const char* typeName, const Py::Object& object);
    };

    template<typename VARIANTTYPE>
    struct PythonVariantImpl : public PythonVariant
    {
        PythonVariantImpl(const VARIANTTYPE& v) : m_variant(v) {}
        PythonVariantImpl(const Py::Object& obj) : m_variant( PythonType<VARIANTTYPE>::toVariant(obj) ) {
            krossdebug( QString("PythonVariantImpl<VARIANTTYPE> CTOR typename=%1").arg(typeid(VARIANTTYPE).name()) );
        }
        virtual ~PythonVariantImpl() {
            krossdebug( QString("PythonVariantImpl<VARIANTTYPE> DTOR typename=%1").arg(typeid(VARIANTTYPE).name()) );
        }
        virtual void* toVoidStar() { return (void*) &m_variant; }
        VARIANTTYPE m_variant;
    };

    template<>
    struct PythonVariantImpl<Object::Ptr> : public PythonVariant
    {
        PythonVariantImpl(Object::Ptr obj) : m_object(obj) {
            Q_ASSERT(m_object.data());
            krossdebug( QString("PythonVariantImpl<Object::Ptr> CTOR objectName=%1 typename=%2").arg(m_object->objectName()).arg(typeid(m_object).name()) );
        }
        virtual ~PythonVariantImpl() {
            krossdebug( QString("PythonVariantImpl<Object::Ptr> DTOR objectName=%1 typename=%2").arg(m_object->objectName()).arg(typeid(m_object).name()) );
        }
        virtual void* toVoidStar() { return (void*) &m_object; }
        Object::Ptr m_object;
    };

    template<>
    struct PythonVariantImpl< void* > : public PythonVariant
    {
        PythonVariantImpl(int typeId, void* obj) : m_typeId(typeId), m_object(obj) {
            krossdebug( QString("PythonVariantImpl< void* > CTOR typeid=%1 typename=%2").arg(m_typeId).arg(typeid(m_object).name()) );
        }
        virtual ~PythonVariantImpl() {
            krossdebug( QString("PythonVariantImpl< void* > DTOR typeid=%1 typename=%2").arg(m_typeId).arg(typeid(m_object).name()) );
        }
        virtual void* toVoidStar() { return (void*) &m_object; }
        int m_typeId;
        void* m_object;
    };

    template<>
    struct PythonVariantImpl<void> : public PythonVariant
    {
        PythonVariantImpl() {}
        virtual ~PythonVariantImpl() {}
        virtual void* toVoidStar() { return (void*)0; }
    };

}

#endif
