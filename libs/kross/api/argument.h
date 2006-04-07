/***************************************************************************
 * argument.h
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

#ifndef KROSS_API_ARGUMENT_H
#define KROSS_API_ARGUMENT_H

#include <qstring.h>
#include <q3valuelist.h>

#include "object.h"

namespace Kross { namespace Api {

    // Forward declaration.
    class ArgumentList;

    /**
     * Each \a Class::Function holds a list of arguments
     * the function supports. The Argument-class represents
     * such a single argument in a \a ArgumentList collection.
     */
    class Argument
    {
        public:

            /**
             * Constructor.
             *
             * \param classname The name of the class this
             *        argument expects.
             */
            explicit Argument(const QString& classname = QString::null /*, Object::Ptr object = 0, bool visible = true*/ );

            /**
             * Destructor.
             */
            ~Argument();

            /**
             * Return the name of the class this argument expects.
             *
             * \return Name of the class this argument expects.
             */
            const QString getClassName();

            /**
             * Return the optional default \a Object this class
             * holds.
             *
             * \return The default object or NULL if the argument
             *         isn't optional and therefore doesn't have
             *         an default object.
             */
            //Object::Ptr getObject() const;

            /**
             * \return true if the \a Argument should be hidden
             * to the scripting backend.
             */
            //bool isVisible() const;

            /**
             * Implementation of the << operator.
             *
             * \param arglist The \a ArgumentList the
             *        operator is applied on.
             * \return The changed \a ArgumentList.
	     *
	     * \todo Document where this argument is added in the list
	     *       and add a brief code example.
             */
            ArgumentList& operator << (ArgumentList& arglist);

        private:
            /// The classname of the argument.
            QString m_classname;
            /// The optional default \a Object this argument holds.
            //Object::Ptr m_object;
            /// Defines if the argument is visible to the user.
            //bool m_visible;
    };

    /**
     * An ArgumentList is a collection of \a Argument
     * objects used in a \a Class::Function.
     */
    class ArgumentList
    {
        public:

            /**
             * Constructor.
             */
            ArgumentList();

            /**
             * Destructor.
             */
            ~ArgumentList();

            /**
             * Operator to return the list of arguments call-by-reference
             * to let code like the \a Callable::checkArguments method
             * enable manipulate those list.
             *
             * \return List of \a Argument instances.
             */
            operator Q3ValueList<Argument>& () { return m_arguments; }

            /**
             * Implementation of the << operator.
             *
             * \param arg The passed \a Argument.
             * \return The changed \a ArgumentList.
             */
            ArgumentList& operator << (const Argument& arg);

            /**
             * Return number of minimal needed parameters.
             *
             * \return Minimal needed parameters.
             */
            //uint getMinParams() const;

            /**
             * Return the number of maximal allowed parameters.
             *
             * \return Maximal needed parameters.
             */
            //uint getMaxParams() const;

            /**
             * \return a string of the classnames each argument in the
             * argumentlist has. Mainly used foir debugging.
             */
            //QString toString();

        private:
            /// Minimal needed parameters.
            //uint m_minparams;
            /// Maximal needed parameters.
            //uint m_maxparams;
            /// List of \a Argument.
            Q3ValueList<Argument> m_arguments;
    };

}}

#endif

