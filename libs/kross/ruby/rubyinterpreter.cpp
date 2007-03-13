/***************************************************************************
 * rubyinterpreter.cpp
 * This file is part of the KDE project
 * copyright (C)2005,2007 by Cyrille Berger (cberger@cberger.net)
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
#include "rubyinterpreter.h"

#include <map>

#include <QRegExp>

//#include "../core/exception.h"
//#include "../core/module.h"
#include <kross/core/manager.h>

#include "rubyconfig.h"
#include "rubyextension.h"
#include "rubymodule.h"
#include "rubyscript.h"

// The in krossconfig.h defined KROSS_EXPORT_INTERPRETER macro defines an
// exported C function used as factory for Kross::RubyInterpreter instances.
KROSS_EXPORT_INTERPRETER( Kross::RubyInterpreter )

using namespace Kross;

namespace Kross {

    //typedef std::map<QString, VALUE> mStrVALUE;
    //typedef mStrVALUE::iterator mStrVALUE_it;
    //typedef mStrVALUE::const_iterator mStrVALUE_cit;

    /// \internal
    class RubyInterpreterPrivate {
        friend class RubyInterpreter;
        QHash<QString, RubyModule* > modules;
        static VALUE s_krossModule;
    };
}

RubyInterpreterPrivate* RubyInterpreter::d = 0;
VALUE RubyInterpreterPrivate::s_krossModule = 0;

RubyInterpreter::RubyInterpreter(Kross::InterpreterInfo* info)
    : Kross::Interpreter(info)
{
#ifdef KROSS_RUBY_INTERPRETER_DEBUG
    krossdebug("RubyInterpreter::RubyInterpreter(info)");
#endif

    if(d == 0)
    {
        initRuby();
    }

    const int defaultsafelevel = 4; // per default use the maximum safelevel
    rb_set_safe_level( info->optionValue("safelevel", defaultsafelevel).toInt() );
}

RubyInterpreter::~RubyInterpreter()
{
    finalizeRuby();
}

QHash<QString, RubyModule* > RubyInterpreter::modules() const
{
    return d->modules;
}

Kross::Script* RubyInterpreter::createScript(Kross::Action* action)
{
    return new RubyScript(this, action);
}

void RubyInterpreter::initRuby()
{
    d = new RubyInterpreterPrivate();
    ruby_init();
    ruby_init_loadpath();
    rb_define_global_function("require", (VALUE (*)(...))RubyInterpreter::require, 1);
}

VALUE RubyInterpreter::krossModule()
{
    if(RubyInterpreterPrivate::s_krossModule == 0)
    {
        RubyInterpreterPrivate::s_krossModule = rb_define_module("Kross");
    }
    return RubyInterpreterPrivate::s_krossModule;
}


void RubyInterpreter::finalizeRuby()
{
    if(d) {
        for(QHash<QString, RubyModule* >::Iterator it = d->modules.begin(); it != d->modules.end(); ++it)
            delete it.value();
        d->modules.clear();
    }

    delete d;
    d = 0;
    ruby_finalize();
}

VALUE RubyInterpreter::require (VALUE obj, VALUE name)
{
    QString modname = StringValuePtr(name);
    if( Kross::Manager::self().hasObject(modname) ) {
        #ifdef KROSS_RUBY_INTERPRETER_DEBUG
            krossdebug( QString("RubyInterpreter::require() module=%1 is internal").arg(modname) );
        #endif

        if(! d->modules.contains(modname)) {
            QObject* obj = Kross::Manager::self().object(modname);
            Q_ASSERT(obj);
            RubyModule* module = new RubyModule(obj, modname);
            //VALUE rmodule = rb_define_module(modname.ascii());
            //rb_define_module_function();
            //VALUE rm = RubyExtension::toVALUE(module);
            //rb_define_variable( ("$" + modname).ascii(), & RubyInterpreter::d->m_modules.insert( mStrVALUE::value_type( modname, rm) ).first->second );
            d->modules.insert(modname, module);
        }

        return Qtrue;
    }

    return rb_f_require(obj, name);
}
