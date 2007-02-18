/***************************************************************************
 * rubyinterpreter.cpp
 * This file is part of the KDE project
 * copyright (C)2005 by Cyrille Berger (cberger@cberger.net)
 * copyright (C)2006 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_RUBYRUBYEXTENSION_H
#define KROSS_RUBYRUBYEXTENSION_H

#include <ruby.h>

#include "rubyconfig.h"

#include <QStringList>
#include <QVariant>
#include <QObject>

namespace Kross {

    class RubyExtensionPrivate;

    /**
    * This class wraps a QObject into the world of ruby.
    *
    * @author Cyrille Berger
    */
    class RubyExtension {
            friend class RubyInterpreter;
            friend class RubyModule;
            friend class RubyScript;
        public:

            /**
            * Constructor.
            *
            * @param object The QObject instance this extension provides access to.
            */
            explicit RubyExtension(QObject* object);

            /**
            * Destructor.
            */
            ~RubyExtension();

            /**
            * \return the QObject this \a RubyExtension wraps.
            */
            QObject* object() const;

        private:

            /// Connect was called.
            //VALUE callConnect(int argc, VALUE *argv, VALUE self);
            /// Disconnect was called.
            //VALUE callDisconnect(int argc, VALUE *argv, VALUE self);

            /**
            * Handle the function call.
            */
            VALUE callMetaMethod(const QByteArray& funcname, int argc, VALUE *argv, VALUE self);

            /**
            * This function will catch functions that are undefined, extracts
            * the matching @a RubyExtension instance and redirects the call
            * to @a call_method_missing .
            * 
            */
            static VALUE method_missing(int argc, VALUE *argv, VALUE self);

            /**
            * This function will call a function in a \a RubyExtension object.
            *
            * @param extension the \a RubyExtension object which contains the function
            * @param argc the number of argument
            * @param argv the lists of arguments (the first argument is the Ruby ID of the function)
            */
            static VALUE call_method_missing(RubyExtension* extension, int argc, VALUE *argv, VALUE self);

#if 0
            /**
            * This function handles "each" calls in a \a RubyExtension object to
            * provide access to children QObject's the wrapped QObject instance has.
            */
            static VALUE call_each(int argc, VALUE *argv, VALUE self);
#endif

            /**
            * This function is called by ruby to delete a RubyExtension object.
            */
            static void delete_object(void* object);

#if 0
            /**
            * This function is called by ruby to delete a RubyException object.
            */
            static void delete_exception(void* object);
#endif

        public:

            /**
            * Test if the ruby object is a \a RubyExtension object.
            */
            static bool isRubyExtension(VALUE obj);

#if 0
            /**
            * Test if the ruby object is an exception.
            */
            static bool isOfExceptionType(VALUE obj);

            /**
            * Convert a ruby object to the exception type.
            * @return 0 if the object wasn't an exception.
            */
            static Kross::Exception* convertToException(VALUE obj);
            /**
            * Wrap an exception in a ruby object.
            */
            static VALUE convertFromException(Kross::Exception::Ptr exc);
#endif

            /**
            * Converts a \a RubyExtension to a VALUE.
            * \param object The RubyExtension to convert.
            * \return The to a VALUE converted RubyExtension.
            */
            static VALUE toVALUE(RubyExtension* object);

        private:
            /// @internal private d-pointer.
            RubyExtensionPrivate * const d;
            /// @internal unwanted copy-ctor.
            RubyExtension(const RubyExtension&);
    };

}

#endif
