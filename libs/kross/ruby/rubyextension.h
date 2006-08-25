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
            RubyExtension(QObject* object);

            /**
            * Destructor.
            */
            ~RubyExtension();

            /**
            * \return the QObject this \a RubyExtension wraps.
            */
            QObject* object() const;

        private:

            /**
            * This function will catch functions that are undefined.
            */
            static VALUE method_missing(int argc, VALUE *argv, VALUE self);

            /**
            * This function will call a function in a \a RubyExtension object
            * @param extension the \a RubyExtension object which contains the function
            * @param argc the number of argument
            * @param argv the lists of arguments (the first argument is the Ruby ID of the function)
            */
            static VALUE call_method(RubyExtension* extension, int argc, VALUE *argv);

            /**
            * This function is called by ruby to delete a RubyExtension object
            */
            static void delete_object(void* object);

#if 0
            /**
            * This function is called by ruby to delete a RubyExtension object
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

#if 0
            /**
            * This function iterats through a ruby hash
            */
            static int convertHash_i(VALUE key, VALUE value, VALUE vmap);

            /**
            * Converts a \a VALUE into a QVariant.
            * \param value The ruby VALUE to convert.
            * \return The to a QVariant converted Ruby VALUE.
            */
            static QVariant toVariant(VALUE value);

            /**
            * Converts a QString to a VALUE. If
            * the QString isNull() then a "" will
            * be returned.
            * \param s The QString to convert.
            * \return The to a VALUE converted QString.
            */
            static VALUE toVALUE(const QString& s);

            /**
            * Converts a QStringList to a VALUE.
            * \param list The QStringList to convert.
            * \return The to a VALUE converted QStringList.
            */
            static VALUE toVALUE(QStringList list);

            /**
            * Converts a QMap to a VALUE.
            * \param map The QMap to convert.
            * \return The to a VALUE converted QMap.
            */
            static VALUE toVALUE(QVariantMap map);

            /**
            * Converts a QList to a VALUE.
            * \param list The QValueList to convert.
            * \return The to a VALUE converted QValueList.
            */
            static VALUE toVALUE(QVariantList list);

            /**
            * Converts a QVariant to a VALUE.
            * \param variant The QVariant to convert.
            * \return The to a VALUE converted QVariant.
            */
            static VALUE toVALUE(const QVariant& variant);
#endif

#if 0
            /**
            * Converts a QObject to a VALUE.
            * \param object The QObject to convert.
            * \return The to a VALUE converted QObject.
            */
            static VALUE toVALUE(QObject* object);
#endif

            /**
            * Converts a \a RubyExtension to a VALUE.
            * \param object The RubyExtension to convert.
            * \return The to a VALUE converted RubyExtension.
            */
            static VALUE toVALUE(RubyExtension* object);

        private:
            /// Private d-pointer.
            RubyExtensionPrivate* d;
            /// Unwanted copy-ctor.
            RubyExtension(const RubyExtension&) {}
    };

}

#endif
