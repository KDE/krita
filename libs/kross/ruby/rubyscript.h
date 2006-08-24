/***************************************************************************
 * rubyscript.h
 * This file is part of the KDE project
 * copyright (C)2005 by Cyrille Berger (cberger@cberger.net)
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

#ifndef KROSS_RUBYRUBYSCRIPT_H
#define KROSS_RUBYRUBYSCRIPT_H

#include "../core/krossconfig.h"
#include "../core/script.h"

namespace Kross {

class RubyScriptPrivate;

/**
 * Handle ruby scripts. This class implements
 * \a Kross::Script for ruby.
 * @author Cyrille Berger
 */
class RubyScript : public Kross::Script
{
    public:

        /**
         * Constructor.
         *
         * @param interpreter The @a RubyInterpreter instance used to
         *        create this script.
         * @param Action The @a Kross::Action
         *        instance this @a RubyScript does handle the
         *        backend-work for.
         */
        RubyScript(Kross::Interpreter* interpreter, Kross::Action* Action);

        /**
         * Destructor.
         */
        ~RubyScript();

        /**
         * Execute the script.
         *
         * \param args The optional arguments passed to the script
         * on excution.
         */
        virtual void execute();

        /**
         * \return the list of functionnames.
         */
        virtual QStringList functionNames();

        /**
         * Call a function in the script.
         *
         * \param name The name of the function which should be called.
         * \param args The optional list of arguments.
         */
        virtual QVariant callFunction(const QString& name, const QVariantList& args = QVariantList());

#if 0
        /**
         * Return a list of class types this script supports.
         */
        virtual const QStringList& getClassNames();

        /**
         * Create and return a new class instance.
         */
        virtual Kross::Object::Ptr classInstance(const QString& name);
#endif

    private:

        /**
         * Compile the script.
         */
        void compile();

    private:
        /// Private d-pointer.
        RubyScriptPrivate* d;
};

}

#endif
