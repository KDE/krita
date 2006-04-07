/***************************************************************************
 * module.h
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

#ifndef KROSS_API_MODULE_H
#define KROSS_API_MODULE_H

#include <qstring.h>

#include "class.h"

namespace Kross { namespace Api {

    /**
     * The Module class. Modules are managed in the \a Manager singleton 
     * instance and are implemented as in scripting plugins as main
     * entry point to load and work with them.
     */
    class Module : public Class<Module>
    {
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<Module> Ptr;

            /**
             * Constructor.
             *
             * \param name The name of this module.
             *        Each module needs a unique name cause
             *        the application using Kross identifies
             *        modules with there names.
             */
            explicit Module(const QString& name)
                : Class<Module>(name)
            {
                krossdebug( QString("Kross::Api::Module %1 created").arg(name) );
            }

            /**
             * Destructor.
             */
            virtual ~Module()
            {
                krossdebug( QString("Kross::Api::Module %1 destroyed").arg(getName()) );
            }

            /**
             * Method to load from \a Kross::Api::Object inherited classes
             * this module implements from within other modules.
             */
            virtual Object::Ptr get(const QString& /*name*/, void* /*pointer*/ = 0)
            {
                return Object::Ptr();
            }

    };


}}

#endif

