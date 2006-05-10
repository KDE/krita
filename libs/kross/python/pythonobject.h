/***************************************************************************
 * pythonobject.h
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

#ifndef KROSS_PYTHON_OBJECT_H
#define KROSS_PYTHON_OBJECT_H

#include "pythonconfig.h"
#include "../api/object.h"
#include "../api/list.h"
#include "pythonextension.h"

#include <QString>
#include <qstringlist.h>

namespace Kross { namespace Python {

    /**
     * The PythonObject class is used for Instances of Python
     * Classes by the \a PythonExtension class.
     */
    class PythonObject : public Kross::Api::Object
    {
        public:

            /**
             * Constructor.
             *
             * \param object The Py::Object this \a PythonObject
             *        provides access to.
             */
            explicit PythonObject(const Py::Object& object);

            /**
             * Destructor.
             */
            virtual ~PythonObject();

            /**
             * Return the class name. This could be something
             * like "Kross::Python::PythonObject" for this
             * object. The value is mainly used for display
             * purposes.
             *
             * \return The name of this class.
             */
            virtual const QString getClassName() const;

            /**
             * Pass a call to the object. Objects like \a Class
             * are able to handle call's by just implementating
             * this function.
             *
             * \throws TypeException if the object or the name
             *         is not callable.
             * \param name Each call has a name that says what
             *        should be called. In the case of a \a Class
             *        the name is the functionname.
             * \param arguments The list of arguments passed to
             *        the call.
             * \return The call-result as Object* instance or
             *         NULL if the call has no result.
             */
            virtual Kross::Api::Object::Ptr call(const QString& name, Kross::Api::List::Ptr arguments);

            /**
             * Return a list of supported callable objects.
             *
             * \return List of supported calls.
             */
            virtual QStringList getCalls();

        private:
            const Py::Object m_pyobject;
            QStringList m_calls;
    };

}}

#endif
