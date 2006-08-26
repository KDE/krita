/***************************************************************************
 * metatype.h
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

#ifndef KROSS_METATYPE_H
#define KROSS_METATYPE_H

#include "krossconfig.h"
//#include "object.h"

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMetaType>

#include <typeinfo>

//#include <QDate>
//#include <QTime>
//#include <QDateTime>

namespace Kross {

    /**
     * Base class for metatype-implementations.
     */
    class MetaType
    {
        public:
            virtual ~MetaType() {}

            virtual int typeId() = 0;
            //virtual QObject* toObject() = 0;
            //virtual QVariant toVariant() = 0;
            virtual void* toVoidStar() = 0;
    };

    /**
     * Metatypes which are registered in the QMetaType system.
     */
    template<typename METATYPE>
    class MetaTypeImpl : public MetaType
    {
        public:
            MetaTypeImpl(const METATYPE& v) : m_variant(v) {
                #ifdef KROSS_METATYPE_DEBUG
                    krossdebug( QString("MetaTypeImpl<METATYPE> Ctor typeid=%1 typename=%2").arg(qMetaTypeId<METATYPE>()).arg(typeid(METATYPE).name()) );
                #endif
            }
            virtual ~MetaTypeImpl() {
                #ifdef KROSS_METATYPE_DEBUG
                    krossdebug( QString("MetaTypeImpl<METATYPE> Dtor typeid=%1 typename=%2").arg(qMetaTypeId<METATYPE>()).arg(typeid(METATYPE).name()) );
                #endif
            }

            virtual int typeId() { return qMetaTypeId<METATYPE>(); }
            //virtual QVariant toVariant() { return QVariant(typeId(), m_variant); }
            virtual void* toVoidStar() { return (void*) &m_variant; }

        private:
            METATYPE m_variant;
    };

    /**
     * Metatypes which are listened in QVariant::Type.
     */
    template<typename VARIANTTYPE>
    class MetaTypeVariant : public MetaType
    {
        public:
            MetaTypeVariant(const VARIANTTYPE& v) : m_value(v) {
                #ifdef KROSS_METATYPE_DEBUG
                    krossdebug( QString("MetaTypeVariant<VARIANTTYPE> Ctor value=%1 typename=%2").arg(qVariantFromValue(m_value).toString()).arg(qVariantFromValue(m_value).type()) );
                #endif
            }
            virtual ~MetaTypeVariant() {
                #ifdef KROSS_METATYPE_DEBUG
                    krossdebug( QString("MetaTypeVariant<VARIANTTYPE> Dtor value=%1 typename=%2").arg(qVariantFromValue(m_value).toString()).arg(qVariantFromValue(m_value).type()) );
                #endif
            }

            virtual int typeId() { return qVariantFromValue(m_value).type(); }
            //virtual QVariant toVariant() { return qVariantFromValue(m_value); }
            virtual void* toVoidStar() { return (void*) &m_value; }

        private:
            VARIANTTYPE m_value;
    };

    /**
     * Metatype for generic VoidStar pointers.
     */
    class MetaTypeVoidStar : public MetaType
    {
        public:
            MetaTypeVoidStar(int typeId, void* obj) : m_typeId(typeId), m_object(obj) {
                #ifdef KROSS_METATYPE_DEBUG
                    krossdebug( QString("MetaTypeVoidStar Ctor typeid=%1 typename=%2").arg(m_typeId).arg(typeid(m_object).name()) );
                #endif
            }
            virtual ~MetaTypeVoidStar() {
                #ifdef KROSS_METATYPE_DEBUG
                    krossdebug( QString("MetaTypeVoidStar Ctor typeid=%1 typename=%2").arg(m_typeId).arg(typeid(m_object).name()) );
                #endif
            }
            virtual int typeId() { return m_typeId; }
            //virtual QVariant toVariant() { return QVariant(m_typeId, m_object); }
            virtual void* toVoidStar() { return (void*) &m_object; }

        private:
            int m_typeId;
            void* m_object;
    };

}

#endif
