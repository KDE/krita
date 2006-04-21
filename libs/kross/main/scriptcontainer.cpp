/***************************************************************************
 * scriptcontainer.cpp
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

#include "scriptcontainer.h"
#include "../api/object.h"
#include "../api/list.h"
#include "../api/interpreter.h"
#include "../api/script.h"
#include "../main/manager.h"
#include "mainmodule.h"

#include <qfile.h>

#include <klocale.h>

using namespace Kross::Api;

namespace Kross { namespace Api {

    /// @internal
    class ScriptContainerPrivate
    {
        public:

            /**
            * The \a Script instance the \a ScriptContainer uses
            * if initialized. It will be NULL as long as we
            * didn't initialized it what will be done on
            * demand.
            */
            Script* script;

            /**
            * The unique name the \a ScriptContainer is
            * reachable as.
            */
            QString name;

            /**
            * The scripting code.
            */
            QString code;

            /**
            * The name of the interpreter. This could be
            * something like "python" for the python
            * binding.
            */
            QString interpretername;

            /**
            * The name of the scriptfile that should be
            * executed. Those scriptfile will be readed
            * and the content will be used to set the
            * scripting code and, if not defined, the
            * used interpreter.
            */
            QString scriptfile;

            /**
            * Map of options that overwritte the \a InterpreterInfo::Option::Map
            * standard options.
            */
            QMap<QString, QVariant> options;

    };

}}

ScriptContainer::ScriptContainer(const QString& name)
    : MainModule(name)
    , d( new ScriptContainerPrivate() ) // initialize d-pointer class
{
    krossdebug( QString("ScriptContainer::ScriptContainer() Ctor name='%1'").arg(name) );

    d->script = 0;
    d->name = name;
}

ScriptContainer::~ScriptContainer()
{
    krossdebug( QString("ScriptContainer::~ScriptContainer() Dtor name='%1'").arg(d->name) );

    finalize();
    delete d;
}

QString ScriptContainer::getName() const
{
    return d->name;
}

void ScriptContainer::setName(const QString& name)
{
    d->name = name;
}

QString ScriptContainer::getCode() const
{
    return d->code;
}

void ScriptContainer::setCode(const QString& code)
{
    finalize();
    d->code = code;
}

QString ScriptContainer::getInterpreterName() const
{
    return d->interpretername;
}

void ScriptContainer::setInterpreterName(const QString& interpretername)
{
    finalize();
    d->interpretername = interpretername;
}

QString ScriptContainer::getFile() const
{
    return d->scriptfile;
}

void ScriptContainer::setFile(const QString& scriptfile)
{
    finalize();
    d->scriptfile = scriptfile;
}

QMap<QString, QVariant>& ScriptContainer::getOptions()
{
    return d->options;
}

QVariant ScriptContainer::getOption(const QString name, QVariant defaultvalue, bool /*recursive*/)
{
    if(d->options.contains(name))
        return d->options[name];
    Kross::Api::InterpreterInfo* info = Kross::Api::Manager::scriptManager()->getInterpreterInfo( d->interpretername );
    return info ? info->getOptionValue(name, defaultvalue) : defaultvalue;
}

bool ScriptContainer::setOption(const QString name, const QVariant& value)
{
    Kross::Api::InterpreterInfo* info = Kross::Api::Manager::scriptManager()->getInterpreterInfo( d->interpretername );
    if(info) {
        if(info->hasOption(name)) {
            d->options.insert(name, value);
            return true;
        } else krosswarning( QString("Kross::Api::ScriptContainer::setOption(%1, %2): No such option").arg(name).arg(value.toString()) );
    } else krosswarning( QString("Kross::Api::ScriptContainer::setOption(%1, %2): No such interpreterinfo").arg(name).arg(value.toString()) );
    return false;
}

Object::Ptr ScriptContainer::execute()
{
    if(! d->script)
        if(! initialize())
            return Object::Ptr();

    if(hadException())
        return Object::Ptr();

    Object::Ptr r = d->script->execute();
    if(d->script->hadException()) {
        setException( d->script->getException() );
        finalize();
        return Object::Ptr();
    }
    return r;
}

const QStringList ScriptContainer::getFunctionNames()
{
    return d->script ? d->script->getFunctionNames() : QStringList(); //FIXME init before if needed?
}

Object::Ptr ScriptContainer::callFunction(const QString& functionname, List::Ptr arguments)
{
    if(! d->script)
        if(! initialize())
            return Object::Ptr();

    if(hadException())
        return Object::Ptr();

    if(functionname.isEmpty()) {
        setException( new Exception(QString(i18n("No functionname defined for ScriptContainer::callFunction()."))) );
        finalize();
        return Object::Ptr();
    }

    Object::Ptr r = d->script->callFunction(functionname, arguments);
    if(d->script->hadException()) {
        setException( d->script->getException() );
        finalize();
        return Object::Ptr();
    }
    return r;
}

QStringList ScriptContainer::getClassNames()
{
    return d->script ? d->script->getClassNames() : QStringList(); //FIXME init before if needed?
}

Object::Ptr ScriptContainer::classInstance(const QString& classname)
{
    if(! d->script)
        if(! initialize())
            return Object::Ptr();

    if(hadException())
        return Object::Ptr();

    Object::Ptr r = d->script->classInstance(classname);
    if(d->script->hadException()) {
        setException( d->script->getException() );
        finalize();
        return Object::Ptr();
    }
    return r;
}

bool ScriptContainer::initialize()
{
    finalize();

    if(! d->scriptfile.isNull()) {
        krossdebug( QString("Kross::Api::ScriptContainer::initialize() file=%1").arg(d->scriptfile) );

        if(d->interpretername.isNull()) {
            d->interpretername = Manager::scriptManager()->getInterpreternameForFile( d->scriptfile );
            if(d->interpretername.isNull()) {
                setException( new Exception(QString(i18n("Failed to determinate interpreter for scriptfile '%1'")).arg(d->scriptfile)) );
                return false;
            }
        }

        QFile f( d->scriptfile );
        if(! f.open(QIODevice::ReadOnly)) {
            setException( new Exception(QString(i18n("Failed to open scriptfile '%1'")).arg(d->scriptfile)) );
            return false;
        }
        d->code = QString( f.readAll() );
        f.close();
    }

    Interpreter* interpreter = Manager::scriptManager()->getInterpreter(d->interpretername);
    if(! interpreter) {
        setException( new Exception(QString(i18n("Unknown interpreter '%1'")).arg(d->interpretername)) );
        return false;
    }

    d->script = interpreter->createScript(this);
    if(! d->script) {
        setException( new Exception(QString(i18n("Failed to create script for interpreter '%1'")).arg(d->interpretername)) );
        return false;
    }
    if(d->script->hadException()) {
        setException( d->script->getException() );
        finalize();
        return false;
    }
    setException( 0 ); // clear old exception

    return true;
}

void ScriptContainer::finalize()
{
    delete d->script;
    d->script = 0;
}


