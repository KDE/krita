/***************************************************************************
 * guiclient.cpp
 * This file is part of the KDE project
 * copyright (C) 2005-2006 by Sebastian Sauer (mail@dipe.org)
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

#include "guiclient.h"
#include "manager.h"

#include "../core/interpreter.h"

#include <QRegExp>
#include <qdom.h>

#include <kapplication.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kmenu.h>
//#include <kdialog.h>
#include <kstandarddirs.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kurl.h>
#include <kicon.h>
#include <kstandarddirs.h>

using namespace Kross;

namespace Kross {

    /// \internal d-pointer class.
    class GUIClient::Private
    {
        public:
            /// The \a KXMLGUIClient that is parent of the \a GUIClient instance.
            KXMLGUIClient* guiclient;
            /// The optional parent QWidget widget.
            QWidget* parent;
            /// The collection of installed script-packages.
            KActionCollection* actions;
            /// The menu used to display the scripts.
            KActionMenu* scriptsmenu;
    };

}

GUIClient::GUIClient(KXMLGUIClient* guiclient, QWidget* parent)
    : QObject(parent)
    , KXMLGUIClient(guiclient)
    , d(new Private())
{
    setInstance( GUIClient::instance() );

    d->guiclient = guiclient;
    d->parent = parent;
    d->actions = Manager::self().actionCollection();

    d->scriptsmenu = new KActionMenu(i18n("Scripts"), actionCollection(), "scripts");
    connect(d->scriptsmenu->menu(), SIGNAL(aboutToShow()), this, SLOT(slotMenuAboutToShow()));
    //connect(d->actions, SIGNAL(inserted(KAction*)), scriptsmenu, SLOT(addAction(QAction*)));
    //connect(d->actions, SIGNAL(removed(KAction*)), scriptsmenu, SLOT(removeAction(QAction*)));

    // action to execute a scriptfile.
    KAction* execfileaction = new KAction(i18n("Execute Script File..."), actionCollection(), "executescriptfile");
    connect(execfileaction, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), SLOT(executeFile()));

    // acion to show the ScriptManagerGUI dialog.
    KAction* manageraction =  new KAction(i18n("Script Manager..."), actionCollection(), "configurescripts");
    connect(manageraction, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), SLOT(showManager()));

    // read the script-actions.
    if (! Manager::self().readConfig()) {
        // if there is no scripts-section in the configfile yet, we assume that the user uses a fresh
        // configfile. So, we just try to read all available script-packages and add them to the config.
        if (writeConfigFromPackages())
            Manager::self().readConfig();
    }

    // The GUIClient provides feedback if e.g. an execution failed.
    connect(&Manager::self(), SIGNAL( started(Kross::Action*) ), this, SLOT( started(Kross::Action*) ));
    connect(&Manager::self(), SIGNAL( finished(Kross::Action*) ), this, SLOT( finished(Kross::Action*) ));
}

GUIClient::~GUIClient()
{
    delete d;
}

void GUIClient::setXMLFile(const QString& file, bool merge, bool setXMLDoc)
{
    KXMLGUIClient::setXMLFile(file, merge, setXMLDoc);
}

void GUIClient::setDOMDocument(const QDomDocument &document, bool merge)
{
    //ActionCollection* installedcollection = d->collections["installedscripts"];
    //if(! merge && installedcollection) installedcollection->clear();

    KXMLGUIClient::setDOMDocument(document, merge);
    //loadScriptConfigDocument(xmlFile(), document);
}

KActionCollection* GUIClient::scriptsActionCollection() const
{
    return d->actions;
}

bool GUIClient::writeConfigFromPackages()
{
    KConfig* config = KApplication::kApplication()->sessionConfig();
    krossdebug( QString("GUIClient::readConfigFromPackages hasGroup=%1 isReadOnly=%2 isImmutable=%3 ConfigState=%4").arg(config->hasGroup("scripts")).arg(config->isReadOnly()).arg(config->isImmutable()).arg(config->getConfigState()) );
    if(config->isReadOnly())
        return false;

    config->setGroup("scripts");
    QStringList names = config->readEntry("names", QStringList());

    QByteArray partname = d->guiclient->instance()->instanceName();
    QStringList files = KGlobal::dirs()->findAllResources("data", partname + "/scripts/*/install.rc");
    files.sort();
    foreach(QString file, files) {
        krossdebug( QString("GUIClient::readConfigFromPackages trying to read \"%1\"").arg(file) );
        QFile f(file);
        if(! f.open(QIODevice::ReadOnly)) {
            krossdebug( QString("GUIClient::readAllConfigs reading \"%1\" failed. Skipping package.").arg(file) );
            continue;
        }

        QDomDocument domdoc;
        bool ok = domdoc.setContent(&f);
        f.close();
        if(! ok) {
            krossdebug( QString("GUIClient::readConfigFromPackages parsing \"%1\" failed. Skipping package.").arg(file) );
            continue;
        }

        QDomNodeList nodelist = domdoc.elementsByTagName("ScriptAction");
        int nodelistcount = nodelist.count();
        for(int i = 0; i < nodelistcount; ++i) {
            QDomElement element = nodelist.item(i).toElement();
            const QString name = element.attribute("name");
            if(d->actions->action(name) != 0) {
                // if the script-package is already in the list of actions, it's an
                // already enabled one and therefore it's not needed to add it again.
                continue;
            }

            names << name;
            config->writeEntry(QString("%1_text").arg(name).toLatin1(), element.attribute("text"));
            config->writeEntry(QString("%1_description").arg(name).toLatin1(), element.attribute("description"));
            config->writeEntry(QString("%1_icon").arg(name).toLatin1(), element.attribute("icon"));
            config->writeEntry(QString("%1_interpreter").arg(name).toLatin1(), element.attribute("interpreter"));

            QString f = element.attribute("file");
            QFileInfo fi(f);
            if(! QFileInfo(f).exists()) {
                const QDir packagepath = QFileInfo(file).dir();
                QFileInfo fi2(packagepath, f);
                if( fi2.exists() ) {
                    f = fi2.absoluteFilePath();
                }
                else {
                    QString resource = KGlobal::dirs()->findResource("appdata", QString("scripts/%1/%2").arg(name).arg(f));
                    if( ! resource.isNull() )
                        f = resource;
                }
            }
            config->writeEntry(QString("%1_file").arg(name).toLatin1(), f);
        }
    }

    config->writeEntry("names", names);
    config->sync();
    return true;
}

