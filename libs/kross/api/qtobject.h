/***************************************************************************
 * qtobject.h
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

#ifndef KROSS_API_QTOBJECT_H
#define KROSS_API_QTOBJECT_H

#include "class.h"

#include <QString>
#include <QObject>
#include <koffice_export.h>

namespace Kross { namespace Api {

    // Forward declarations.
    class Object;
    class Variant;
    class ScriptContainer;

    /**
     * Class to wrap \a QObject or inherited instances.
     *
     * This class publishs all SIGNAL's, SLOT's and Q_PROPERTY's
     * the QObject has.
     */
    class KROSS_EXPORT QtObject : public Class<QtObject>
    {
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<QtObject> Ptr;

            /**
             * Constructor.
             *
             * \param object The \a QObject instance this
             *        class wraps.
             * \param name The name this \a QtObject instance
             *        will be published under.
             */
            QtObject(QObject* object, const QString& name);

            /**
             * Destructor.
             */
            virtual ~QtObject();

            /// \see Kross::Api::Object::getClassName()
            virtual const QString getClassName() const;

            /**
             * Return the \a QObject instance this class wraps.
             *
             * \return The wrapped QObject.
             */
            QObject* getObject() const;


        private:
            /// The wrapped QObject.
            QObject* m_object;

            // QObject

            /// Return the classname. E.g. "QObject" or "QWidget".
            const QString className() const;
            /// Return the name the object has.
            const QString objectName() const;
            /// Set the name the object has.
            void setObjectName(const QString& name);

            // QProperty

            /// Return a property.
            QVariant property(const char* name);
            /// Set a property.
            bool setProperty(const char* name, const QVariant& value);
            /// Return a list of property names.
            QStringList propertyNames() const;
            /// Return the name of the property's type. E.g. "QString" or "int".
            QString propertyTypeName(const char* name);

            // Enumerator

            /// Return the name of the enumerator identified with the index-value.
            QString enumeratorName(int index) const;
            int enumeratorCount() const;
            /// Return a list of all enumerator names.
            QStringList enumeratorNames() const;
            /// Return the index the enumerator identified with name has.
            int enumeratorIndex(const char* name) const;

            // Methods

            QStringList methodNames() const;
            //bool invokeMethod(const char* name);

            /*
            // Slots

            /// Return a list of slot names.
            Kross::Api::Object::Ptr slotNames(Kross::Api::List::Ptr);
            /// Return true if the slot exists else false.
            Kross::Api::Object::Ptr hasSlot(Kross::Api::List::Ptr);
            /// Call a slot.
            Kross::Api::Object::Ptr callSlot(Kross::Api::List::Ptr);

            // Signals

            /// Return a list of signal names.
            Kross::Api::Object::Ptr signalNames(Kross::Api::List::Ptr);
            /// Return true if the signal exists else false.
            Kross::Api::Object::Ptr hasSignal(Kross::Api::List::Ptr);
            /// Emit a signal.
            Kross::Api::Object::Ptr emitSignal(Kross::Api::List::Ptr);

            /// Connect signal with a QObject slot.
            Kross::Api::Object::Ptr connectSignal(Kross::Api::List::Ptr);
            /// Disconnect signal from QObject slot.
            Kross::Api::Object::Ptr disconnectSignal(Kross::Api::List::Ptr);
            */

    };

}}

#endif

