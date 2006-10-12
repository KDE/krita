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
#include "interpreter.h"
#include "action.h"

#include <QObject>
#include <QMetaObject>
#include <QFile>
#include <QRegExp>

#include <klibloader.h>
#include <klocale.h>
#include <kstaticdeleter.h>
#include <kdialog.h>

extern "C"
{
    typedef QObject* (*def_module_func)();
}

using namespace Kross;

namespace Kross {

    /// @internal
    class ManagerPrivate
    {
        public:
            /// List of \a InterpreterInfo instances.
            QMap<QString, InterpreterInfo*> interpreterinfos;

            /// Loaded modules.
            QMap<QString, QObject* > modules;
    };

}

static KStaticDeleter<Manager> m_manager;
static Manager* _self = 0;

Manager& Manager::self()
{
    if(! _self)
        m_manager.setObject(_self, new Manager());
    return *_self;
}

Manager::Manager()
    : QObject()
    , ChildrenInterface()
    , d( new ManagerPrivate() )
{

#ifdef KROSS_PYTHON_LIBRARY
    QString pythonlib = QFile::encodeName( KLibLoader::self()->findLibrary(KROSS_PYTHON_LIBRARY) );
    if(! pythonlib.isEmpty()) { // If the Kross Python plugin exists we offer is as supported scripting language.
        InterpreterInfo::Option::Map pythonoptions;
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
    if(! rubylib.isEmpty()) { // If the Kross Ruby plugin exists we offer is as supported scripting language.
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
        krossdebug("Ruby interpreter for kross is unavailable");
    }
#endif

#ifdef KROSS_KJS_LIBRARY
    QString kjslib = QFile::encodeName( KLibLoader::self()->findLibrary(KROSS_KJS_LIBRARY) );
    if(! kjslib.isEmpty()) { // If the Kjs plugin exists we offer is as supported scripting language.
        InterpreterInfo::Option::Map kjsoptions;
        d->interpreterinfos.insert("javascript",
            new InterpreterInfo("javascript",
                kjslib, // library
                "*.js", // file filter-wildcard
                QStringList() << "application/x-javascript", // mimetypes
                kjsoptions // options
            )
        );
    }
#endif

    // publish ourself.
    ChildrenInterface::addObject(this, "Kross");
}

Manager::~Manager()
{
    for(QMap<QString, InterpreterInfo*>::Iterator it = d->interpreterinfos.begin(); it != d->interpreterinfos.end(); ++it)
        delete it.value();
    delete d;
}

QMap<QString, InterpreterInfo*> Manager::interpreterInfos()
{
    return d->interpreterinfos;
}

bool Manager::hasInterpreterInfo(const QString& interpretername) const
{
    return d->interpreterinfos.contains(interpretername);
}

InterpreterInfo* Manager::interpreterInfo(const QString& interpretername)
{
    return d->interpreterinfos[interpretername];
}

const QString Manager::interpreternameForFile(const QString& file)
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

Interpreter* Manager::interpreter(const QString& interpretername)
{
    if(! d->interpreterinfos.contains(interpretername)) {
        krosswarning( QString("No such interpreter '%1'").arg(interpretername) );
        return 0;
    }
    return d->interpreterinfos[interpretername]->getInterpreter();
}

QStringList Manager::interpreters()
{
    QStringList list;
    QMap<QString, InterpreterInfo*>::Iterator it( d->interpreterinfos.begin() );
    for(; it != d->interpreterinfos.end(); ++it)
        list << it.key();
    return  list;
}

bool Manager::hasAction(const QString& name)
{
    return findChild< Action* >(name) != 0L;
}

QObject* Manager::action(const QString& name)
{
    Action* action = findChild< Action* >(name);
    if(! action) {
        action = new Action(name);
        action->setParent(this);
    }
    return action;
}

QObject* Manager::module(const QString& modulename)
{
    if( d->modules.contains(modulename) ) {
        return d->modules[modulename];
    }

    if( modulename.isEmpty() || modulename.contains( QRegExp("[^a-zA-Z0-9]") ) ) {
        krosswarning( QString("Invalid module name '%1'").arg(modulename) );
        return 0;
    }

    QByteArray libraryname = QString("krossmodule%1").arg(modulename).toLower().toLatin1();

    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->globalLibrary( libraryname );
    if( ! lib ) {
        krosswarning( QString("Failed to load module '%1': %2").arg(modulename).arg(loader->lastErrorMessage()) );
        return 0;
    }

    def_module_func func;
    func = (def_module_func) lib->symbol("krossmodule");
    if( ! func ) {
        krosswarning( QString("Failed to determinate init function in module '%1'").arg(modulename) );
        return 0;
    }

    QObject* module = (QObject*) (func)(); // call the function
    //lib->unload(); // unload the library.

    if( ! module ) {
        krosswarning( QString("Failed to load module object '%1'").arg(modulename) );
        return 0;
    }

    //krossdebug( QString("Manager::module Module successfully loaded: modulename=%1 module.objectName=%2 module.className=%3").arg(modulename).arg(module->objectName()).arg(module->metaObject()->className()) );
    d->modules.insert(modulename, module);
    return module;
}

#include "manager.moc"
