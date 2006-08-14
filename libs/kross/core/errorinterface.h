/***************************************************************************
 * errorinterface.h
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

#ifndef KROSS_ERRORINTERFACE_H
#define KROSS_ERRORINTERFACE_H

#include <QString>
#include <koffice_export.h>

#include "krossconfig.h"

namespace Kross {

    /**
     * Interface for error-handling.
     */
    class KROSS_EXPORT ErrorInterface
    {
        public:

            /**
             * Constructor.
             *
             * \param error The error message.
             * \param lineno The liner number in the scripting
             *        code where this exception got thrown.
             */
            ErrorInterface() {}

            /**
             * \return true if there was an error else false is returned.
             */
            bool hadError() const { return ! m_error.isNull(); }

            /**
             * \return the trace message.
             */
            const QString errorMessage() const { return m_trace; }

            /**
             * \return the error message.
             */
            const QString errorTrace() const { return m_error; }

            /**
             * \return the line number in the scripting code where the
             * exception got thrown or -1 if there was no line number defined.
             */
            long errorLineNo() const { return m_lineno; }

            /**
             * Set the error message.
             */
            void setError(const QString& errormessage, const QString& tracemessage = QString::null, long lineno = -1) {
                m_error = errormessage;
                m_trace = tracemessage;
                m_lineno = lineno;
                krosswarning( QString("Error error=%1 lineno=%2 trace=\n%3").arg(m_error).arg(m_lineno).arg(m_trace) );
            }

            /**
             * Set the error message.
             */
            void setError(ErrorInterface* error) {
                m_error = error->errorMessage();
                m_trace = error->errorTrace();
                m_lineno = error->errorLineNo();
            }

            /**
             * Clear the error.
             */
            void clearError() {
                m_error = QString::null;
                m_trace = QString::null;
                m_lineno = -1;
            }

        private:
            /// The error message.
            QString m_error;
            /// The trace message.
            QString m_trace;
            /// The line number where the exception got thrown
            long m_lineno;
    };

}

#endif

