/***************************************************************************
 * scriptaction.cpp
 * This file is part of the KDE project
 * copyright (C) 2005 by Sebastian Sauer (mail@dipe.org)
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

#include "scriptaction.h"
#include "manager.h"

#include <q3stylesheet.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <Q3ValueList>
#include <kurl.h>
#include <kicon.h>
#include <kstandarddirs.h>
#include <kmimetype.h>

using namespace Kross::Api;

namespace Kross { namespace Api {

    /// @internal
    class ScriptActionPrivate
    {
        public:
            /**
            * The packagepath is the directory that belongs to this
            * \a ScriptAction instance. If this \a ScriptAction points
            * to a scriptfile the packagepath will be the directory
            * the scriptfile is located in.
            */
            QString packagepath;

            /**
            * List of logs this \a ScriptAction has. Initialization,
            * execution and finalization should be logged for
            * example. So, the logs are usuabled to provide some
            * more detailed visual information to the user what
            * our \a ScriptAction did so far.
            */
            QStringList logs;

            /**
            * The versionnumber this \a ScriptAction has. We are using
            * the version to handle \a ScriptAction instances which
            * have the same unique \a ScriptAction::name() . If the name
            * is the same, we are able to use the version to determinate
            * which \a ScriptAction is newer / replaces the other.
            */
            int version;

            /**
            * The unique name the \a ScriptAction is reachable as.
            */
            QString name;

            /**
            * The description used to provide a way to the user to describe
            * the \a ScriptAction with a longer string.
            */
            QString description;

            /**
            * List of \a ScriptActionCollection instances this \a ScriptAction
            * is attached to.
            */
            Q3ValueList<ScriptActionCollection*> collections;

            /**
            * Constructor.
            */
            explicit ScriptActionPrivate() : version(0) {}
    };

}}

ScriptAction::ScriptAction(const QString& file)
    : KAction(0, file.toLatin1().data())
    , Kross::Api::ScriptContainer(file)
    , d( new ScriptActionPrivate() ) // initialize d-pointer class
{
    KUrl url(file);
    if(url.isLocalFile()) {
        setFile(file);
        setText(url.fileName());
        setIcon(KIcon(KMimeType::iconNameForURL(url)));
    }
    else {
        setText(file);
    }

    setDescription(file);
    setEnabled(false);
}

ScriptAction::ScriptAction(const QString& scriptconfigfile, const QDomElement& element)
    : KAction(0, "ScriptAction")
    , Kross::Api::ScriptContainer()
    , d( new ScriptActionPrivate() ) // initialize d-pointer class
{
    QString name = element.attribute("name");
    QString text = element.attribute("text");
    QString description = element.attribute("description");
    QString file = element.attribute("file");
    QString icon = element.attribute("icon");

    QString version = element.attribute("version");
    bool ok;
    int v = version.toInt(&ok);
    if(ok) d->version = v;

    if(file.isEmpty()) {
        if(text.isEmpty())
            text = name;
    }
    else {
        if(name.isEmpty())
            name = file;
        if(text.isEmpty())
            text = file;
    }

    //d->scriptcontainer = Manager::scriptManager()->getScriptContainer(name);

    QString interpreter = element.attribute("interpreter");
    if(interpreter.isNull())
        setEnabled(false);
    else
        setInterpreterName( interpreter );

    if(file.isNull()) {
        setCode( element.text().trimmed() );
        if(description.isNull())
            description = text;
        ScriptContainer::setName(name);
    }
    else {
        QDir dir = QFileInfo(scriptconfigfile).dir();
        d->packagepath = dir.absolutePath();
        QFileInfo fi(dir, file);
        file = fi.absoluteFilePath();
        setEnabled(fi.exists());
        setFile(file);
        if(icon.isNull()) {
            Kross::krossdebug( QString("FILE => %1").arg(file) );
            icon = KMimeType::iconNameForURL( KUrl::fromPath(file) );
        }
        if(description.isEmpty())
            description = QString("%1<br>%2").arg(text.isEmpty() ? name : text).arg(file);
        else
            description += QString("<br>%1").arg(file);
        ScriptContainer::setName(file);
    }

    d->name = name;
    KAction::setText(text);
    setDescription(description);
    KAction::setIcon(KIcon(icon));

    // connect signal
    connect(this, SIGNAL(activated()), this, SLOT(activate()));
}

ScriptAction::~ScriptAction()
{
    detachAll();
    delete d;
}

int ScriptAction::version() const
{
    return d->version;
}

const QString ScriptAction::getName() const
{
    return d->name;
}

void ScriptAction::setName(const QString& name)
{
    d->name = name;
}

const QString ScriptAction::getDescription() const
{
    return d->description;
}

void ScriptAction::setDescription(const QString& description)
{
    d->description = description;
    setToolTip( description );
    setWhatsThis( description );
}

void ScriptAction::setInterpreterName(const QString& name)
{
    setEnabled( Manager::scriptManager()->hasInterpreterInfo(name) );
    Kross::Api::ScriptContainer::setInterpreterName(name);
}

const QString ScriptAction::getPackagePath() const
{
    return d->packagepath;
}

const QStringList& ScriptAction::getLogs() const
{
    return d->logs;
}

void ScriptAction::attach(ScriptActionCollection* collection)
{
    d->collections.append( collection );
}

void ScriptAction::detach(ScriptActionCollection* collection)
{
    d->collections.remove( collection );
}

void ScriptAction::detachAll()
{
    for(Q3ValueList<ScriptActionCollection*>::Iterator it = d->collections.begin(); it != d->collections.end(); ++it)
        (*it)->detach( ScriptAction::Ptr(this) );
}

void ScriptAction::activate()
{
    emit activated(this);
    Kross::Api::ScriptContainer::execute();
    if( Kross::Api::ScriptContainer::hadException() ) {
        QString errormessage = Kross::Api::ScriptContainer::getException()->getError();
        QString tracedetails = Kross::Api::ScriptContainer::getException()->getTrace();
        d->logs << QString("<b>%1</b><br>%2")
                   .arg( Q3StyleSheet::escape(errormessage) )
                   .arg( Q3StyleSheet::escape(tracedetails) );
        emit failed(errormessage, tracedetails);
    }
    else {
        emit success();
    }
}

void ScriptAction::finalize()
{
    Kross::Api::ScriptContainer::finalize();
}

#include "scriptaction.moc"
