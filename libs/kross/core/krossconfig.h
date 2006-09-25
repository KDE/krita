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

#include <QString>
#include <koffice_export.h>

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

    #define KROSS_OBJECT_METACALL_DEBUG
    //#define KROSS_METATYPE_DEBUG
    #define KROSS_INTERPRETER_DEBUG

    // The name of the interpreter's library. Those library got loaded
    // dynamically during runtime. Comment out to disable compiling of
    // the interpreter-plugin or to hardcode the location of the lib
    // like I did at the following line.

    //#define KROSS_PYTHON_LIBRARY "/home/kde4/koffice/_build/lib/krosspython.la"
    #define KROSS_PYTHON_LIBRARY "krosspython"
    #define KROSS_RUBY_LIBRARY "krossruby"
    #define KROSS_KJS_LIBRARY "krosskjs"

}

#endif

