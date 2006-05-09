/***************************************************************************
 * krossconfig.h
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

#ifndef KROSS_MAIN_KROSSCONFIG_H
#define KROSS_MAIN_KROSSCONFIG_H

#include <qstring.h>
#include <koffice_export.h>
/**
 * The Kross scripting bridge to embed scripting functionality
 * into an application.
 *
 * - abstract API to access the scripting functionality.
 * - interpreter independend to be able to decide on runtime
 *   if we like to use the python, kjs (KDE JavaScript) or
 *   whatever scripting interpreter.
 * - flexibility by beeing able to connect different
 *   scripting interpreters together into something like
 *   a "working chain" (e.g. python-script script1 spends
 *   some functionality the kjs-script script2 likes to
 *   use.
 * - transparently bridge functionality wrappers like
 *   \a Kross::KexiDB together with interpreters like \a Kross::Python.
 * - Introspection where needed to be able to manipulate
 *   behaviours and functionality on runtime.
 * - Qt/KDE based, so use the extended techs both spends.
 * - integrate nicly as powerfull scripting system into the
 *   Kexi application.
 *
 * \author Sebastian Sauer
 * \sa http://www.koffice.org/kexi
 * \sa http://www.dipe.org/kross
 */
namespace Kross {

    /// Debugging enabled.
    #define KROSS_DEBUG_ENABLED

    #ifdef KROSS_DEBUG_ENABLED

        /**
         * Debugging function.
         */
        KROSS_EXPORT void krossdebug(const QString &s);

        /**
         * Warning function.
         */
        KROSS_EXPORT void krosswarning(const QString &s);

    #else
        // Define these to an empty statement if debugging is disabled.
        #define krossdebug(x)
        #define krosswarning(x)
    #endif

    /**
     * The common Kross API used as common codebase.
     *
     * The API spends \a Kross::Api::Object and more specialized
     * classes to bridge other Kross parts together. Interaction
     * between objects got wrapped at runtime and introspection-
     * functionality enables dynamic manipulations.
     * The proxy functionality prevents cross-dependencies
     * between Kross parts like the \a Kross::Python implementation
     * and the \a Kross::KexiDB wrapper.
     *
     * \author Sebastian Sauer
     */
    namespace Api {

        //#define KROSS_API_OBJECT_CTOR_DEBUG
        //#define KROSS_API_OBJECT_DTOR_DEBUG
        //#define KROSS_API_OBJECT_ADDCHILD_DEBUG
        //#define KROSS_API_OBJECT_REMCHILD_DEBUG
        //#define KROSS_API_OBJECT_CALL_DEBUG

        //#define KROSS_API_EVENT_CALL_DEBUG

        //#define KROSS_API_CALLABLE_CALL_DEBUG
        //#define KROSS_API_CALLABLE_CHECKARG_DEBUG

        //#define KROSS_API_EVENTSLOT_CALL_DEBUG
        //#define KROSS_API_EVENTSIGNAL_CALL_DEBUG

        // The name of the interpreter's library. Those library got loaded
        // dynamicly during runtime. Comment out to disable compiling of
        // the interpreter-plugin or to hardcode the location of the lib
        // like I did at the following line.

        //#define KROSS_PYTHON_LIBRARY "/home/snoopy/cvs/kde/trunk/koffice/lib/kross/python/krosspython.la"
        #define KROSS_PYTHON_LIBRARY "krosspython"
        #define KROSS_RUBY_LIBRARY "krossruby"

    }

}

#endif

