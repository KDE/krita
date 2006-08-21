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
#include "guimanager.h"

#include <QRegExp>

#include <kapplication.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
//#include <kdialog.h>
#include <kstandarddirs.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kurl.h>
#include <kicon.h>
#include <kstandarddirs.h>
#include <ktar.h>
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
    d->actions = new KActionCollection(this, instance());

    // action to execute a scriptfile.
    KAction* execfileaction = new KAction(i18n("Execute Script File..."), actionCollection(), "executescriptfile");
    connect(execfileaction, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), this, SLOT(executeFile()));

    // acion to show the ScriptManagerGUI dialog.
    KAction* manageraction =  new KAction(i18n("Scripts Manager..."), actionCollection(), "configurescripts");
    connect(manageraction, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), this, SLOT(showManager()));

    d->scriptsmenu = new KActionMenu(i18n("Scripts"), actionCollection(), "scripts");
    //connect(d->actions, SIGNAL(inserted(KAction*)), scriptsmenu, SLOT(addAction(QAction*)));
    //connect(d->actions, SIGNAL(removed(KAction*)), scriptsmenu, SLOT(removeAction(QAction*)));

    // read the script-actions.
    readConfig( instance()->config() );

    //TESTCASE
    //installPackage( KUrl("/home/kde4/koffice/libs/kross/test/Archiv.tar.gz") );
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

void GUIClient::readConfig(KConfig* config)
{
    krossdebug( QString("GUIClient::read hasGroup=%1 isReadOnly=%2 isImmutable=%3 ConfigState=%4").arg(config->hasGroup("scripts")).arg(config->isReadOnly()).arg(config->isImmutable()).arg(config->getConfigState()) );

    config->setGroup("scripts");
    foreach(QString name, config->readEntry("names", QStringList())) {
        QString text = config->readEntry(QString("%1_text").arg(name).toLatin1());
        QString description = config->readEntry(QString("%1_description").arg(name).toLatin1());
        QString icon = config->readEntry(QString("%1_icon").arg(name).toLatin1());
        QString file = config->readEntry(QString("%1_file").arg(name).toLatin1());
        QString interpreter = config->readEntry(QString("%1_interpreter").arg(name).toLatin1());

        QFileInfo fi(file);
        if(! fi.exists()) {
            QString resource = KGlobal::dirs()->findResource("appdata", QString("scripts/%1/%2").arg(name).arg(file));
            if(! resource.isNull())
                file = resource;
        }

        if(text.isNull())
            text = file;

        if(description.isEmpty())
            description = QString("%1<br>%2").arg(text.isEmpty() ? name : text).arg(file);
        else
            description += QString("<br>%1").arg(file);

        if(icon.isNull())
            icon = KMimeType::iconNameForURL( KUrl(file) );

        krossdebug( QString("GUIClient::readConfig Add scriptaction name='%1' file='%2'").arg(name).arg(file) );

        Action* action = new Action(d->actions, name, file);
        action->setText(text);
        action->setDescription(description);
        if(! icon.isNull())
            action->setIcon(KIcon(icon));
        if(! interpreter.isNull())
            action->setInterpreterName(interpreter);
        d->scriptsmenu->addAction(action);
    }

    d->scriptsmenu->setEnabled( ! d->actions->isEmpty() );
}

void GUIClient::writeConfig(KConfig* config)
{
    krossdebug( QString("GUIClient::write hasGroup=%1 isReadOnly=%2 isImmutable=%3 ConfigState=%4").arg(config->hasGroup("scripts")).arg(config->isReadOnly()).arg(config->isImmutable()).arg(config->getConfigState()) );

    config->deleteGroup("scripts"); // remove old entries
    config->setGroup("scripts"); // according to the documentation it's needed to re-set the group after delete.

    QStringList names;
    foreach(KAction* a, d->actions->actions(QString::null)) {
        Action* action = static_cast< Action* >(a);
        const QString name = action->objectName();
        names << name;
        config->writeEntry(QString("%1_text").arg(name).toLatin1(), action->text());
        config->writeEntry(QString("%1_description").arg(name).toLatin1(), action->description());

        //TODO hmmm... kde4's KIcon / Qt4's QIcon does not allow to reproduce the iconname?
        //config->writeEntry(QString("%1_icon").arg(name).toLatin1(), action->icon());

        config->writeEntry(QString("%1_file").arg(name).toLatin1(), action->getFile().path());
        config->writeEntry(QString("%1_interpreter").arg(name).toLatin1(), action->getInterpreterName());
    }

    config->writeEntry("names", names);
    config->sync();
}

