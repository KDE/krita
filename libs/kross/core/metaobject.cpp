/***************************************************************************
 * metaobject.cpp
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

#include "metaobject.h"
#include "object.h"
//#include "function.h"
//#include "exception.h"
//#include <typeinfo>

#include <QHash>
#include <QVarLengthArray>
#include <QMetaMethod>
#include <QMetaProperty>

#include <kdebug.h>

using namespace Kross;

namespace Kross {

    /// \internal struct that represents the header implementation of QMetaObject's as defined in corelib/kernel/qmetaobject.cpp.
    struct Header
    {
        int revision;
        int className;
        int classInfoCount, classInfoData;
        int methodCount, methodData;
        int propertyCount, propertyData;
        int enumeratorCount, enumeratorData;
        // following items are Kross specific
        int propertyKrossData;
        int methodKrossData;
    };

    /// \internal method implementation.
    class Method
    {
        public:
            QByteArray parameters;
            QByteArray typeName;
            QByteArray tag;
            QByteArray inputSignature, outputSignature;
            QVarLengthArray<int, 4> inputTypes, outputTypes;
            int flags;

            Method(const QMetaMethod& m)
            {
                //parameters = ;
                typeName = m.typeName(); // return value
                tag = m.tag(); // moc-related
                //flags = 0x05; // MethodSignal|AccessProtected
                //inputSignature = "()";
                //outputSignature = "()";
            }
    };

    /// \internal property implementation.
    class Property
    {
        public:
            QByteArray typeName;
            QByteArray signature;
            int type;
            int flags;

            Property(const QMetaProperty& p)
            {
                typeName = p.typeName();
                //signature = ;
                //type = ;
                //flags = ;
            }
    };

    /// \internal number of int's per property.
    static const int intsperproperty = 2;
    /// \internal number of int's per method.
    static const int intspermethod = 4;

    /// \internal d-pointer class.
    class MetaObject::Private
    {
        public:
            QList< QObject* > objects;
            Header* header;
            QMap<QByteArray, Method> methods;
            QMap<QByteArray, Property> properties;
    };
}

MetaObject::MetaObject(const Object* object)
    : QMetaObject()
    , dptr(new Private())
{
    kDebug() << "MetaObject::Constructor" << endl;

    //Method testmethod;
    //dptr->methods.insert( QByteArray("objectfunc2(QVariant)"), testmethod );

    if(object->object())
        attachObject(object->object());
    else
        rebuild();
}

MetaObject::~MetaObject()
{
    //delete [] d.stringdata;
    //delete [] d.data;
    //delete dptr->header;
    delete dptr;
}

void MetaObject::rebuild()
{
    // fill the header struct
    QVarLengthArray<int> headerdata;
    {
        headerdata.resize( sizeof(Header) / sizeof(int) );
        dptr->header = reinterpret_cast< Header* >(headerdata.data());
        dptr->header->revision = 1;
        dptr->header->className = 0;
        dptr->header->classInfoCount = 0;
        dptr->header->classInfoData = 0;
        dptr->header->methodCount = dptr->methods.count();
        dptr->header->methodData = headerdata.size();
        dptr->header->propertyCount = dptr->properties.count();
        dptr->header->propertyData = dptr->header->methodData + dptr->header->methodCount * 5;
        dptr->header->enumeratorCount = 0;
        dptr->header->enumeratorData = 0;
    }

    // set custom part of the header
    {
        dptr->header->propertyKrossData = dptr->header->propertyData + dptr->header->propertyCount * 3;
        dptr->header->methodKrossData = dptr->header->propertyKrossData + dptr->header->propertyCount * intsperproperty;

        // needed to resize cause the custom data changed the size
        int data_size = headerdata.size() + (dptr->header->methodCount * (5+intspermethod)) + (dptr->header->propertyCount * (3+intsperproperty));
        foreach (const Method &mm, dptr->methods)
            data_size += 2 + mm.inputTypes.count() + mm.outputTypes.count();
        headerdata.resize(data_size + 1);
    }

    // the stringdata does contain a stream of all relevant meta-informations splitted
    // by NULL-bytes. The string starts with the classname.
    QByteArray stringdata("Kross::Object\0", 8192);
    {
        // prepare methods
        int offset = dptr->header->methodData;
        int signatureOffset = dptr->header->methodKrossData;
        int typeidOffset = dptr->header->methodKrossData + dptr->header->methodCount * intspermethod;
        headerdata[typeidOffset++] = 0; // eod

        // add each method
        for (QMap<QByteArray, Method>::ConstIterator it = dptr->methods.constBegin(); it != dptr->methods.constEnd(); ++it) {
            const Method &mm = it.value();
            // prototype
            headerdata[offset++] = stringdata.length();
            stringdata += it.key() + '\0';
            // parameters
            headerdata[offset++] = stringdata.length();
            stringdata += mm.parameters + '\0';
            // typeName
            headerdata[offset++] = stringdata.length();
            stringdata += mm.typeName + '\0';
            // tag
            headerdata[offset++] = stringdata.length();
            stringdata += mm.tag + '\0';
            // flags
            headerdata[offset++] = mm.flags;
            // inputSignature
            headerdata[signatureOffset++] = stringdata.length();
            stringdata += mm.inputSignature + '\0';
            // outputSignature
            headerdata[signatureOffset++] = stringdata.length();
            stringdata += mm.outputSignature + '\0';

            headerdata[signatureOffset++] = typeidOffset;
            headerdata[typeidOffset++] = mm.inputTypes.count();
            memcpy(headerdata.data() + typeidOffset, mm.inputTypes.data(), mm.inputTypes.count() * sizeof(int));
            typeidOffset += mm.inputTypes.count();

            headerdata[signatureOffset++] = typeidOffset;
            headerdata[typeidOffset++] = mm.outputTypes.count();
            memcpy(headerdata.data() + typeidOffset, mm.outputTypes.data(), mm.outputTypes.count() * sizeof(int));
            typeidOffset += mm.outputTypes.count();
        }

        Q_ASSERT(offset == dptr->header->propertyData);
        Q_ASSERT(signatureOffset == dptr->header->methodKrossData + dptr->header->methodCount * intspermethod);
        Q_ASSERT(typeidOffset == headerdata.size());

        // add each property
        signatureOffset = dptr->header->propertyKrossData;
        for(QMap<QByteArray, Property>::ConstIterator it = dptr->properties.constBegin(); it != dptr->properties.constEnd(); ++it) {
            const Property &mp = it.value();
            // name
            headerdata[offset++] = stringdata.length();
            stringdata += it.key() + '\0';
            // typeName
            headerdata[offset++] = stringdata.length();
            stringdata += mp.typeName + '\0';
            // flags
            headerdata[offset++] = mp.flags;
            // signature
            headerdata[signatureOffset++] = stringdata.length();
            stringdata += mp.signature + '\0';
            // type
            headerdata[signatureOffset++] = mp.type;
        }

        Q_ASSERT(offset == dptr->header->propertyKrossData);
        Q_ASSERT(signatureOffset == dptr->header->methodKrossData);
    }

    // finally put the metaobject together
    {
        char *string_data = new char[ stringdata.length() ];
        memcpy(string_data, stringdata, stringdata.length());

        uint *uint_data = new uint[ headerdata.size() ];
        memcpy(uint_data, headerdata.data(), headerdata.size() * sizeof(int));

        d.data = uint_data;
        d.extradata = 0;
        d.stringdata = string_data;
        d.superdata = &Object::staticMetaObject;
    }
}

void MetaObject::attachObject(QObject* object)
{
    dptr->objects.append( object );
    const QMetaObject* metaobject = object->metaObject();

    { // add methods
        const int methodcount = metaobject->methodCount();
        for(int i = 0; i < methodcount; ++i) {
            const QMetaMethod m = metaobject->method(i);
            if(m.access() != QMetaMethod::Public) continue; // Protected and Private members are not published
            dptr->methods.insert(m.signature(), Method(m));
        }
    }

    { // add properties
        const int propcount = metaobject->propertyCount();
        for(int i = 0; i < propcount; ++i) {
            const QMetaProperty p = metaobject->property(i);
            dptr->properties.insert(p.name(), Property(p));
        }
    }

    rebuild();
}
