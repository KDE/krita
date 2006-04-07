/***************************************************************************
 * scriptguiclient.cpp
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

#include "scriptguiclient.h"
#include "manager.h"
#include "../api/interpreter.h"
#include "wdgscriptsmanager.h"

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kurl.h>
#include <ktar.h>
#include <kstandarddirs.h>

#include <kio/netaccess.h>
//Added by qt3to4:
#include <Q3CString>

using namespace Kross::Api;

namespace Kross { namespace Api {

    /// @internal
    class ScriptGUIClientPrivate
    {
        public:
            /**
             * The \a KXMLGUIClient that is parent of the \a ScriptGUIClient
             * instance.
             */
            KXMLGUIClient* guiclient;

            /**
             * The optional parent QWidget widget.
             */
            QWidget* parent;

            /**
             * Map of \a ScriptActionCollection instances the \a ScriptGUIClient
             * is attached to.
             */
            QMap<QString, ScriptActionCollection*> collections;
    };

}}

ScriptGUIClient::ScriptGUIClient(KXMLGUIClient* guiclient, QWidget* parent)
    : QObject( parent )
    , KXMLGUIClient( guiclient )
    , d( new ScriptGUIClientPrivate() ) // initialize d-pointer class
{
    krossdebug( QString("ScriptGUIClient::ScriptGUIClient() Ctor") );

    d->guiclient = guiclient;
    d->parent = parent;

    setInstance( ScriptGUIClient::instance() );

    // action to execute a scriptfile.
    new KAction(i18n("Execute Script File..."), 0, 0, this, SLOT(executeScriptFile()), actionCollection(), "executescriptfile");

    // acion to show the ScriptManagerGUI dialog.
    new KAction(i18n("Scripts Manager..."), 0, 0, this, SLOT(showScriptManager()), actionCollection(), "configurescripts");

    // The predefined ScriptActionCollection's this ScriptGUIClient provides.
    d->collections.replace("installedscripts",
        new ScriptActionCollection(i18n("Scripts"), actionCollection(), "installedscripts") );
    d->collections.replace("loadedscripts",
        new ScriptActionCollection(i18n("Loaded"), actionCollection(), "loadedscripts") );
    d->collections.replace("executedscripts",
        new ScriptActionCollection(i18n("History"), actionCollection(), "executedscripts") );

    reloadInstalledScripts();
}

ScriptGUIClient::~ScriptGUIClient()
{
    krossdebug( QString("ScriptGUIClient::~ScriptGUIClient() Dtor") );
    for(QMap<QString, ScriptActionCollection*>::Iterator it = d->collections.begin(); it != d->collections.end(); ++it)
        delete it.data();
    delete d;
}

bool ScriptGUIClient::hasActionCollection(const QString& name)
{
    return d->collections.contains(name);
}

ScriptActionCollection* ScriptGUIClient::getActionCollection(const QString& name)
{
    return d->collections[name];
}

QMap<QString, ScriptActionCollection*> ScriptGUIClient::getActionCollections()
{
    return d->collections;
}

void ScriptGUIClient::addActionCollection(const QString& name, ScriptActionCollection* collection)
{
    removeActionCollection(name);
    d->collections.replace(name, collection);
}

bool ScriptGUIClient::removeActionCollection(const QString& name)
{
    if(d->collections.contains(name)) {
        ScriptActionCollection* c = d->collections[name];
        d->collections.remove(name);
        delete c;
        return true;
    }
    return false;
}

void ScriptGUIClient::reloadInstalledScripts()
{
    ScriptActionCollection* installedcollection = d->collections["installedscripts"];
    if(installedcollection)
        installedcollection->clear();

    Q3CString partname = d->guiclient->instance()->instanceName();
    QStringList files = KGlobal::dirs()->findAllResources("data", partname + "/scripts/*/*.rc");
    //files.sort();
    for(QStringList::iterator it = files.begin(); it != files.end(); ++it)
        loadScriptConfigFile(*it);
}

