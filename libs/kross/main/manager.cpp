/***************************************************************************
 * manager.cpp
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

#include "manager.h"

#include "../api/interpreter.h"
//#include "../api/qtobject.h"
#include "../api/eventslot.h"
#include "../api/eventsignal.h"
//#include "../api/script.h"

#include "../api/krossconfig.h"
#include "scriptcontainer.h"

#include <QObject>
#include <QFile>
#include <QRegExp>

#include <klibloader.h>
#include <klocale.h>
#include <kstaticdeleter.h>

extern "C"
{
    typedef Kross::Api::Object* (*def_module_func)(Kross::Api::Manager*);
}

using namespace Kross::Api;

namespace Kross { namespace Api {

    /// @internal
    class ManagerPrivate
    {
        public:
            /// List of \a InterpreterInfo instances.
            QMap<QString, InterpreterInfo*> interpreterinfos;

            /// Loaded modules.
            QMap<QString, Module::Ptr> modules;
    };

    /**
     * Free the static Manager instance if the lib is unloaded or
     * the app terminates by using the KStaticDeleter template.
     */
    static KStaticDeleter<Manager> m_managerdeleter;

    /**
     * The Manager-singleton instance is NULL by default till the
     * Manager::scriptManager() method got called first time.
     */
    static Manager* m_manager = 0;

}}

Manager* Manager::scriptManager()
{
    if(! m_manager) {
        // Create the Manager-singleton on demand and let the
        // KStaticDeleter take care of freeing it if not needed
        // any longer.
        m_managerdeleter.setObject(m_manager, new Manager());
    }

    // and finally return the singleton.
    return m_manager;
}

Manager::Manager()
    : MainModule("Kross") // the manager has the name "Kross"
    , d( new ManagerPrivate() )
{
#ifdef KROSS_PYTHON_LIBRARY
    QString pythonlib = QFile::encodeName( KLibLoader::self()->findLibrary(KROSS_PYTHON_LIBRARY) );
    if(! pythonlib.isEmpty()) { // If the Kross Python plugin exists we offer it as supported scripting language.
        InterpreterInfo::Option::Map pythonoptions;
        pythonoptions.insert("restricted",
            new InterpreterInfo::Option("Restricted", "Restricted Python interpreter", QVariant(false))
        );
        d->interpreterinfos.insert("python",
            new InterpreterInfo("python",
                pythonlib, // library
                "*.py", // file filter-wildcard
                QStringList() << /* "text/x-python" << */ "application/x-python", // mimetypes
                pythonoptions // options
            )
        );
    }
#endif
#ifdef KROSS_RUBY_LIBRARY
    QString rubylib = QFile::encodeName( KLibLoader::self()->findLibrary(KROSS_RUBY_LIBRARY) );
    if(! rubylib.isEmpty()) { // If the Kross Ruby plugin exists we offer it as supported scripting language.
      InterpreterInfo::Option::Map rubyoptions;
      rubyoptions.insert("safelevel",
                          new InterpreterInfo::Option("safelevel", "Level of safety of the Ruby interpreter", QVariant(0)) // 0 -> unsafe, 4 -> very safe
                           );
      d->interpreterinfos.insert("ruby",
                                  new InterpreterInfo("ruby",
                                      rubylib, // library
                                      "*.rb", // file filter-wildcard
                                      QStringList() << /* "text/x-ruby" << */ "application/x-ruby", // mimetypes
                                      rubyoptions // options
                                                     )
                                 );
    } else {
        krossdebug("Ruby interpreter for kross in unavailable");
    }
#endif
}

Manager::~Manager()
{
    for(QMap<QString, InterpreterInfo*>::Iterator it = d->interpreterinfos.begin(); it != d->interpreterinfos.end(); ++it)
        delete it.value();
    delete d;
}

QMap<QString, InterpreterInfo*> Manager::getInterpreterInfos()
{
    return d->interpreterinfos;
}

bool Manager::hasInterpreterInfo(const QString& interpretername) const
{
    return d->interpreterinfos.contains(interpretername);
}

InterpreterInfo* Manager::getInterpreterInfo(const QString& interpretername)
{
    return d->interpreterinfos[interpretername];
}

const QString Manager::getInterpreternameForFile(const QString& file)
{
    QRegExp rx;
    rx.setPatternSyntax(QRegExp::Wildcard);
    for(QMap<QString, InterpreterInfo*>::Iterator it = d->interpreterinfos.begin(); it != d->interpreterinfos.end(); ++it) {
        rx.setPattern((*it)->getWildcard());
        if( file.contains(rx) )
            return (*it)->getInterpretername();
    }
    return QString::null;
}

ScriptContainer::Ptr Manager::getScriptContainer(const QString& scriptname)
{
    //TODO at the moment we don't share ScriptContainer instances.

    //if(d->m_scriptcontainers.contains(scriptname))
    //    return d->m_scriptcontainers[scriptname];
    ScriptContainer* scriptcontainer = new ScriptContainer(scriptname);
    //ScriptContainer script(this, scriptname);
    //d->m_scriptcontainers.insert(scriptname, scriptcontainer);

    return ScriptContainer::Ptr( scriptcontainer );
}

Interpreter* Manager::getInterpreter(const QString& interpretername)
{
    setException(0); // clear previous exceptions

    if(! d->interpreterinfos.contains(interpretername)) {
        setException( new Exception(i18n("No such interpreter '%1'",interpretername)) );
        return 0;
    }

    return d->interpreterinfos[interpretername]->getInterpreter();
}

const QStringList Manager::getInterpreters()
{
    QStringList list;

    QMap<QString, InterpreterInfo*>::Iterator it( d->interpreterinfos.begin() );
    for(; it != d->interpreterinfos.end(); ++it)
        list << it.key();

//list << "TestCase";

    return  list;
}

bool Manager::addModule(Module::Ptr module)
{
    QString name = module->getName();
    //if( d->modules.contains(name) ) return false;
    d->modules.insert(name, module);
    return true;
}

Module* Manager::loadModule(const QString& modulename)
{
    if(d->modules.contains(modulename)) {
        return d->modules[modulename].data();
    }

    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->globalLibrary( modulename.toLatin1().data() );
    if(! lib) {
        krosswarning( QString("Failed to load module '%1': %2").arg(modulename).arg(loader->lastErrorMessage()) );
        return 0;
    }
    krossdebug( QString("Successfully loaded module '%1'").arg(modulename) );

    def_module_func func;
    func = (def_module_func) lib->symbol("init_module");

    if(! func) {
        krosswarning( QString("Failed to determinate init function in module '%1'").arg(modulename) );
        return 0;
    }

    Kross::Api::Module* module;
    try {
        module = (Kross::Api::Module*) (func)(this);
    }
    catch(Kross::Api::Exception::Ptr e) {
        krosswarning( e->toString() );
        module = 0;
    }
    lib->unload();

    if(! module) {
        krosswarning( QString("Failed to load module '%1'").arg(modulename) );
        return 0;
    }

    // Don't remember module cause we like to have freeing it handled by the caller.
    //d->modules.insert(modulename, module);

    //krossdebug( QString("Kross::Api::Manager::loadModule modulename='%1' module='%2'").arg(modulename).arg(module->toString()) );
    return module;
}

