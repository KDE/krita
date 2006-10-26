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
    typedef void* (*def_interpreter_func)(Kross::InterpreterInfo*);
}

using namespace Kross;

/*************************************************************************
 * InterpreterInfo
 */

namespace Kross {

    /// \internal d-pointer class.
    class InterpreterInfo::Private
    {
        public:
            /// The name the interpreter has. Could be something like "python" or "kjs".
            QString interpretername;
            /// The name of the library to load for the interpreter.
            QString library;
            /// The file wildcard used to determinate extensions.
            QString wildcard;
            /// List of mimetypes this interpreter supports.
            QStringList mimetypes;
            /// A \a Option::Map with options.
            Option::Map options;
            /// The \a Interpreter instance.
            Interpreter* interpreter;
    };

}

InterpreterInfo::InterpreterInfo(const QString& interpretername, const QString& library, const QString& wildcard, QStringList mimetypes, Option::Map options)
    : d( new Private() )
{
    d->interpretername = interpretername;
    d->library = library;
    d->wildcard = wildcard;
    d->mimetypes = mimetypes;
    d->options = options;
    d->interpreter = 0;
}

InterpreterInfo::~InterpreterInfo()
{
    delete d->interpreter;
    //d->interpreter = 0;
    delete d;
}

const QString InterpreterInfo::interpreterName() const
{
    return d->interpretername;
}

const QString InterpreterInfo::wildcard() const
{
    return d->wildcard;
}

const QStringList InterpreterInfo::mimeTypes() const
{
    return d->mimetypes;
}

bool InterpreterInfo::hasOption(const QString& key) const
{
    return d->options.contains(key);
}

InterpreterInfo::Option* InterpreterInfo::option(const QString name) const
{
    return d->options[name];
}

const QVariant InterpreterInfo::optionValue(const QString name, QVariant defaultvalue) const
{
    Option* o = option(name);
    return o ? o->value : defaultvalue;
}

InterpreterInfo::Option::Map InterpreterInfo::options()
{
    return d->options;
}

Interpreter* InterpreterInfo::interpreter()
{
    if(d->interpreter) // buffered
        return d->interpreter;

#ifdef KROSS_INTERPRETER_DEBUG
    krossdebug( QString("Loading the interpreter library for %1").arg(d->interpretername) );
#endif
    // Load the krosspython library.
    KLibLoader *libloader = KLibLoader::self();

    KLibrary* library = libloader->globalLibrary( d->library.toLatin1().data() );
    if(! library) {
        /*
        setException(
            new Exception( QString("Could not load library \"%1\" for the \"%2\" interpreter.").arg(d->library).arg(d->interpretername) )
        );
        */
        krosswarning( QString("Could not load library \"%1\" for the \"%2\" interpreter.").arg(d->library).arg(d->interpretername) );
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
        d->interpreter = (Interpreter*) (interpreter_func)(this);
        if(! d->interpreter) {
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

    return d->interpreter;
}

/*************************************************************************
 * Interpreter
 */

namespace Kross {

    /// \internal d-pointer class.
    class Interpreter::Private
    {
        public:
            /// The \a InterpreterInfo instance this interpreter belongs to.
            InterpreterInfo* interpreterinfo;

            Private(InterpreterInfo* info) : interpreterinfo(info) {}
    };

}

Interpreter::Interpreter(InterpreterInfo* info)
    : ErrorInterface()
    , d( new Private(info) )
{
}

Interpreter::~Interpreter()
{
    delete d;
}

InterpreterInfo* Interpreter::interpreterInfo() const
{
    return d->interpreterinfo;
}

void Interpreter::virtual_hook(int id, void* data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
}
