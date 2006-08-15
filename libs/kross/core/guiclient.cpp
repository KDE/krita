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
#include "../core/interpreter.h"
#include "manager.h"

#include <kapplication.h>
#include <kdialog.h>
#include <kstandarddirs.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kurl.h>
#include <ktar.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>

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

            Private(KXMLGUIClient* g, QWidget* p) : guiclient(g), parent(p) {}
    };

}

GUIClient::GUIClient(KXMLGUIClient* guiclient, QWidget* parent)
    : QObject( parent )
    , KXMLGUIClient(guiclient)
    , d(new Private(guiclient, parent))
{
    setInstance( GUIClient::instance() );

    // action to execute a scriptfile.
    KAction* execfileaction = new KAction(i18n("Execute Script File..."), actionCollection(), "executescriptfile");
    connect(execfileaction, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), this, SLOT(executeFile()));

    // acion to show the ScriptManagerGUI dialog.
    KAction* manageraction =  new KAction(i18n("Scripts Manager..."), actionCollection(), "configurescripts");
    connect(manageraction, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), this, SLOT(showManager()));

#if 0
    // The predefined ActionCollection's this GUIClient provides.
    d->collections.insert("installedscripts",
        new ActionCollection(i18n("Scripts"), actionCollection(), "installedscripts") );
    d->collections.insert("loadedscripts",
        new ActionCollection(i18n("Loaded"), actionCollection(), "loadedscripts") );
    d->collections.insert("executedscripts",
        new ActionCollection(i18n("History"), actionCollection(), "executedscripts") );

    reloadInstalledScripts();
#endif
}

GUIClient::~GUIClient()
{
#if 0
    for(QMap<QString, ActionCollection*>::Iterator it = d->collections.begin(); it != d->collections.end(); ++it)
        delete it.value();
#endif
    delete d;
}

void GUIClient::setXMLFile(const QString& file, bool merge, bool setXMLDoc)
{
    KXMLGUIClient::setXMLFile(file, merge, setXMLDoc);
}

void GUIClient::setDOMDocument(const QDomDocument &document, bool merge)
{
    /*
    ActionCollection* installedcollection = d->collections["installedscripts"];
    if(! merge && installedcollection)
        installedcollection->clear();

    KXMLGUIClient::setDOMDocument(document, merge);
    loadScriptConfigDocument(xmlFile(), document);
    */
}

#if 0
bool GUIClient::hasActionCollection(const QString& name)
{
    return d->collections.contains(name);
}

ActionCollection* GUIClient::getActionCollection(const QString& name)
{
    return d->collections[name];
}

QMap<QString, ActionCollection*> GUIClient::getActionCollections()
{
    return d->collections;
}

void GUIClient::addActionCollection(const QString& name, ActionCollection* collection)
{
    removeActionCollection(name);
    d->collections.insert(name, collection);
}

bool GUIClient::removeActionCollection(const QString& name)
{
    if(d->collections.contains(name)) {
        ActionCollection* c = d->collections[name];
        d->collections.remove(name);
        delete c;
        return true;
    }
    return false;
}

void GUIClient::reloadInstalledScripts()
{
    ActionCollection* installedcollection = d->collections["installedscripts"];
    if(installedcollection)
        installedcollection->clear();

    QByteArray partname = d->guiclient->instance()->instanceName();
    QStringList files = KGlobal::dirs()->findAllResources("data", partname + "/scripts/*/*.rc");
    //files.sort();
    for(QStringList::iterator it = files.begin(); it != files.end(); ++it)
        loadScriptConfigFile(*it);
}

