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
#include <koffice_export.h>
#include <QString>
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
             */
            Callable(const QString& name);

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
             * Returns if the defined child is avaible.
             *
             * \return true if child exists else false.
             */
            bool hasChild(const QString& name) const;

            /**
             * Return the defined child or NULL if there is
             * no such object with that name avaible.
             *
             * \param name The name of the Object to return.
             * \return The Object matching to the defined
             *         name or NULL if there is no such Object.
             */
            Object::Ptr getChild(const QString& name) const;

            /**
             * Return all children.
             *
             * \return A \a ObjectMap of children this Object has.
             */
            QMap<QString, Object::Ptr> getChildren() const;

            /**
             * Add a new child. Replaces a possible already existing
             * child with such a name.
             *
             * \param name the name of the child
             * \param object The Object to add.
             * \return true if the Object was added successfully
             *         else false.
             */
            bool addChild(Object* object, const QString& name = QString::null);

            /**
             * Remove an existing child.
             *
             * \param name The name of the Object to remove.
             *        If there doesn't exists an Object with
             *        such name just nothing will be done.
             */
            void removeChild(const QString& name);

            /**
             * Remove all children.
             */
            void removeAllChildren();

            /**
             * Call the object.
             */
            virtual Object::Ptr call(const QString& name, List::Ptr arguments);

        private:
            /// A list of childobjects.
            QMap<QString, Object::Ptr> m_children;
    };

}}

#endif

