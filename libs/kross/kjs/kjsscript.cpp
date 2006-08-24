/***************************************************************************
 * kjsscript.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
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

#include "kjsscript.h"
#include "../core/action.h"
#include "../core/manager.h"

// for Kjs
#include <kjs/interpreter.h>
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/PropertyNameArray.h>

// for KjsEmbed
//#include <kjsembed/kjsembed/kjsembed.h>
#include <kjsembed/kjsembed.h>

using namespace Kross;

namespace Kross {

    /// \internal
    class KjsScriptPrivate
    {
        public:
            /// One engine per script to have them clean separated.
            KJSEmbed::Engine engine;

            /// Extract an errormessage from a \a KJS::Completion object.
            QString extractError(const KJS::Completion& completion, KJS::ExecState* exec)
            {
                QString type;
                switch( completion.complType() ) {
                    case KJS::Normal: type = "Normal"; break;
                    case KJS::Break: type = "Break"; break;
                    case KJS::Continue: type = "Continue"; break;
                    case KJS::ReturnValue: type = "ReturnValue"; break;
                    case KJS::Throw: type = "Throw"; break;
                    case KJS::Interrupted: type = "Interrupted"; break;
                    default: type = "Unknown"; break;
                }
                KJS::JSValue* value = completion.value();
                return QString("%1: %2").arg(type).arg(value ? value->toString(exec).qstring() : "NULL");
            }

    };

}

KjsScript::KjsScript(Kross::Interpreter* interpreter, Kross::Action* action)
    : Kross::Script(interpreter, action)
    , d(new KjsScriptPrivate())
{
    Kross::krossdebug( QString("KjsScript::KjsScript Ctor") );

    KJS::Interpreter* kjsinterpreter = d->engine.interpreter();
    KJS::JSObject* obj = kjsinterpreter->globalObject();
    KJS::ExecState* exec = kjsinterpreter->globalExec();

    { // publish the global objects.
        QHash< QString, QObject* > objects = Kross::Manager::self().objects();
        QHash< QString, QObject* >::Iterator it(objects.begin()), end(objects.end());
        for(; it != end; ++it)
            d->engine.addObject( it.value(), it.key() );
    }

    { // some debugging
        Kross::krossdebug( QString("Global object") );
        KJS::PropertyNameArray props;
        obj->getPropertyNames(exec, props);
        for(KJS::PropertyNameArrayIterator it = props.begin(); it != props.end(); it++)
            Kross::krossdebug( QString("  property name=%1").arg( it->ascii() ) );
    }
}

KjsScript::~KjsScript()
{
    Kross::krossdebug( QString("KjsScript::~KjsScript Dtor") );
    delete d;
}

bool KjsScript::initialize()
{
    finalize(); // finalize before initialize
    clearError(); // clear previous errors.

    //d->engine.interpreter()->initGlobalObject();
    return true;
}

void KjsScript::finalize()
{
}

void KjsScript::execute(const QVariant& args)
{
    Q_UNUSED(args);

    if(! initialize()) {
        krossdebug( QString("KjsScript::execute aborted cause initialize failed.") );
        return;
    }

    KJS::UString code = m_action->getCode();
    KJSEmbed::Engine::ExitStatus exitstatus = d->engine.execute(code);
    KJS::Completion completion = d->engine.completion();

    if(exitstatus != KJSEmbed::Engine::Success) {
        KJS::Interpreter* kjsinterpreter = d->engine.interpreter();
        KJS::ExecState* exec = kjsinterpreter->globalExec();
        setError( d->extractError(completion, exec) );
    }
}

QStringList KjsScript::functionNames()
{
    //TODO
    return QStringList();
}

QVariant KjsScript::callFunction(const QString& name, const QVariantList& args)
{
    //TODO
    return QVariant();
}