#if 0
void GUIClient::setXMLFile(const QString& file, bool merge, bool setXMLDoc)
{
    KXMLGUIClient::setXMLFile(file, merge, setXMLDoc);
}
void GUIClient::setDOMDocument(const QDomDocument &document, bool merge)
{
    ActionCollection* installedcollection = d->collections["installedscripts"];
    if(! merge && installedcollection) installedcollection->clear();
    KXMLGUIClient::setDOMDocument(document, merge);
    loadScriptConfigDocument(xmlFile(), document);
}
#endif

void GUIClient::slotMenuAboutToShow()
{
    d->scriptsmenu->menu()->clear();
    foreach(KAction* a, Manager::self().actionCollection()->actions())
        d->scriptsmenu->menu()->addAction(a);
}

void GUIClient::started(Kross::Action* action)
{
    Q_UNUSED(action);
    krossdebug( QString("GUIClient::started(Kross::Action*) name='%1'").arg(action->objectName()) );
}

void GUIClient::finished(Kross::Action* action)
{
    krossdebug( QString("GUIClient::finished(Kross::Action*) name='%1'").arg(action->objectName()) );
    if( action->hadError() ) {
        if( action->errorTrace().isNull() )
            KMessageBox::error(0, action->errorMessage());
        else
            KMessageBox::detailedError(0, action->errorMessage(), action->errorTrace());
    }
    //emit executionFinished(action);
}

bool GUIClient::executeFile()
{
    QStringList mimetypes;
    QMap<QString, InterpreterInfo*> infos = Manager::self().interpreterInfos();
    for(QMap<QString, InterpreterInfo*>::Iterator it = infos.begin(); it != infos.end(); ++it)
        mimetypes.append( it.value()->mimeTypes().join(" ").trimmed() );

    KFileDialog* filedialog = new KFileDialog(
        KUrl("kfiledialog:///KrossExecuteScript"), // startdir
        mimetypes.join(" "), // filter
        0, // custom widget
        0 // parent
    );
    filedialog->setCaption( i18n("Execute Script File") );
    filedialog->setOperationMode( KFileDialog::Opening );
    filedialog->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
    return filedialog->exec() ? executeFile(filedialog->selectedUrl()) : false;
}

bool GUIClient::executeFile(const KUrl& file)
{
    krossdebug( QString("GUIClient::executeFile() file='%1'").arg(file.path()) );
    Action* action = new Action( file.path() );
    action->trigger();
    bool ok = ! action->hadError();
    delete action;
    return ok;
}

void GUIClient::showManager()
{
    krossdebug( QString("GUIClient::showManagerDialog()") );
    QObject* obj = Manager::self().module("scriptmanager");
    if( obj ) {
        if( QMetaObject::invokeMethod(obj, "showManagerDialog") )
            return; // successfully called the method.
        krosswarning( QString("GUIClient::showManagerDialog() No such method.") );
    }
    KMessageBox::sorry(0, i18n("Failed to load the Script Manager."));
}

#include "guiclient.moc"
