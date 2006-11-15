/***************************************************************************
 * action.cpp
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

#include "action.h"
#include "interpreter.h"
#include "script.h"
#include "manager.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDomElement>

#include <klocale.h>
#include <kicon.h>
#include <kmimetype.h>
#include <kstandarddirs.h>

using namespace Kross;

namespace Kross {

    /// \internal d-pointer class.
    class Action::Private
    {
        public:

            /**
            * The \a Script instance the \a Action uses if initialized. It will
            * be NULL as long as we didn't initialized it what will be done on
            * demand.
            */
            Script* script;

            /**
            * The optional description to provide some more details about the
            * Action to the user.
            */
            QString description;

            /**
            * The scripting code.
            */
            QString code;

            /**
            * The name of the interpreter. This could be something
            * like for example "python" for the python binding.
            */
            QString interpretername;

            /**
            * The name of the scriptfile that should be executed. Those
            * scriptfile will be readed and the content will be used to
            * set the scripting code and, if not defined, the used
            * interpreter.
            */
            QString scriptfile;

            /**
            * The current path the \a Script is running in or
            * an empty string if there is no path current defined.
            */
            QString currentpath;

            /**
            * Map of options that overwritte the \a InterpreterInfo::Option::Map
            * standard options.
            */
            QMap<QString, QVariant> options;

            Private() : script(0) {}
    };

}

Action::Action(KActionCollection* collection, const QString& name)
    : KAction( collection, name )
    , ChildrenInterface()
    , ErrorInterface()
    , d( new Private() )
{
    setEnabled( false );
}

Action::Action(const QString& file)
    : KAction( 0 /* no parent KActionCollection */, file )
    , ChildrenInterface()
    , ErrorInterface()
    , d( new Private() )
{
    KUrl url(file);
    setText( url.fileName() );
    setIcon( KIcon(KMimeType::iconNameForUrl(url)) );
    setFile( file );
}

Action::Action(KActionCollection* collection, const QDomElement& element, const QDir& packagepath)
    : KAction( collection, element.attribute("name") )
    , ChildrenInterface()
    , ErrorInterface()
    , d( new Private() )
{
    setText( element.attribute("text") );
    setDescription( element.attribute("description") );
    setInterpreter( element.attribute("interpreter") );

    QString file = element.attribute("file");
    if( ! file.isEmpty() ) {
        if(! QFileInfo(file).exists()) {
            QFileInfo fi(packagepath, file);
            if(fi.exists())
                file = fi.absoluteFilePath();
            else
                setEnabled(false);
        }
        setFile(file);
    }
    else {
        d->currentpath = packagepath.absolutePath();
    }

    QString icon = element.attribute("icon");
    if( icon.isEmpty() && ! d->scriptfile.isNull() )
        icon = KMimeType::iconNameForUrl( KUrl(d->scriptfile) );
    setIcon( KIcon(icon) );
}

Action::~Action()
{
    krossdebug( QString("Action::~Action() Dtor name='%1'").arg(objectName()) );
    finalize();
    delete d;
}

QString Action::description() const
{
    return d->description;
}

void Action::setDescription(const QString& description)
{
    d->description = description;
}

QString Action::code() const
{
    return d->code;
}

void Action::setCode(const QString& code)
{
    if( d->code != code ) {
        finalize();
        d->code = code;
    }
}

QString Action::interpreter() const
{
    return d->interpretername;
}

void Action::setInterpreter(const QString& interpretername)
{
    if( d->interpretername != interpretername ) {
        finalize();
        d->interpretername = interpretername;
        setEnabled( Manager::self().interpreters().contains(interpretername) );
    }
}

QString Action::file() const
{
    return d->scriptfile;
}

bool Action::setFile(const QString& scriptfile)
{
    if( d->scriptfile != scriptfile ) {
        finalize();
        if ( scriptfile.isNull() ) {
            if( ! d->scriptfile.isNull() )
                d->interpretername.clear();
            d->scriptfile.clear();
            d->currentpath.clear();
        }
        else {
            d->scriptfile = scriptfile;
            d->currentpath = QFileInfo(scriptfile).absolutePath();
            d->interpretername = Manager::self().interpreternameForFile(scriptfile);
            if( d->interpretername.isNull() )
                return false;
        }
    }
    return true;
}

QString Action::currentPath() const
{
    return d->currentpath;
}

QMap<QString, QVariant>& Action::options() const
{
    return d->options;
}

QVariant Action::option(const QString name, QVariant defaultvalue)
{
    if(d->options.contains(name))
        return d->options[name];
    InterpreterInfo* info = Manager::self().interpreterInfo( d->interpretername );
    return info ? info->optionValue(name, defaultvalue) : defaultvalue;
}

bool Action::setOption(const QString name, const QVariant& value)
{
    InterpreterInfo* info = Manager::self().interpreterInfo( d->interpretername );
    if(info) {
        if(info->hasOption(name)) {
            d->options.insert(name, value);
            return true;
        } else krosswarning( QString("Kross::Action::setOption(%1, %2): No such option").arg(name).arg(value.toString()) );
    } else krosswarning( QString("Kross::Action::setOption(%1, %2): No such interpreterinfo").arg(name).arg(value.toString()) );
    return false;
}

QStringList Action::functionNames()
{
    if(! d->script) {
        if(! initialize())
            return QStringList();
    }
    return d->script->functionNames();
}

QVariant Action::callFunction(const QString& name, const QVariantList& args)
{
    if(! d->script) {
        if(! initialize())
            return QVariant();
    }
    return d->script->callFunction(name, args);
}

bool Action::initialize()
{
    finalize();

    if( ! d->scriptfile.isNull() ) {
        QFile f( d->scriptfile );
        if( ! f.exists() ) {
            setError(i18n("There exists no such scriptfile '%1'", d->scriptfile));
            return false;
        }
        if( d->interpretername.isNull() ) {
            setError(i18n("Failed to determinate interpreter for scriptfile '%1'", d->scriptfile));
            return false;
        }
        if( ! f.open(QIODevice::ReadOnly) ) {
            setError(i18n("Failed to open scriptfile '%1'", d->scriptfile));
            return false;
        }
        d->code = QString( f.readAll() );
        f.close();
    }

    Interpreter* interpreter = Manager::self().interpreter(d->interpretername);
    if( ! interpreter ) {
        setError(i18n("Unknown interpreter '%1'", d->interpretername));
        return false;
    }

    d->script = interpreter->createScript(this);
    if( ! d->script ) {
        setError(i18n("Failed to create script for interpreter '%1'", d->interpretername));
        return false;
    }

    if( d->script->hadError() ) {
        setError(d->script);
        finalize();
        return false;
    }

    clearError(); // clear old exception
    return true;
}

void Action::finalize()
{
    delete d->script;
    d->script = 0;
}

void Action::slotTriggered()
{
    //krossdebug( QString("Action::slotTriggered()") );
    emit started(this);

    if( ! d->script ) {
        if( ! initialize() )
            Q_ASSERT( hadError() );
    }

    if( hadError() ) {
        krossdebug( QString("Action::slotTriggered() name=%1 errorMessage=%2").arg(objectName()).arg(errorMessage()) );
        emit finished(this);
        return;
    }

    d->script->execute();
    if( d->script->hadError() ) {
        setError(d->script);
        finalize();
    }

    emit finished(this);
}

#include "action.moc"
