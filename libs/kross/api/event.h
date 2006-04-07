/***************************************************************************
 * event.h
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

#ifndef KROSS_API_EVENT_H
#define KROSS_API_EVENT_H

#include "../main/krossconfig.h"
#include "object.h"
#include "argument.h"
#include "callable.h"
#include "list.h"
#include "exception.h"
#include "function.h"
#include "proxy.h"
#include "variant.h"

#include <qstring.h>
#include <q3valuelist.h>
#include <qmap.h>

namespace Kross { namespace Api {

    /**
     * Template class for all kinds of callable events. An
     * event is the abstract base for callable objects like
     * methodfunctions in \a Class instances or \a EventSlot
     * and \a EventSignal to access Qt signals and slots.
     */
    template<class T>
    class Event : public Callable
    {
        private:

            /**
             * Definition of function-pointers.
             */
            typedef Object::Ptr(T::*FunctionPtr)(List::Ptr);

            /**
             * List of memberfunctions. Each function is accessible
             * by the functionname.
             */
            QMap<QString, Function* > m_functions;

        public:

            /**
             * Constructor.
             *
             * \param name The name this \a Event has.
             * \param parent The \a Object that this \a Event is
             *        child of.
             */
            Event(const QString& name, Object* parent)
                : Callable(name, parent, ArgumentList())
            {
            }

            /**
             * Destructor.
             */
            virtual ~Event()
            {
                QMapIterator<QString, Function* > it(m_functions);
                while(it.hasNext())
                    delete it.next().value();
            }

            /**
             * Add a \a Callable methodfunction to the list of functions
             * this Object supports.
             *
             * The FunctionPtr points to the concret
             * Object::Ptr myfuncname(List::Ptr)
             * method in the class defined with template T.
             *
             * \param name The functionname. Each function this object
             *        holds should have an unique name to be
             *        still accessable.
             * \param function A pointer to the methodfunction that
             *        should handle calls.
             * \param arglist A list of arguments for the function.
             *
             * \todo Remove this method as soon as there is no code using it
             */
//TODO remove Argument+ArgumentList. They are replaced with ProxyFunction now.
            void addFunction(const QString& name, FunctionPtr function, const ArgumentList& arglist = ArgumentList())
            {
                Q_UNUSED(arglist);
                m_functions.replace(name, new Function0<T>(static_cast<T*>(this), function));
            }

            /**
             * Add a methodfunction to the list of functions this Object
             * supports.
             *
             * \param name The functionname. Each function this object
             *        holds should have an unique name to be
             *        still accessable.
             * \param function A \a Function instance which defines
             *        the methodfunction. This \a Event will be the
             *        owner of the \a Function instance and will take
             *        care of deleting it if this \a Event got deleted.
             */
            void addFunction(const QString& name, Function* function)
            {
                m_functions.replace(name, function);
            }

            /**
             * Template function to add a \a ProxyFunction as builtin-function
             * to this \a Event instance.
             */
            template<class RETURNOBJ, class ARG1OBJ, class ARG2OBJ, class ARG3OBJ, class ARG4OBJ, class INSTANCE, typename METHOD>
            inline void addProxyFunction(const QString& name, INSTANCE* instance, METHOD method, ARG1OBJ* arg1 = 0, ARG2OBJ* arg2 = 0, ARG3OBJ* arg3 = 0, ARG4OBJ* arg4 = 0)
            {
                m_functions.replace(name,
                    new Kross::Api::ProxyFunction<INSTANCE, METHOD, RETURNOBJ, ARG1OBJ, ARG2OBJ, ARG3OBJ, ARG4OBJ>
                        (instance, method, arg1, arg2, arg3, arg4)
                );
            }

            /// Same as above with three arguments.
            template<class RETURNOBJ, class ARG1OBJ, class ARG2OBJ, class ARG3OBJ, class INSTANCE, typename METHOD>
            inline void addProxyFunction(const QString& name, INSTANCE* instance, METHOD method, ARG1OBJ* arg1 = 0, ARG2OBJ* arg2 = 0, ARG3OBJ* arg3 = 0)
            {
                m_functions.replace(name,
                    new Kross::Api::ProxyFunction<INSTANCE, METHOD, RETURNOBJ, ARG1OBJ, ARG2OBJ, ARG3OBJ>
                        (instance, method, arg1, arg2, arg3)
                );
            }

            /// Same as above with two arguments.
            template<class RETURNOBJ, class ARG1OBJ, class ARG2OBJ, class INSTANCE, typename METHOD>
            inline void addProxyFunction(const QString& name, INSTANCE* instance, METHOD method, ARG1OBJ* arg1 = 0, ARG2OBJ* arg2 = 0)
            {
                m_functions.replace(name,
                    new Kross::Api::ProxyFunction<INSTANCE, METHOD, RETURNOBJ, ARG1OBJ, ARG2OBJ>
                        (instance, method, arg1, arg2)
                );
            }

            /// Same as above, but with one argument.
            template<class RETURNOBJ, class ARG1OBJ, class INSTANCE, typename METHOD>
            inline void addProxyFunction(const QString& name, INSTANCE* instance, METHOD method, ARG1OBJ* arg1 = 0)
            {
                m_functions.replace(name,
                    new Kross::Api::ProxyFunction<INSTANCE, METHOD, RETURNOBJ, ARG1OBJ>
                        (instance, method, arg1)
                );
            }

            /// Same as above with no arguments.
            template<class RETURNOBJ, class INSTANCE, typename METHOD>
            inline void addProxyFunction(const QString& name, INSTANCE* instance, METHOD method)
            {
                m_functions.replace(name,
                    new Kross::Api::ProxyFunction<INSTANCE, METHOD, RETURNOBJ>
                        (instance, method)
                );
            }

           /**
            * Check if a function is a member of this \a Callable
            * \param name the function name
            * \return true if the function is available in this \a Callable
            */
            bool isAFunction(const QString & name) const
            {
                return m_functions.contains(name);
            }

            /**
             * Overloaded method to handle function-calls.
             *
             * \throw AttributeException if argumentparameters
             *        arn't valid.
             * \throw RuntimeException if the functionname isn't
             *        valid.
             * \param name The functionname. Each function this
             *        Object holds should have a different
             *        name cause they are access by they name.
             *        If name is QString::null or empty, a
             *        self-reference to this instance is
             *        returned.
             * \param arguments The list of arguments.
             * \return An Object representing the call result
             *         or NULL if there doesn't exists such a
             *         function with defined name.
             */
            virtual Object::Ptr call(const QString& name, List::Ptr arguments)
            {
#ifdef KROSS_API_EVENT_CALL_DEBUG
                krossdebug( QString("Event::call() name='%1' getName()='%2'").arg(name).arg(getName()) );
#endif

                Function* function = m_functions[name];
                if(function) {
#ifdef KROSS_API_EVENT_CALL_DEBUG
                    krossdebug( QString("Event::call() name='%1' is a builtin function.").arg(name) );
#endif
                    return function->call(arguments);
                }

                if(name.isNull()) {
                    // If no name is defined, we return a reference to our instance.
                    return Object::Ptr( this );
                }

                // Redirect the call to the Kross::Api::Callable we are inherited from.
                return Callable::call(name, arguments);
            }

    };

}}

#endif

