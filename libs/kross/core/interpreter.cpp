/***************************************************************************
 * interpreter.cpp
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

#include "interpreter.h"
#include "script.h"
#include "action.h"
#include "manager.h"

#include <klibloader.h>

extern "C"
{
    typedef int (*def_interpreter_func)(Kross::InterpreterInfo*);
}

using namespace Kross;

/*************************************************************************
 * InterpreterInfo
 */

InterpreterInfo::InterpreterInfo(const QString& interpretername, const QString& library, const QString& wildcard, QStringList mimetypes, Option::Map options)
    : m_interpretername(interpretername)
    , m_library(library)
    , m_wildcard(wildcard)
    , m_mimetypes(mimetypes)
    , m_options(options)
    , m_interpreter(0)
{
}

InterpreterInfo::~InterpreterInfo()
{
    for(Option::Map::Iterator it = m_options.begin(); it != m_options.end(); ++it) {
        delete it.value();
    }

    delete m_interpreter;
    m_interpreter = 0;
}

const QString InterpreterInfo::getInterpretername()
{
    return m_interpretername;
}

const QString InterpreterInfo::getWildcard()
{
    return m_wildcard;
}

const QStringList InterpreterInfo::getMimeTypes()
{
    return m_mimetypes;
}

bool InterpreterInfo::hasOption(const QString& key)
{
    return m_options.contains(key);
}

InterpreterInfo::Option* InterpreterInfo::getOption(const QString name)
{
    return m_options[name];
}

const QVariant InterpreterInfo::getOptionValue(const QString name, QVariant defaultvalue)
{
    Option* o = m_options[name];
    return o ? o->value : defaultvalue;
}

InterpreterInfo::Option::Map InterpreterInfo::getOptions()
{
    return m_options;
}

Interpreter* InterpreterInfo::getInterpreter()
{
    if(m_interpreter) // buffered
        return m_interpreter;
#ifdef KROSS_INTERPRETER_DEBUG
    krossdebug( QString("Loading the interpreter library for %1").arg(m_interpretername) );
#endif
    // Load the krosspython library.
    KLibLoader *libloader = KLibLoader::self();

    KLibrary* library = libloader->globalLibrary( m_library.toLatin1().data() );
    if(! library) {
        /*
        setException(
            new Exception( QString("Could not load library \"%1\" for the \"%2\" interpreter.").arg(m_library).arg(m_interpretername) )
        );
        */
        krosswarning( QString("Could not load library \"%1\" for the \"%2\" interpreter.").arg(m_library).arg(m_interpretername) );
        return 0;
    }

    // Get the extern "C" krosspython_instance function.
    def_interpreter_func interpreter_func;
    interpreter_func = (def_interpreter_func) library->symbol("krossinterpreter");
    if(! interpreter_func) {
        //setException( new Exception("Failed to load symbol in krosspython library.") );
        krosswarning("Failed to load the 'krossinterpreter' symbol from the library.");
    }
    else {
        // and execute the extern krosspython_instance function.
        m_interpreter = (Interpreter*) (interpreter_func)(this);
        if(! m_interpreter) {
            krosswarning("Failed to load the Interpreter instance from library.");
        }
        else {
            // Job done. The library is loaded and our Interpreter* points
            // to the external Kross::Python::Interpreter* instance.
#ifdef KROSS_INTERPRETER_DEBUG 
            krossdebug("Successfully loaded Interpreter instance from library.");
#endif
        }
    }

    // finally unload the library.
    library->unload();

    return m_interpreter;
}

/*************************************************************************
 * Interpreter
 */

Interpreter::Interpreter(InterpreterInfo* info)
    : ErrorInterface()
    , m_interpreterinfo(info)
{
}

Interpreter::~Interpreter()
{
}
