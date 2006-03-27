/***************************************************************************
 * exception.h
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

#ifndef KROSS_API_EXCEPTION_H
#define KROSS_API_EXCEPTION_H

#include "object.h"

#include <qstring.h>
#include <ksharedptr.h>

namespace Kross { namespace Api {

    /**
     * Common exception class used for representing exceptions
     * in Kross.
     *
     * Internal we use \a Exception instances to throw and handle
     * exceptions. Those exceptions are inherited from \a Object
     * and therefore they are first class citizens in Kross.
     */
    class Exception : public Object
    {
        private:

            /// The error message.
            QString m_error;

            /// The trace message.
            QString m_trace;

            /// The line number where the exception got thrown
            long m_lineno;

        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<Exception> Ptr;

            /**
             * Constructor.
             *
             * \param error The error message.
             * \param lineno The liner number in the scripting
             *        code where this exception got thrown.
             * \param parent The parent \a Object or NULL if
             *        this exception object doesn't has a
             *        parent.
             */
            Exception(const QString& error, long lineno = -1, Object::Ptr parent = 0);

            /**
             * Destructor.
             */
            virtual ~Exception();

            /// \see Kross::Api::Object::getClassName()
            virtual const QString getClassName() const;

            /// \see Kross::Api::Object::toString()
            virtual const QString toString();

            /**
             * \return the error message.
             */
            const QString& getError() const;

            /**
             * \return a more detailed tracemessage or QString::null if
             * there is no trace avaiable.
             */
            const QString& getTrace() const;

            /**
             * Set a more detailed tracemessage.
             */
            void setTrace(const QString& tracemessage);

            /**
             * \return the line number in the scripting code
             * where the exception got thrown or -1 if there
             * was no line number defined.
             */
            long getLineNo() const;
    };

}}

#endif

