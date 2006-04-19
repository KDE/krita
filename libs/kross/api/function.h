/***************************************************************************
 * function.h
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

#ifndef KROSS_API_FUNCTION_H
#define KROSS_API_FUNCTION_H

#include "krossconfig.h"
#include "object.h"
#include "list.h"

#include <qstring.h>

namespace Kross { namespace Api {

    /**
     * The base class for functions. Classes like \a Function0 and
     * \a ProxyFunction inheritate this class.
     */
    class Function
    {
        public:

            /**
             * Each function needs to implement the call-method which will
             * be executed if the function itself should be executed.
             */
            virtual Object::Ptr call(List::Ptr) = 0;

            /**
             * Destructor.
             */
            virtual ~Function() {}
    };

    /**
     * This class implements the most abstract way to work with functions. It
     * implements pointing to functions of the form
     * @code
     * Kross::Api::Object::Ptr myfunc(Kross::Api::List::Ptr)
     * @endcode
     * where a low-level \a Object got returned that represents the returnvalue
     * of the function-call, and a \a List instance is passed that may contain
     * optional \a Object instances as parameters.
     */
    template<class INSTANCE>
    class Function0 : public Function
    {
        private:
            typedef Object::Ptr(INSTANCE::*Method)(List::Ptr);
            INSTANCE* m_instance;
            Method m_method;
        public:
            Function0(INSTANCE* instance, Method method)
                : m_instance(instance), m_method(method) {}
            Object::Ptr call(List::Ptr args)
                { return (m_instance->*m_method)(args); }
    };

    /**
     * Specialization of the \a Function0 which takes as additional parameter
     * a const-value. This const-value will be hidden for the scripting backend
     * and is only passed through on function-call.
     *
     * So, this class could be as example used to point to a function like;
     * @code
     * Kross::Api::Object::Ptr myfunc(Kross::Api::List::Ptr, int myinteger)
     * @endcode
     * and then we are able to point to the function with something like
     * @code
     * this->addFunction("myfunctionname",
     *     new Kross::Api::Function1< MYCLASS, int >(
     *         this, // pointer to an instance of MYCLASS
     *         &MYCLASS::myfunction, // the method which should be wrapped
     *         17 // the const-value we like to pass to the function.
     *         ) );
     * @endcode
     * The defined integer myinteger which has the value 17 will be passed
     * transparently to myfunc. The scripting-backend won't know that there
     * exists such an additional integer at all. So, it's hidden and the user
     * aka the scripting code won't be able to manipulate that additional
     * value.
     */
    template<class INSTANCE, typename P1>
    class Function1 : public Function
    {
        private:
            typedef Object::Ptr(INSTANCE::*Method)(List::Ptr, P1);
            INSTANCE* m_instance;
            Method m_method;
            P1 m_p1;
        public:
            Function1(INSTANCE* instance, Method method, P1 p1)
                : m_instance(instance), m_method(method), m_p1(p1) {}
            Object::Ptr call(List::Ptr args)
                { return (m_instance->*m_method)(args, m_p1); }
    };

    /**
     * Same as \a Function1 but with 2 additional parameters.
     */
    template<class INSTANCE, typename P1, typename P2>
    class Function2 : public Function
    {
        private:
            typedef Object::Ptr(INSTANCE::*Method)(List::Ptr, P1, P2);
            INSTANCE* m_instance;
            Method m_method;
            P1 m_p1;
            P2 m_p2;
        public:
            Function2(INSTANCE* instance, Method method, P1 p1, P2 p2)
                : m_instance(instance), m_method(method), m_p1(p1), m_p2(p2) {}
            Object::Ptr call(List::Ptr args)
                { return (m_instance->*m_method)(args, m_p1, m_p2); }
    };

}}

#endif

