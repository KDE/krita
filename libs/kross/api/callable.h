/***************************************************************************
 * callable.h
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

#ifndef KROSS_API_CALLABLE_H
#define KROSS_API_CALLABLE_H

#include "object.h"
#include "list.h"
//#include "exception.h"
#include "argument.h"
#include <koffice_export.h>
#include <qstring.h>
#include <q3valuelist.h>
#include <ksharedptr.h>

namespace Kross { namespace Api {

    /**
     * Base class for callable objects. Classes like
     * \a Event or \a Class are inherited from this class
     * and implement the \a Object::call() method to handle
     * the call.
     */
    class KROSS_EXPORT Callable : public Object
    {
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<Callable> Ptr;

            /**
             * Constructor.
             *
             * \param name The name this callable object has and
             *       it is reachable as via \a Object::getChild() .
             * \param parent The parent \a Object this instance is
             *       child of.
             * \param arglist A list of arguments the callable
             *       object expects if it got called.
             */
            Callable(const QString& name, Object* parent, const ArgumentList& arglist);

            /**
             * Destructor.
             */
            virtual ~Callable();

            /**
             * Return the class name. This could be something
             * like "Kross::Api::Callable" for this object. The
             * value is mainly used for display purposes.
             *
             * \return The name of this class.
             */
            virtual const QString getClassName() const;

            /**
             * Call the object.
             */
            virtual Object::Ptr call(const QString& name, List::Ptr arguments);

            /**
             * Wrapper for the \a Kross::Api::Object::hasChild() method
             * to check if this object has children.
             */
            Object::Ptr hasChild(List::Ptr args);

            /**
             * Wrapper for the \a Kross::Api::Object::getChild() method
             * to return a children this object has.
             */
            Object::Ptr getChild(List::Ptr args);

            //Object::Ptr setChild(List::Ptr args);

            /**
             * Wrapper for the \a Kross::Api::Object::getChildren() method
             * to return a list of childrennames this object has.
             *
             * \return a \a List filled with a list of names of the
             *        children this object has.
             */
            Object::Ptr getChildrenList(List::Ptr args);

            /**
             * Wrapper for the \a Kross::Api::Object::getChild() method
             * to return a dictonary of children this object has.
             *
             * \return a \a Dict filled with the children.
             */
            Object::Ptr getChildrenDict(List::Ptr args);

            /**
             * Wrapper for the \a Kross::Api::Object::call() method
             * to call a children.
             */
            Object::Ptr callChild(List::Ptr args);

        protected:

            /**
             * List of arguments this callable object supports.
             */
            ArgumentList m_arglist;

            /**
             * Check the passed arguments against the \a m_arglist and throws 
             * an exception if failed.
             */
            //void checkArguments(KSharedPtr<List> arguments);

    };

}}

#endif

