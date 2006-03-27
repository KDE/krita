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

#include "../main/krossconfig.h"
#include "object.h"
#include "list.h"

#include <qstring.h>

namespace Kross { namespace Api {

    class Function
    {
        public:
            virtual Object::Ptr call(List::Ptr) = 0;
    };

    template<class INSTANCE>
    class ConstFunction0 : public Function
    {
        private:
            typedef Object::Ptr(INSTANCE::*Method)();
            INSTANCE* m_instance;
            Method m_method;
        public:
            ConstFunction0(INSTANCE* instance, Method method)
                : m_instance(instance), m_method(method) {}
            Object::Ptr call(List::Ptr)
                { return (m_instance->*m_method)(); }
    };

    template<class INSTANCE, typename P1>
    class ConstFunction1 : public Function
    {
        private:
            typedef Object::Ptr(INSTANCE::*Method)(P1);
            INSTANCE* m_instance;
            Method m_method;
            P1 m_p1;
        public:
            ConstFunction1(INSTANCE* instance, Method method, P1 p1)
                : m_instance(instance), m_method(method), m_p1(p1) {}
            Object::Ptr call(List::Ptr)
                { return (m_instance->*m_method)(m_p1); }
    };

    template<class INSTANCE, typename P1, typename P2>
    class ConstFunction2 : public Function
    {
        private:
            typedef Object::Ptr(INSTANCE::*Method)(P1, P2);
            INSTANCE* m_instance;
            Method m_method;
            P1 m_p1;
            P2 m_p2;
        public:
            ConstFunction2(INSTANCE* instance, Method method, P1 p1, P2 p2)
                : m_instance(instance), m_method(method), m_p1(p1), m_p2(p2) {}
            Object::Ptr call(List::Ptr)
                { return (m_instance->*m_method)(m_p1, m_p2); }
    };

    template<class INSTANCE>
    class VarFunction0 : public Function
    {
        private:
            typedef Object::Ptr(INSTANCE::*Method)(List::Ptr);
            INSTANCE* m_instance;
            Method m_method;
        public:
            VarFunction0(INSTANCE* instance, Method method)
                : m_instance(instance), m_method(method) {}
            Object::Ptr call(List::Ptr args)
                { return (m_instance->*m_method)(args); }
    };

    template<class INSTANCE, typename P1>
    class VarFunction1 : public Function
    {
        private:
            typedef Object::Ptr(INSTANCE::*Method)(List::Ptr, P1);
            INSTANCE* m_instance;
            Method m_method;
            P1 m_p1;
        public:
            VarFunction1(INSTANCE* instance, Method method, P1 p1)
                : m_instance(instance), m_method(method), m_p1(p1) {}
            Object::Ptr call(List::Ptr args)
                { return (m_instance->*m_method)(args, m_p1); }
    };

    template<class INSTANCE, typename P1, typename P2>
    class VarFunction2 : public Function
    {
        private:
            typedef Object::Ptr(INSTANCE::*Method)(List::Ptr, P1, P2);
            INSTANCE* m_instance;
            Method m_method;
            P1 m_p1;
            P2 m_p2;
        public:
            VarFunction2(INSTANCE* instance, Method method, P1 p1, P2 p2)
                : m_instance(instance), m_method(method), m_p1(p1), m_p2(p2) {}
            Object::Ptr call(List::Ptr args)
                { return (m_instance->*m_method)(args, m_p1, m_p2); }
    };

}}

#endif

