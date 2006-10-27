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
#include "../core/interpreter.h"

// for Kjs
#include <kjs/interpreter.h>
#include <kjs/ustring.h>
#include <kjs/object.h>
//#include <kjs/PropertyNameArray.h>

// for KjsEmbed
#include <kjsembed/kjsembed.h>
//#include <kjsembed/qobject_binding.h>

using namespace Kross;

namespace Kross {

    /// Extract an errormessage from a KJS::Completion object.
    static ErrorInterface extractError(const KJS::Completion& completion, KJS::ExecState* exec)
    {
        QString type;
        switch( completion.complType() ) {
            case KJS::Normal: type = "Normal"; break;
            case KJS::Break: type = "Break"; break;
            case KJS::Continue: type = "Continue"; break;
            case KJS::ReturnValue: type = "ReturnValue"; break;
            case KJS::Throw: {
                type = "Throw";
            } break;
            case KJS::Interrupted: type = "Interrupted"; break;
            default: type = "Unknown"; break;
        }

        KJS::JSValue* value = completion.value();
        Q_ASSERT(value);
        int lineno = -1;
        if( value->type() == KJS::ObjectType ) {
            KJS::JSValue* linevalue = value->getObject()->get(exec, "line");
            if( linevalue && linevalue->type() == KJS::NumberType )
                lineno = linevalue->toInt32(exec);
        }
        const QString message = QString("%1%2: %3").arg( type ).arg( (lineno >= 0) ? QString(" line%1").arg(lineno) : "" ).arg( value->toString(exec).qstring() );

        ErrorInterface err;
        err.setError(message, QString::null, lineno);
        return err;
    }

    /// Publish a QObject to a KJSEmbed::Engine.
    static void publishObject(KJSEmbed::Engine* engine, KJS::ExecState* exec, QString name, QObject* object, bool restricted)
    {
        KJS::JSObject* obj = engine->addObject(object, name.isEmpty() ? object->objectName() : name);
        if( ! obj ) {
            krossdebug( QString("Failed to publish the QObject name=\"%1\" objectName=\"%2\" restricted=\"%3\"").arg(name).arg(object ? object->objectName() : "NULL").arg(restricted) );
            return;
        }
        if( restricted ) {

/*FIXME needs fix for #include <kjs/object.h> which does #include "internal.h"
            KJSEmbed::QObjectBinding* objImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, obj);
            objImp->setAccess(
                KJSEmbed::QObjectBinding::ScriptableSlots |
                KJSEmbed::QObjectBinding::NonScriptableSlots |
                KJSEmbed::QObjectBinding::PublicSlots |
                KJSEmbed::QObjectBinding::ScriptableSignals |
                KJSEmbed::QObjectBinding::NonScriptableSignals |
                KJSEmbed::QObjectBinding::PublicSignals |
                KJSEmbed::QObjectBinding::ScriptableProperties |
                KJSEmbed::QObjectBinding::NonScriptableProperties |
                KJSEmbed::QObjectBinding::GetParentObject |
                KJSEmbed::QObjectBinding::ChildObjects
            );
*/

        }
    }

    /// \internal d-pointer class.
    class KjsScriptPrivate
    {
        public:
            /// One engine per script to have them clean separated.
            KJSEmbed::Engine* engine;
    };

}

KjsScript::KjsScript(Kross::Interpreter* interpreter, Kross::Action* action)
    : Kross::Script(interpreter, action)
    , d(new KjsScriptPrivate())
{
    Kross::krossdebug( QString("KjsScript::KjsScript") );
    d->engine = 0;
}

KjsScript::~KjsScript()
{
    Kross::krossdebug( QString("KjsScript::~KjsScript") );
    finalize();
    delete d;
}

bool KjsScript::initialize()
{
    finalize(); // finalize before initialize
    clearError(); // clear previous errors.

    bool restricted = interpreter()->interpreterInfo()->optionValue("restricted", true).toBool();

    Kross::krossdebug( QString("KjsScript::initialize restricted=%1").arg(restricted) );

    d->engine = new KJSEmbed::Engine();

    KJS::Interpreter* kjsinterpreter = d->engine->interpreter();
    KJS::ExecState* exec = kjsinterpreter->globalExec();

    { // publish the global objects.
        QHash< QString, QObject* > objects = Kross::Manager::self().objects();
        QHash< QString, QObject* >::Iterator it(objects.begin()), end(objects.end());
        for(; it != end; ++it)
            publishObject(d->engine, exec, it.key(), it.value(), restricted);
    }

    { // publish the local objects.
        QHash< QString, QObject* > objects = action()->objects();
        QHash< QString, QObject* >::Iterator it(objects.begin()), end(objects.end());
        for(; it != end; ++it)
            publishObject(d->engine, exec, it.key(), it.value(), restricted);
    }

    /*
    { // some debugging
        Kross::krossdebug( QString("Global object") );
        KJS::JSObject* obj = kjsinterpreter->globalObject();
        KJS::ExecState* exec = kjsinterpreter->globalExec();
        KJS::PropertyNameArray props;
        obj->getPropertyNames(exec, props);
        for(KJS::PropertyNameArrayIterator it = props.begin(); it != props.end(); it++)
            Kross::krossdebug( QString("  property name=%1").arg( it->ascii() ) );
    }
    */

    return true;
}

void KjsScript::finalize()
{
    delete d->engine;
    d->engine = 0;
}

void KjsScript::execute()
{
    if(! initialize()) {
        krossdebug( QString("KjsScript::execute aborted cause initialize failed.") );
        return;
    }

    KJS::UString code = action()->code();
    //krossdebug( QString("KjsScript::execute code=\n%1").arg(code.qstring()) );
    KJSEmbed::Engine::ExitStatus exitstatus = d->engine->execute(code);
    KJS::Completion completion = d->engine->completion();

    if(exitstatus != KJSEmbed::Engine::Success) {
        KJS::Interpreter* kjsinterpreter = d->engine->interpreter();
        KJS::ExecState* exec = kjsinterpreter->globalExec();
        ErrorInterface error = extractError(completion, exec);
        setError(&error);
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
    Q_UNUSED(name);
    Q_UNUSED(args);
    return QVariant();
}

