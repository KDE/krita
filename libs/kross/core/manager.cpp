/***************************************************************************
 * manager.cpp
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

#include "manager.h"
#include "interpreter.h"
#include "action.h"
#include "actioncollection.h"

#include <QObject>
#include <QMetaObject>
#include <QFile>
#include <QRegExp>
#include <QAbstractItemModel>
#include <QFileInfo>

#include <kapplication.h>
#include <kactioncollection.h>
#include <klibloader.h>
#include <klocale.h>
#include <kstaticdeleter.h>
#include <kdialog.h>
#include <kicon.h>
#include <kconfig.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kmimetype.h>
#include <kmenu.h>

extern "C"
{
    typedef QObject* (*def_module_func)();
}

using namespace Kross;

namespace Kross {

    /// @internal
    class Manager::Private
    {
        public:
            /// List of \a InterpreterInfo instances.
            QHash< QString, InterpreterInfo* > interpreterinfos;

            /// List of the interpreter names.
            QStringList interpreters;

            /// Loaded modules.
            QHash< QString, QPointer<QObject> > modules;

            /// The collection of \a Action instances.
            ActionCollection* collection;
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
    , d( new Private() )
{
    d->collection = new ActionCollection("main");

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
    } else {
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("Python interpreter for kross is unavailable");
        #endif
    }
#endif

#ifdef KROSS_RUBY_LIBRARY
    QString rubylib = QFile::encodeName( KLibLoader::self()->findLibrary(KROSS_RUBY_LIBRARY) );
    if(! rubylib.isEmpty()) { // If the Kross Ruby plugin exists we offer is as supported scripting language.
        InterpreterInfo::Option::Map rubyoptions;
        rubyoptions.insert("safelevel",
            new InterpreterInfo::Option(
                i18n("Level of safety of the Ruby interpreter"),
                QVariant(0) // 0 -> unsafe, 4 -> very safe
            )
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
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("Ruby interpreter for kross is unavailable");
        #endif
    }
#endif

#ifdef KROSS_KJS_LIBRARY
    QString kjslib = QFile::encodeName( KLibLoader::self()->findLibrary(KROSS_KJS_LIBRARY) );
    if(! kjslib.isEmpty()) { // If the Kjs plugin exists we offer is as supported scripting language.
        InterpreterInfo::Option::Map kjsoptions;
        kjsoptions.insert("restricted",
            new InterpreterInfo::Option(
                i18n("Restricted mode for untrusted scripts"),
                QVariant(true) // per default enabled
            )
        );
        d->interpreterinfos.insert("javascript",
            new InterpreterInfo("javascript",
                kjslib, // library
                "*.js", // file filter-wildcard
                QStringList() << "application/x-javascript", // mimetypes
                kjsoptions // options
            )
        );
    } else {
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("KDE JavaScript interpreter for kross is unavailable");
        #endif
    }
#endif

    // fill the list of supported interpreternames.
    QHash<QString, InterpreterInfo*>::Iterator it( d->interpreterinfos.begin() );
    for(; it != d->interpreterinfos.end(); ++it)
        d->interpreters << it.key();
    //d->interpreters.sort();

    // publish ourself.
    ChildrenInterface::addObject(this, "Kross");
}

Manager::~Manager()
{
    for(QHash<QString, InterpreterInfo* >::Iterator it = d->interpreterinfos.begin(); it != d->interpreterinfos.end(); ++it)
        delete it.value();
    for(QHash<QString, QPointer<QObject> >::Iterator it = d->modules.begin(); it != d->modules.end(); ++it)
        delete it.value();
    delete d->collection;
    delete d;
}

QHash< QString, InterpreterInfo* > Manager::interpreterInfos() const
{
    return d->interpreterinfos;
}

bool Manager::hasInterpreterInfo(const QString& interpretername) const
{
    return d->interpreterinfos.contains(interpretername);
}

InterpreterInfo* Manager::interpreterInfo(const QString& interpretername) const
{
    return d->interpreterinfos[interpretername];
}

const QString Manager::interpreternameForFile(const QString& file)
{
    QRegExp rx;
    rx.setPatternSyntax(QRegExp::Wildcard);
    for(QHash<QString, InterpreterInfo*>::Iterator it = d->interpreterinfos.begin(); it != d->interpreterinfos.end(); ++it) {
        rx.setPattern((*it)->wildcard());
        if( file.contains(rx) )
            return (*it)->interpreterName();
    }
    return QString();
}

Interpreter* Manager::interpreter(const QString& interpretername) const
{
    if(! d->interpreterinfos.contains(interpretername)) {
        krosswarning( QString("No such interpreter '%1'").arg(interpretername) );
        return 0;
    }
    return d->interpreterinfos[interpretername]->interpreter();
}

QStringList Manager::interpreters() const
{
    return d->interpreters;
}

ActionCollection* Manager::actionCollection() const
{
    return d->collection;
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
#if 0
        d->actioncollection->insert(action); //FIXME should we really remember the action?
#endif
    }
    return action;
}

QObject* Manager::module(const QString& modulename)
{
    if( d->modules.contains(modulename) ) {
        QObject* obj = d->modules[modulename];
        if( obj )
            return obj;
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
    lib->unload(); // unload the library

    if( ! module ) {
        krosswarning( QString("Failed to load module object '%1'").arg(modulename) );
        return 0;
    }

    //krossdebug( QString("Manager::module Module successfully loaded: modulename=%1 module.objectName=%2 module.className=%3").arg(modulename).arg(module->objectName()).arg(module->metaObject()->className()) );
    d->modules.insert(modulename, module);
    return module;
}

#include "manager.moc"