bool GUIClient::installScriptPackage(const QString& scriptpackagefile)
{
    krossdebug( QString("Install script package: %1").arg(scriptpackagefile) );
    KTar archive( scriptpackagefile );
    if(! archive.open(QIODevice::ReadOnly)) {
        KMessageBox::sorry(0, i18n("Could not read the package \"%1\".", scriptpackagefile));
        return false;
    }

    QByteArray partname = d->guiclient->instance()->instanceName();
    QString destination = KGlobal::dirs()->saveLocation("data", partname + "/scripts/", true);
    //QString destination = KGlobal::dirs()->saveLocation("appdata", "scripts", true);
    if(destination.isNull()) {
        krosswarning("GUIClient::installScriptPackage() Failed to determinate location where the scriptpackage should be installed to!");
        return false;
    }

    QString packagename = QFileInfo(scriptpackagefile).baseName();
    destination += packagename; // add the packagename to the name of the destination-directory.

    if( QDir(destination).exists() ) {
        if( KMessageBox::warningContinueCancel(0,
            i18n("A script package with the name \"%1\" already exists. Replace this package?" , packagename),
            i18n("Replace")) != KMessageBox::Continue )
                return false;

        if(! KIO::NetAccess::del(destination, 0) ) {
            KMessageBox::sorry(0, i18n("Could not uninstall this script package. You may not have sufficient permissions to delete the folder \"%1\".", destination));
            return false;
        }
    }

    krossdebug( QString("Copy script-package to destination directory: %1").arg(destination) );
    const KArchiveDirectory* archivedir = archive.directory();
    archivedir->copyTo(destination, true);

    reloadInstalledScripts();
    return true;
}

bool GUIClient::uninstallScriptPackage(const QString& scriptpackagepath)
{
    if(! KIO::NetAccess::del(scriptpackagepath, 0) ) {
        KMessageBox::sorry(0, i18n("Could not uninstall this script package. You may not have sufficient permissions to delete the folder \"%1\".", scriptpackagepath));
        return false;
    }
    reloadInstalledScripts();
    return true;
}

bool GUIClient::loadScriptConfigFile(const QString& scriptconfigfile)
{
    krossdebug( QString("GUIClient::loadScriptConfig file=%1").arg(scriptconfigfile) );

    QDomDocument domdoc;
    QFile file(scriptconfigfile);
    if(! file.open(QIODevice::ReadOnly)) {
        krosswarning( QString("GUIClient::loadScriptConfig(): Failed to read scriptconfigfile: %1").arg(scriptconfigfile) );
        return false;
    }
    bool ok = domdoc.setContent(&file);
    file.close();
    if(! ok) {
        krosswarning( QString("GUIClient::loadScriptConfig(): Failed to parse scriptconfigfile: %1").arg(scriptconfigfile) );
        return false;
    }

    return loadScriptConfigDocument(scriptconfigfile, domdoc);
}

bool GUIClient::loadScriptConfigDocument(const QString& scriptconfigfile, const QDomDocument &document)
{
    ActionCollection* installedcollection = d->collections["installedscripts"];
    QDomNodeList nodelist = document.elementsByTagName("Action");
    uint nodelistcount = nodelist.count();
    for(uint i = 0; i < nodelistcount; i++) {
        Action::Ptr action = Action::Ptr( new Action(scriptconfigfile, nodelist.item(i).toElement()) );

        if(installedcollection) {
            Action::Ptr otheraction = installedcollection->action( action->objectName() );
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
                    krosswarning( QString("GUIClient::loadScriptConfigDocument: There exists already a scriptaction with name \"%1\". Added anyway...").arg(action->objectName()) );
                }
            }
            installedcollection->attach( action );
        }

        connect(action.data(), SIGNAL( failed(const QString&, const QString&) ),
                this, SLOT( executionFailed(const QString&, const QString&) ));
        connect(action.data(), SIGNAL( success() ),
                this, SLOT( successfullyExecuted() ));
        connect(action.data(), SIGNAL( activated(const Kross::Action*) ), SIGNAL( executionStarted(const Kross::Action*)));
    }
    emit collectionChanged(installedcollection);
    return true;
}

void GUIClient::setXMLFile(const QString& file, bool merge, bool setXMLDoc)
{
    KXMLGUIClient::setXMLFile(file, merge, setXMLDoc);
}

void GUIClient::setDOMDocument(const QDomDocument &document, bool merge)
{
    ActionCollection* installedcollection = d->collections["installedscripts"];
    if(! merge && installedcollection)
        installedcollection->clear();

    KXMLGUIClient::setDOMDocument(document, merge);
    loadScriptConfigDocument(xmlFile(), document);
}
#endif

