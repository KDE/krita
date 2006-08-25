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

#ifndef KROSS_RUBYRUBYMODULE_H
#define KROSS_RUBYRUBYMODULE_H

#include <ruby.h>

#include <QString>
#include <QObject>

#include "../core/krossconfig.h"
//#include "../core/object.h"
//#include "../core/module.h"

namespace Kross {

    class RubyModulePrivate;

    /**
    * A ruby module.
    * @author Cyrille Berger
    */
    class RubyModule {
        public:

            /**
            * Constructor.
            *
            * @param mod The \a Kross::Module this RubyExtension
            *        wraps.
            * @param modname The name the module will be published as.
            */
            RubyModule(QObject* object, QString modname);

            /**
            * Destructor.
            */
            ~RubyModule();

        private:

            /**
            * This function will catch functions that are undefined.
            */
            static VALUE method_missing(int argc, VALUE *argv, VALUE self);

        private:
            /// Private d-pointer.
            RubyModulePrivate* d;
            /// Unwanted copy-ctor.
            RubyModule(const RubyModule&) {}
    };

}

#endif
