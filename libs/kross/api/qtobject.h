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

#include <qstring.h>
#include <qobject.h>

// Forward-declaration of the builtin Qt QUObject struct.
struct QUObject;

namespace Kross { namespace Api {

    // Forward declarations.
    class Object;
    class Variant;
    class ScriptContainer;
    class ScriptContrainer;

    /**
     * Class to wrap \a QObject or inherited instances.
     *
     * This class publishs all SIGNAL's, SLOT's and Q_PROPERTY's
     * the QObject has.
     */
    class QtObject : public Kross::Api::Class<QtObject>
    {
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<QtObject> Ptr;

            /**
             * Constructor.
             *
             * \param parent The \a QObject instance this
             *        class is children of.
             * \param object The \a QObject instance this
             *        class wraps.
             * \param name The unique name this \a QtObject
             *       instance has. If not defined then the
             *       \a QObject::name() will be used.
             */
            QtObject(Object::Ptr parent, QObject* object, const QString& name = QString::null);

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
            QObject* getObject();

            /**
            * Build a Qt QUObject struct out of the Qt signal or
            * slot signature and the passed \a List arguments.
            *
            * \throw RuntimeException If the try to translate \p arguments
            *       failed.
            * \param signature The Qt signal or slot signature.
            * \param arguments The optional \a List of arguments.
            * \return A QUObject array.
            */
            static QUObject* toQUObject(const QString& signature, List::Ptr arguments);

        private:
            /// The wrapped QObject.
            QObject* m_object;

            // QProperty's

            /// Return a list of property names.
            Kross::Api::Object::Ptr propertyNames(Kross::Api::List::Ptr);
            /// Return true if the property exists else false.
            Kross::Api::Object::Ptr hasProperty(Kross::Api::List::Ptr);
            /// Return a property.
            Kross::Api::Object::Ptr getProperty(Kross::Api::List::Ptr);
            /// Set a property.
            Kross::Api::Object::Ptr setProperty(Kross::Api::List::Ptr);

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

    };

}}

#endif

