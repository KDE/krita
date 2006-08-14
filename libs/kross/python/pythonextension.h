/***************************************************************************
 * pythonextension.h
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

#ifndef KROSS_PYTHONEXTENSION_H
#define KROSS_PYTHONEXTENSION_H

#include "pythonconfig.h"

#include "../core/krossconfig.h"
#include "../core/object.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QVariant>
#include <QPointer>

namespace Kross {

    // Forward declaration.
    class PythonVariant;
    class PythonScript;

    /**
     * The PythonExtension is a wrapper-object to let C++ and
     * Python interact together.
     * Instances of this class are used everytime if we send
     * or got something to/from python.
     */
    class PythonExtension : public Py::PythonExtension<PythonExtension>
    {
            friend class PythonScript;
            //friend class PythonModule;
            //friend class PythonVariant;
        public:

            /**
             * Constructor.
             *
             * \param object The QObject this extension instance wraps.
             */
            explicit PythonExtension(QObject* object);

            /**
             * Destructor.
             */
            virtual ~PythonExtension();

            /**
             * \return the QObject instance this extension instance wraps.
             */
            QObject* object() const;

            /**
             * Overloaded method to return the string-representation
             * of this object.
             *
             * \return The string representation.
             */
            virtual Py::Object str();

            /**
             * Overloaded method to return the string-representation
             * of the value this object has.
             *
             * \return A string representation of the value.
             */
            virtual Py::Object repr();

            /**
             * Overloaded method to handle attribute calls from within python.
             *
             * \param name The name of the attribute that should be handled.
             * \return An \a Py::Object that could be a value or a callable
             * object. Python will decide what to do with the returnvalue.
             */
            virtual Py::Object getattr(const char* name);
            //virtual Py::Object getattr_methods(const char* name);
            //virtual int setattr(const char* name, const Py::Object& value);

        private:

            /// The QObject this PythonExtension wraps.
            QPointer<QObject> m_object;
            /// \internal string for debugging.
            QString m_debuginfo;
            /// The cached list of methods.
            QHash<QByteArray, int>* m_methods;
            /// Update the cached list of methods.
            Py::List updateMethods();
            /// The proxymethod which will handle all calls to our \a PythonExtension instance.
            Py::MethodDefExt<PythonExtension>* m_proxymethod;

            /**
             * The static proxy-handler which will be used to dispatch
             * a call to our \a PythonExtension instance and redirect
             * the call to the matching method.
             *
             * \param _self_and_name_tuple A tuple containing as first
             *        argument a reference to our \a PythonExtension
             *        instance.
             * \param _args The optional passed arguments for the method
             *        which should be called.
             * \return The returnvalue of the methodcall.
             */
            static PyObject* proxyhandler(PyObject* _self_and_name_tuple, PyObject* _args);
    };

}

#endif
