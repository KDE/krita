/***************************************************************************
 * rubyscript.h
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

#include "rubyscript.h"

#include <ruby.h>
#include <env.h>
#include <rubysig.h>
#include <node.h>

#include "../core/action.h"

#include "rubyconfig.h"
#include "rubyextension.h"
#include "rubyinterpreter.h"

extern NODE *ruby_eval_tree;

using namespace Kross;

namespace Kross {

    class RubyScriptPrivate {
        friend class RubyScript;
        RubyScriptPrivate() : m_compile(0) { }
        RNode* m_compile;
        /// A list of functionnames.
        QStringList m_functions;
        #if 0
        /// A list of classnames.
        QStringList m_classes;
        #endif
    };

}

RubyScript::RubyScript(Kross::Interpreter* interpreter, Kross::Action* Action)
    : Kross::Script(interpreter, Action), d(new RubyScriptPrivate())
{
}

RubyScript::~RubyScript()
{
}

#define selectScript() \
    NODE* old_tree = ruby_eval_tree; \
    ruby_eval_tree = d->m_compile;
#define unselectScript() \
    d->m_compile = 0; \
    ruby_eval_tree = old_tree;

void RubyScript::compile()
{
    #ifdef KROSS_RUBY_SCRIPT_DEBUG
        krossdebug("RubyScript::compile()");
    #endif

    int critical;

    ruby_nerrs = 0;
    ruby_errinfo = Qnil;
    VALUE src = RubyExtension::toVALUE( m_action->code() );
    StringValue(src);
    critical = rb_thread_critical;
    rb_thread_critical = Qtrue;
    ruby_in_eval++;
    d->m_compile = rb_compile_string((char*) m_action->objectName().toLatin1().data(), src, 0);
    ruby_in_eval--;
    rb_thread_critical = critical;

    if (ruby_nerrs != 0) {
        #ifdef KROSS_RUBY_SCRIPT_DEBUG
            krossdebug("Compilation has failed");
        #endif
        setError( QString("Failed to compile ruby code: %1").arg(STR2CSTR( rb_obj_as_string(ruby_errinfo) )) ); // TODO: get the error
        d->m_compile = 0;
    }

    #ifdef KROSS_RUBY_SCRIPT_DEBUG
        krossdebug("Compilation was successfull");
    #endif
}

void RubyScript::execute()
{
    #ifdef KROSS_RUBY_SCRIPT_DEBUG
        krossdebug("RubyScript::execute()");
    #endif

    if(d->m_compile == 0) {
        compile();
    }

    #ifdef KROSS_RUBY_SCRIPT_DEBUG
        krossdebug("Start execution");
    #endif

    selectScript();
    int result = ruby_exec();
    if (result != 0) {
        #ifdef KROSS_RUBY_SCRIPT_DEBUG
            krossdebug("Execution has failed");
        #endif

        /*
        if( TYPE( ruby_errinfo )  == T_DATA && RubyExtension::isOfExceptionType( ruby_errinfo ) )
        {
            setError( RubyExtension::convertToException( ruby_errinfo ) );
        } else
        */
        setError( QString("Failed to execute ruby code: %1").arg(STR2CSTR( rb_obj_as_string(ruby_errinfo) )) ); // TODO: get the error
    }

    unselectScript();

    #ifdef KROSS_RUBY_SCRIPT_DEBUG
        krossdebug("Execution is finished");
    #endif
}

QStringList RubyScript::functionNames()
{
    #ifdef KROSS_RUBY_SCRIPT_DEBUG
        krossdebug("RubyScript::getFunctionNames()");
    #endif

    if(d->m_compile == 0) {
        compile();
    }
    return d->m_functions;
}

QVariant RubyScript::callFunction(const QString& name, const QVariantList& args)
{
    Q_UNUSED(name)
    Q_UNUSED(args)

    #ifdef KROSS_RUBY_SCRIPT_DEBUG
        krossdebug("RubyScript::callFunction()");
    #endif

    if(d->m_compile == 0) {
        compile();
    }
    selectScript();
    unselectScript();

//TODO    return Kross::Object::Ptr(0);

    return QVariant();
}

#if 0
const QStringList& RubyScript::getClassNames()
{
#ifdef KROSS_RUBY_SCRIPT_DEBUG
    krossdebug("RubyScript::getClassNames()");
#endif
    if(d->m_compile == 0) {
        compile();
    }
    return d->m_classes;
}

Kross::Object::Ptr RubyScript::classInstance(const QString& name)
{
    Q_UNUSED(name)
#ifdef KROSS_RUBY_SCRIPT_DEBUG
    krossdebug("RubyScript::classInstance()");
#endif
    if(d->m_compile == 0) {
        compile();
    }
    selectScript();
    unselectScript();
    return Kross::Object::Ptr(0);
}
#endif