bool ScriptGUIClient::installScriptPackage(const QString& scriptpackagefile)
{
    krossdebug( QString("Install script package: %1").arg(scriptpackagefile) );
    KTar archive( scriptpackagefile );
    if(! archive.open(QIODevice::ReadOnly)) {
        KMessageBox::sorry(0, i18n("Could not read the package \"%1\".").arg(scriptpackagefile));
        return false;
    }

    Q3CString partname = d->guiclient->instance()->instanceName();
    QString destination = KGlobal::dirs()->saveLocation("data", partname + "/scripts/", true);
    //QString destination = KGlobal::dirs()->saveLocation("appdata", "scripts", true);
    if(destination.isNull()) {
        krosswarning("ScriptGUIClient::installScriptPackage() Failed to determinate location where the scriptpackage should be installed to!");
        return false;
    }

    QString packagename = QFileInfo(scriptpackagefile).baseName();
    destination += packagename; // add the packagename to the name of the destination-directory.

    if( QDir(destination).exists() ) {
        if( KMessageBox::warningContinueCancel(0,
            i18n("A script package with the name \"%1\" already exists. Replace this package?" ).arg(packagename),
            i18n("Replace")) != KMessageBox::Continue )
                return false;

        if(! KIO::NetAccess::del(destination, 0) ) {
            KMessageBox::sorry(0, i18n("Could not uninstall this script package. You may not have sufficient permissions to delete the folder \"%1\".").arg(destination));
            return false;
        }
    }

    krossdebug( QString("Copy script-package to destination directory: %1").arg(destination) );
    const KArchiveDirectory* archivedir = archive.directory();
    archivedir->copyTo(destination, true);

    reloadInstalledScripts();
    return true;
}

bool ScriptGUIClient::uninstallScriptPackage(const QString& scriptpackagepath)
{
    if(! KIO::NetAccess::del(scriptpackagepath, 0) ) {
        KMessageBox::sorry(0, i18n("Could not uninstall this script package. You may not have sufficient permissions to delete the folder \"%1\".").arg(scriptpackagepath));
        return false;
    }
    reloadInstalledScripts();
    return true;
}

bool ScriptGUIClient::loadScriptConfigFile(const QString& scriptconfigfile)
{
    krossdebug( QString("ScriptGUIClient::loadScriptConfig file=%1").arg(scriptconfigfile) );

    QDomDocument domdoc;
    QFile file(scriptconfigfile);
    if(! file.open(QIODevice::ReadOnly)) {
        krosswarning( QString("ScriptGUIClient::loadScriptConfig(): Failed to read scriptconfigfile: %1").arg(scriptconfigfile) );
        return false;
    }
    bool ok = domdoc.setContent(&file);
    file.close();
    if(! ok) {
        krosswarning( QString("ScriptGUIClient::loadScriptConfig(): Failed to parse scriptconfigfile: %1").arg(scriptconfigfile) );
        return false;
    }

    return loadScriptConfigDocument(scriptconfigfile, domdoc);
}

bool ScriptGUIClient::loadScriptConfigDocument(const QString& scriptconfigfile, const QDomDocument &document)
{
    ScriptActionCollection* installedcollection = d->collections["installedscripts"];
    QDomNodeList nodelist = document.elementsByTagName("ScriptAction");
    uint nodelistcount = nodelist.count();
    for(uint i = 0; i < nodelistcount; i++) {
        ScriptAction::Ptr action = new ScriptAction(scriptconfigfile, nodelist.item(i).toElement());

        if(installedcollection) {
            ScriptAction::Ptr otheraction = installedcollection->action( action->name() );
            if(otheraction) {
                // There exists already an action with the same name. Use the versionnumber
                // to see if one of them is newer and if that's the case display only
                // the newer aka those with the highest version.
                if(action->version() < otheraction->version() && action->version() >= 0) {
                    // Just don't do anything with the above created action. The
                    // shared pointer will take care of freeing the instance.
                    continue;
                }
                else if(action->version() > otheraction->version() && otheraction->version() >= 0) {
                    // The previously added scriptaction isn't up-to-date any
                    // longer. Remove it from the list of installed scripts.
                    otheraction->finalize();
                    installedcollection->detach(otheraction);
                    //otheraction->detachAll() //FIXME: why it crashes with detachAll() ?
                }
                else {
                    // else just print a warning and fall through (so, install the action
                    // and don't care any longer of the duplicated name)...
                    krosswarning( QString("Kross::Api::ScriptGUIClient::loadScriptConfigDocument: There exists already a scriptaction with name \"%1\". Added anyway...").arg(action->name()) );
                }
            }
            installedcollection->attach( action );
        }

        connect(action.data(), SIGNAL( failed(const QString&, const QString&) ),
                this, SLOT( executionFailed(const QString&, const QString&) ));
        connect(action.data(), SIGNAL( success() ),
                this, SLOT( successfullyExecuted() ));
        connect(action.data(), SIGNAL( activated(const Kross::Api::ScriptAction*) ), SIGNAL( executionStarted(const Kross::Api::ScriptAction*)));
    }
    emit collectionChanged(installedcollection);
    return true;
}

void ScriptGUIClient::setXMLFile(const QString& file, bool merge, bool setXMLDoc)
{
    KXMLGUIClient::setXMLFile(file, merge, setXMLDoc);
}

