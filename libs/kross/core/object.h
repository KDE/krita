/***************************************************************************
 * object.h
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

#ifndef KROSS_OBJECT_H
#define KROSS_OBJECT_H

//#include <QString>
//#include <QStringList>
//#include <QHash>
//#include <QVariant>
#include <QObject>
#include <QMetaType>
#include <QMetaObject>
#include <QMetaMethod>

#include <ksharedptr.h>
#include <koffice_export.h>

#include "krossconfig.h"

namespace Kross {

    /**
     * Base class that implements dynamic QObject's.
     */
    class KROSS_EXPORT Object : public QObject, public KShared
    {
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<Object> Ptr;

            /**
             * Constructor.
             *
             * \param object the QObject which should be wrapped.
             * \param name is the unique name the QObject should be published
             * under. Normaly this equals to \a Object::objectName .
             */
            explicit Object(QObject* const object = 0, const QString& name = QString::null);

            /**
             * Destructor.
             */
            virtual ~Object();

            /**
             * \return the wrapped QObject instance or NULL if there was
             * no object defined.
             */
            QObject* object() const;

            /// \internal static \a MetaObject instance.
            static const QMetaObject staticMetaObject;
            /// \internal dynamic \a MetaObject instance.
            virtual const QMetaObject* metaObject() const;
            /// \internal implementation to cast this instance.
            virtual void* qt_metacast(const char* classname);
            /// \internal implementation to call this instance.
            virtual int qt_metacall(QMetaObject::Call call, int id, void** args);

        private: //public slot

            /**
             * Relay an object. This method is a builtin public slot as
             * defined in the \a staticMetaObject instance.
             */
            void relay(QObject* object, const QMetaObject* metaobject, int index, QVariantList args);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

Q_DECLARE_METATYPE( Kross::Object::Ptr )

//qRegisterMetaType<Kross::Api::Object>("Kross::Api::Object");
//qRegisterMetaTypeStreamOperator<Kross::Api::Object>("Kross::Api::Object");

#endif