#if 0
void GUIClient::loadScriptConfig()
{
    d->actions->clear();
    QString configfile = "/home/kde4/scripts.rc";//KGlobal::dirs()->findResource("appdata","scripts.rc");
    if(configfile.isNull()) {
        krossdebug( QString("No scriptconfigfile found.") );
        return;
    }
    QDomDocument domdoc;
    QFile file(configfile);
    if(! file.open(QIODevice::ReadOnly)) {
        krosswarning( QString("GUIClient::loadScriptConfig(): Failed to read scriptconfigfile: %1").arg(configfile) );
        return;
    }
    bool ok = domdoc.setContent(&file);
    file.close();
    if(! ok) {
        krosswarning( QString("GUIClient::loadScriptConfig(): Failed to parse scriptconfigfile: %1").arg(configfile) );
        return;
    }
    QDomNodeList nodelist = domdoc.elementsByTagName("ScriptAction");
    uint nodelistcount = nodelist.count();
    for(uint i = 0; i < nodelistcount; i++) {
        QDomElement element = nodelist.item(i).toElement();
        new Action(d->actions, element);

        /*
        connect(action.data(), SIGNAL( failed(const QString&, const QString&) ),
                this, SLOT( executionFailed(const QString&, const QString&) ));
        connect(action.data(), SIGNAL( success() ),
                this, SLOT( successfullyExecuted() ));
        connect(action.data(), SIGNAL( activated(const Kross::Action*) ), SIGNAL( executionStarted(const Kross::Action*)));
        */
    }
    //emit collectionChanged(installedcollection);
    #if 0
    ActionCollection* installedcollection = d->collections["installedscripts"];
    if(installedcollection)
        installedcollection->clear();
    QByteArray partname = d->guiclient->instance()->instanceName();
    QStringList files = KGlobal::dirs()->findAllResources("data", partname + "/scripts/*/*.rc");
    //files.sort();
    for(QStringList::iterator it = files.begin(); it != files.end(); ++it)
        loadScriptConfigFile(*it);
    #endif
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
        connect(action.data(), SIGNAL( failed(const QString&, const QString&) ), this, SLOT( executionFailed(const QString&, const QString&) ));
        connect(action.data(), SIGNAL( success() ), this, SLOT( successfullyExecuted() ));
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
    if(! merge && installedcollection) installedcollection->clear();
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

bool GUIClient::executeFile()
{
    QStringList mimetypes;
    QMap<QString, InterpreterInfo*> infos = Manager::self().getInterpreterInfos();
    for(QMap<QString, InterpreterInfo*>::Iterator it = infos.begin(); it != infos.end(); ++it)
        mimetypes.append( it.value()->getMimeTypes().join(" ").trimmed() );

    KFileDialog* filedialog = new KFileDialog(
        KUrl("kfiledialog:///KrossExecuteScript"), // startdir
        mimetypes.join(" "), // filter
        0, // custom widget
        0 // parent
    );
    filedialog->setCaption( i18n("Execute Script File") );
    return filedialog->exec() ? executeFile(filedialog->selectedUrl()) : false;
}

bool GUIClient::executeFile(const KUrl& file)
{
    krossdebug( QString("GUIClient::executeFile() file='%1'").arg(file.path()) );
    return executeAction( Action::Ptr( new Action(file) ) );
}

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

bool GUIClient::installPackage()
{
    KFileDialog* filedialog = new KFileDialog(
        KUrl("kfiledialog:///KrossInstallPackage"), // startdir
        "*.tar.gz *.tgz *.bz2", // filter
        0, // custom widget
        0 // parent
    );
    filedialog->setCaption(i18n("Install Script Package"));
    return filedialog->exec() ? installPackage(filedialog->selectedUrl()) : false;
}

bool GUIClient::installPackage(const KUrl& file)
{
    const QString scriptpackagefile = file.path();
    KTar archive( scriptpackagefile );
    if(! file.isLocalFile() || ! archive.open(QIODevice::ReadOnly)) {
        KMessageBox::sorry(0, i18n("Could not read the package \"%1\".", scriptpackagefile));
        return false;
    }

    const KArchiveDirectory* archivedir = archive.directory();
    const KArchiveEntry* entry = archivedir->entry("install.xml");
    if(! entry || ! entry->isFile()) {
        KMessageBox::sorry(0, i18n("The package \"%1\" does not contain a valid install.xml file.", scriptpackagefile));
        return false;
    }

    QString xml = static_cast< const KArchiveFile* >(entry)->data();
    QDomDocument domdoc;
    if(! domdoc.setContent(xml)) {
        KMessageBox::sorry(0, i18n("Failed to parse the install.xml file at package \"%1\".", scriptpackagefile));
        return false;
    }

    QDomElement element = domdoc.documentElement();
    const QString name = element.attribute("name");
    const QString version = element.attribute("version");
    const QString text = element.attribute("text");
    const QString description = element.attribute("description");
    const QString icon = element.attribute("icon");
    const QString interpreter = element.attribute("interpreter");
    const QString scriptfile = element.attribute("file");

    if(name.isNull() || name.contains(QRegExp("[^a-zA-Z0-9\\_\\-]"))) {
        KMessageBox::sorry(0, i18n("The install.xml file of the package \"%1\" does not define a valid package-name.", scriptpackagefile));
        return false;
    }

    QString destination = KGlobal::dirs()->saveLocation("appdata", "scripts/", true);
    if(destination.isNull()) {
        KMessageBox::sorry(0, i18n("Failed to determinate location where the package \"%1\" should be installed to.", scriptpackagefile));
        return false;
    }

    destination += name; // add the packagename to the name of the destination-directory.
    if( QDir(destination).exists() ) {
        if( KMessageBox::warningContinueCancel(0,
            i18n("A script package with the name \"%1\" already exists. Replace this package?" , name),
            i18n("Replace")) != KMessageBox::Continue )
                return false;
        if(! KIO::NetAccess::del(destination, 0) ) {
            KMessageBox::sorry(0, i18n("Could not uninstall this script package. You may not have sufficient permissions to delete the folder \"%1\".", destination));
            return false;
        }
    }

    krossdebug( QString("Copy script-package to destination directory: %1").arg(destination) );
    archivedir->copyTo(destination, true);

    Action* action = new Action(d->actions, name, scriptfile);
    action->setText(text);
    action->setDescription(description);
    if(! icon.isNull())
        action->setIcon(KIcon(icon));
    if(! interpreter.isNull())
        action->setInterpreterName(interpreter);
    d->scriptsmenu->addAction(action);
    d->scriptsmenu->setEnabled(true);

    writeConfig( instance()->config() );
    return true;
}

bool GUIClient::uninstallPackage(Action* action)
{
    const QString name = action->objectName();

    KUrl url = action->getFile();
    if(! url.isValid()) {
        KMessageBox::sorry(0, i18n("Could not uninstall the script package \"%1\" since the script is not installed.").arg(action->objectName()));
        return false;
    }

    QDir dir = QFileInfo( url.path() ).dir();
    const QString scriptpackagepath = dir.absolutePath();
    krossdebug( QString("Uninstall script-package with destination directory: %1").arg(scriptpackagepath) );

    if(! KIO::NetAccess::del(scriptpackagepath, 0) ) {
        KMessageBox::sorry(0, i18n("Could not uninstall the script package \"%1\". You may not have sufficient permissions to delete the folder \"%1\".").arg(action->objectName()).arg(scriptpackagepath));
        return false;
    }

    d->scriptsmenu->removeAction(action);
    delete action; action = 0; // removes the action from d->actions as well

    writeConfig( instance()->config() );
    return true;
}

void GUIClient::showManager()
{
    GUIManagerDialog* dialog = new GUIManagerDialog(this, d->parent);
    dialog->resize( QSize(360, 320).expandedTo(dialog->minimumSizeHint()) );
    dialog->show();
}

#include "guiclient.moc"