void ScriptGUIClient::setDOMDocument(const QDomDocument &document, bool merge)
{
    ScriptActionCollection* installedcollection = d->collections["installedscripts"];
    if(! merge && installedcollection)
        installedcollection->clear();

    KXMLGUIClient::setDOMDocument(document, merge);
    loadScriptConfigDocument(xmlFile(), document);
}

void ScriptGUIClient::successfullyExecuted()
{
    const ScriptAction* action = dynamic_cast< const ScriptAction* >( QObject::sender() );
    if(action) {
        emit executionFinished(action);
        ScriptActionCollection* executedcollection = d->collections["executedscripts"];
        if(executedcollection) {
            ScriptAction* actionptr = const_cast< ScriptAction* >( action );
            executedcollection->detach(actionptr);
            executedcollection->attach(actionptr);
            emit collectionChanged(executedcollection);
        }
    }
}

void ScriptGUIClient::executionFailed(const QString& errormessage, const QString& tracedetails)
{
    const ScriptAction* action = dynamic_cast< const ScriptAction* >( QObject::sender() );
    if(action)
        emit executionFinished(action);
    if(tracedetails.isEmpty())
        KMessageBox::error(0, errormessage);
    else
        KMessageBox::detailedError(0, errormessage, tracedetails);
}

KUrl ScriptGUIClient::openScriptFile(const QString& caption)
{
    QStringList mimetypes;
    QMap<QString, InterpreterInfo*> infos = Manager::scriptManager()->getInterpreterInfos();
    for(QMap<QString, InterpreterInfo*>::Iterator it = infos.begin(); it != infos.end(); ++it)
        mimetypes.append( it.data()->getMimeTypes().join(" ").stripWhiteSpace() );

    KFileDialog* filedialog = new KFileDialog(
        QString::null, // startdir
        mimetypes.join(" "), // filter
        0, // parent widget
        "ScriptGUIClientFileDialog", // name
        true // modal
    );
    if(! caption.isNull())
        filedialog->setCaption(caption);
    if( filedialog->exec() )
        return filedialog->selectedURL();
    return KUrl();
}

bool ScriptGUIClient::loadScriptFile()
{
    KUrl url = openScriptFile( i18n("Load Script File") );
    if(url.isValid()) {
        ScriptActionCollection* loadedcollection = d->collections["loadedscripts"];
        if(loadedcollection) {
            ScriptAction::Ptr action = new ScriptAction( url.path() );
            connect(action.data(), SIGNAL( failed(const QString&, const QString&) ),
                    this, SLOT( executionFailed(const QString&, const QString&) ));
            connect(action.data(), SIGNAL( success() ),
                    this, SLOT( successfullyExecuted() ));
            connect(action.data(), SIGNAL( activated(const Kross::Api::ScriptAction*) ), SIGNAL( executionStarted(const Kross::Api::ScriptAction*)));

            loadedcollection->detach(action);
            loadedcollection->attach(action);
            return true;
        }
    }
    return false;
}

bool ScriptGUIClient::executeScriptFile()
{
    KUrl url = openScriptFile( i18n("Execute Script File") );
    if(url.isValid())
        return executeScriptFile( url.path() );
    return false;
}

bool ScriptGUIClient::executeScriptFile(const QString& file)
{
    krossdebug( QString("Kross::Api::ScriptGUIClient::executeScriptFile() file='%1'").arg(file) );

    ScriptAction::Ptr action = new ScriptAction(file);
    return executeScriptAction(action);
}

bool ScriptGUIClient::executeScriptAction(ScriptAction::Ptr action)
{
    connect(action.data(), SIGNAL( failed(const QString&, const QString&) ),
            this, SLOT( executionFailed(const QString&, const QString&) ));
    connect(action.data(), SIGNAL( success() ),
            this, SLOT( successfullyExecuted() ));
    connect(action.data(), SIGNAL( activated(const Kross::Api::ScriptAction*) ), SIGNAL( executionStarted(const Kross::Api::ScriptAction*)));
    action->activate();
    bool ok = action->hadException();
    action->finalize(); // execution is done.
    return ok;
}

void ScriptGUIClient::showScriptManager()
{
    KDialogBase* dialog = new KDialogBase(d->parent, "", true, i18n("Scripts Manager"), KDialogBase::Close);
    WdgScriptsManager* wsm = new WdgScriptsManager(this, dialog);
    dialog->setMainWidget(wsm);
    dialog->resize( QSize(360, 320).expandedTo(dialog->minimumSizeHint()) );
    dialog->show();
}

#include "scriptguiclient.moc"