void GUIClient::executionSuccessfull()
{
    const Action* action = dynamic_cast< const Action* >( QObject::sender() );
    if(action) {
        emit executionFinished(action);
#if 0
        ActionCollection* executedcollection = d->collections["executedscripts"];
        if(executedcollection) {
            Action* actionptr = const_cast< Action* >( action );
            executedcollection->detach( Action::Ptr(actionptr) );
            executedcollection->attach( Action::Ptr(actionptr) );
            emit collectionChanged(executedcollection);
        }
#endif
    }
}

void GUIClient::executionFailed(const QString& errormessage, const QString& tracedetails)
{
    const Action* action = dynamic_cast< const Action* >( QObject::sender() );
    if(action)
        emit executionFinished(action);
    if(tracedetails.isEmpty())
        KMessageBox::error(0, errormessage);
    else
        KMessageBox::detailedError(0, errormessage, tracedetails);
}

KUrl GUIClient::openFile(const QString& caption)
{
    QStringList mimetypes;
    QMap<QString, InterpreterInfo*> infos = Manager::self().getInterpreterInfos();
    for(QMap<QString, InterpreterInfo*>::Iterator it = infos.begin(); it != infos.end(); ++it)
        mimetypes.append( it.value()->getMimeTypes().join(" ").trimmed() );

    KFileDialog* filedialog = new KFileDialog(
        QString(), // startdir
        mimetypes.join(" "), // filter
        0, // widget
        0 // parent
    );
    filedialog->setCaption(caption);
    return filedialog->exec() ? filedialog->selectedUrl() : KUrl();
}

bool GUIClient::executeFile()
{
    KUrl url = openFile( i18n("Execute Script File") );
    if(url.isValid())
        return executeFile(url);
    return false;
}

bool GUIClient::executeFile(const KUrl& file)
{
    krossdebug( QString("GUIClient::executeFile() file='%1'").arg(file) );
    return executeAction( Action::Ptr( new Action(file) ) );
}

#if 0
bool GUIClient::loadFile()
{
    KUrl url = openScriptFile( i18n("Load Script File") );
    if(url.isValid()) {
        ActionCollection* loadedcollection = d->collections["loadedscripts"];
        if(loadedcollection) {
            Action::Ptr action = Action::Ptr( new Action( url.path() ) );
            connect(action.data(), SIGNAL( failed(const QString&, const QString&) ),
                    this, SLOT( executionFailed(const QString&, const QString&) ));
            connect(action.data(), SIGNAL( success() ),
                    this, SLOT( successfullyExecuted() ));
            connect(action.data(), SIGNAL( activated(const Kross::Action*) ), SIGNAL( executionStarted(const Kross::Action*)));

            loadedcollection->detach(action);
            loadedcollection->attach(action);
            return true;
        }
    }
    return false;
}
#endif

bool GUIClient::executeAction(Action::Ptr action)
{
    connect(action.data(), SIGNAL( failed(const QString&, const QString&) ), this, SLOT( executionFailed(const QString&, const QString&) ));
    connect(action.data(), SIGNAL( success() ), this, SLOT( executionSuccessfull() ));

    connect(action.data(), SIGNAL( activated(const Kross::Action*) ), SIGNAL( executionStarted(const Kross::Action*)));

    action->trigger(); // activate the action and execute the script that way

    bool ok = action->hadError();
    action->finalize(); // execution is done.
    return ok;
}

void GUIClient::showManager()
{
#if 0
    KDialog* dialog = new KDialog( d->parent );
    dialog->setCaption( i18n("Scripts Manager") );
    dialog->setModal( true );
    dialog->setButtons( KDialog::Ok );
    dialog->showButtonSeparator( false );

    //KDialogBase* dialog = new KDialogBase(d->parent, "", true, i18n("Scripts Manager"), KDialogBase::Close);
    WdgScriptsManager* wsm = new WdgScriptsManager(this, dialog);
    dialog->setMainWidget(wsm);
    dialog->resize( QSize(360, 320).expandedTo(dialog->minimumSizeHint()) );
    dialog->show();
#endif
}

#include "guiclient.moc"
