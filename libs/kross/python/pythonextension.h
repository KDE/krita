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

#ifndef KROSS_PYTHON_EXTENSION_H
#define KROSS_PYTHON_EXTENSION_H

#include "pythonconfig.h"

#include "../api/object.h"
#include "../api/list.h"
#include "../api/dict.h"
#include "../api/class.h"

#include <qstring.h>
#include <qstringlist.h>
#include <q3valuelist.h>
#include <q3valuevector.h>
#include <qmap.h>
#include <qvariant.h>

namespace Kross { namespace Python {

    // Forward declaration.
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
            friend class PythonObject;
            friend class PythonModule;

        public:

            /**
             * Constructor.
             *
             * \param object The \a Kross::Api::Object object
             *        this instance is the wrapper for.
             */
            explicit PythonExtension(Kross::Api::Object::Ptr object);

            /**
             * Destructor.
             */
            virtual ~PythonExtension();

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
             * Overloaded method to handle attribute calls
             * from within python.
             *
             * \param name The name of the attribute that
             *        should be handled.
             * \return An \a Py::Object that could be
             *         a value or a callable object. Python
             *         will decide what to do with the
             *         returnvalue.
             */
            virtual Py::Object getattr(const char* name);

            //virtual Py::Object getattr_methods(const char* name);
            //virtual int setattr(const char* name, const Py::Object& value);

        private:

            /**
             * Converts a \a Py::Tuple into a \a Kross::Api::List .
             *
             * \param tuple The Py::Tuple to convert.
             * \return The to a Kross::Api::List converted Py::Tuple .
             */
            static Kross::Api::List::Ptr toObject(const Py::Tuple& tuple);

            /**
             * Converts a \a Py::List into a \a Kross::Api::List .
             *
             * \param list The Py::List to convert.
             * \return The to a Kross::Api::List converted Py::List .
             */
            static Kross::Api::List::Ptr toObject(const Py::List& list);

            /**
             * Converts a \a Py::Dict into a \a Kross::Api::Dict .
             *
             * \param dict The Py::Dict to convert.
             * \return The to a Kross::Api::Dict converted Py::Dict .
             */
            static Kross::Api::Dict::Ptr toObject(const Py::Dict& dict);

            /**
             * Converts a \a Py::Object into a \a Kross::Api::Object.
             *
             * \param object The Py::Object to convert.
             * \return The to a Kross::Api::Object converted Py::Object.
             */
            static Kross::Api::Object::Ptr toObject(const Py::Object& object);

            /**
             * Converts a QString to a Py::Object. If
             * the QString isNull() then Py::None() will
             * be returned.
             *
             * \param s The QString to convert.
             * \return The to a Py::String converted QString.
             */
            static const Py::Object toPyObject(const QString& s);

            /**
             * Converts a QStringList to a Py::List.
             *
             * \param list The QStringList to convert.
             * \return The to a Py::List converted QStringList.
             */
            static const Py::List toPyObject(const QStringList& list);

            /**
             * Converts a QMap to a Py::Dict.
             *
             * \param map The QMap to convert.
             * \return The to a Py::Dict converted QMap.
             */
            static const Py::Dict toPyObject(const QMap<QString, QVariant>& map);

            /**
             * Converts a QValueList to a Py::List.
             *
             * \param list The QValueList to convert.
             * \return The to a Py::List converted QValueList.
             */
            static const Py::List toPyObject(const Q3ValueList<QVariant>& list);

            /**
             * Converts a QVariant to a Py::Object.
             *
             * \param variant The QVariant to convert.
             * \return The to a Py::Object converted QVariant.
             */
            static const Py::Object toPyObject(const QVariant& variant);

            /**
             * Converts a \a Kross::Api::Object to a Py::Object.
             *
             * \param object The Kross::Api::Object to convert.
             * \return The to a Py::Object converted Kross::Api::Object.
             */
            static const Py::Object toPyObject(Kross::Api::Object::Ptr object);

            /**
             * Converts a \a Kross::Api::List into a Py::Tuple.
             *
             * \param list The Kross::Api::List to convert.
             * \return The to a Py::Tuple converted Kross::Api::List.
             */
            static const Py::Tuple toPyTuple(Kross::Api::List::Ptr list);

            /// The \a Kross::Api::Object this PythonExtension wraps.
            Kross::Api::Object::Ptr m_object;

            /**
             * The proxymethod which will handle all calls to our
             * \a PythonExtension instance.
             */
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

}}

#endif
