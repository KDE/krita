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
    readConfig();

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

void GUIClient::readConfig()
{
    KConfig* config = instance()->config();
    krossdebug( QString("GUIClient::readConfig hasGroup=%1 isReadOnly=%2 isImmutable=%3 ConfigState=%4").arg(config->hasGroup("scripts")).arg(config->isReadOnly()).arg(config->isImmutable()).arg(config->getConfigState()) );

    if(! config->hasGroup("scripts") && ! config->isReadOnly()) {
        // if there is no scripts-section in the configfile yet, we assume that the user uses a fresh
        // configfile. So, we just read all available script-packages. Since readAllConfigs calls our
        // method, we need to be sure, that we don't end in a loop. So, let's add our expected
        // "scripts" group to the configfile, sync, and re-check if it was really written before we
        // start to create an initial "scripts" configurationgroup which contains all scripts we found.
        config->setGroup("scripts");
        config->writeEntry("names", QStringList());
        config->sync();
        if(config->hasGroup("scripts")) {
            readAllConfigs();
            foreach(KAction* a, d->actions->actions(QString::null))
                a->setVisible(true);
        }
        writeConfig();
        return;
    }

    d->actions->clear();
    d->scriptsmenu->menu()->clear();

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

        if(text.isEmpty())
            text = file;

        if(description.isEmpty())
            description = text.isEmpty() ? name : text;

        if(icon.isEmpty())
            icon = KMimeType::iconNameForUrl( KUrl(file) );

        krossdebug( QString("GUIClient::readConfig Add scriptaction name='%1' file='%2'").arg(name).arg(file) );

        Action* action = new Action(d->actions, name, file);
        action->setText(text);
        action->setDescription(description);
        if(! icon.isNull())
            action->setIcon(KIcon(icon));
        if(! interpreter.isNull())
            action->setInterpreter(interpreter);
        connect(action, SIGNAL( failed(const QString&, const QString&) ), this, SLOT( executionFailed(const QString&, const QString&) ));
        connect(action, SIGNAL( success() ), this, SLOT( executionSuccessfull() ));
        connect(action, SIGNAL( activated(Kross::Action*) ), SIGNAL( executionStarted(Kross::Action*)));

        d->scriptsmenu->addAction(action);
    }
}

void GUIClient::writeConfig()
{
    KConfig* config = instance()->config();
    krossdebug( QString("GUIClient::write hasGroup=%1 isReadOnly=%2 isImmutable=%3 ConfigState=%4").arg(config->hasGroup("scripts")).arg(config->isReadOnly()).arg(config->isImmutable()).arg(config->getConfigState()) );

    config->deleteGroup("scripts"); // remove old entries
    config->setGroup("scripts"); // according to the documentation it's needed to re-set the group after delete.

    QStringList names;
    foreach(KAction* a, d->actions->actions(QString::null)) {
        Action* action = static_cast< Action* >(a);
        if(! action->isVisible()) continue; // don't remember invisible (as in not installed) scripts
        const QString name = action->objectName();
        names << name;
        config->writeEntry(QString("%1_text").arg(name).toLatin1(), action->text());
        config->writeEntry(QString("%1_description").arg(name).toLatin1(), action->description());

        //TODO hmmm... kde4's KIcon / Qt4's QIcon does not allow to reproduce the iconname?
        //config->writeEntry(QString("%1_icon").arg(name).toLatin1(), action->icon());

        config->writeEntry(QString("%1_file").arg(name).toLatin1(), action->getFile().path());
        config->writeEntry(QString("%1_interpreter").arg(name).toLatin1(), action->interpreter());
    }

    config->writeEntry("names", names);
    config->sync();
}

void GUIClient::readAllConfigs()
{
    readConfig();

    QByteArray partname = d->guiclient->instance()->instanceName();
    QStringList files = KGlobal::dirs()->findAllResources("data", partname + "/scripts/*/install.rc");
    foreach(QString file, files) {
        const QDir packagepath = QFileInfo(file).dir();

        krossdebug( QString("GUIClient::readAllConfigs trying to read \"%1\"").arg(file) );
        QFile f(file);
        if(! f.open(QIODevice::ReadOnly)) {
            krossdebug( QString("GUIClient::readAllConfigs reading \"%1\" failed. Skipping package.").arg(file) );
            continue;
        }

        QDomDocument domdoc;
        bool ok = domdoc.setContent(&f);
        f.close();
        if(! ok) {
            krossdebug( QString("GUIClient::readAllConfigs parsing \"%1\" failed. Skipping package.").arg(file) );
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

            krossdebug( QString("GUIClient::readAllConfigs adding script-package \"%1\" from file \"%2\".").arg(name).arg(file) );
            Action* action = new Action(d->actions, element, packagepath);
            action->setVisible(false); // the package is not enabled, so don't display it.
            connect(action, SIGNAL( failed(const QString&, const QString&) ), this, SLOT( executionFailed(const QString&, const QString&) ));
            connect(action, SIGNAL( success() ), this, SLOT( executionSuccessfull() ));
            connect(action, SIGNAL( activated(Kross::Action*) ), SIGNAL( executionStarted(Kross::Action*)));
            d->scriptsmenu->addAction(action);
        }
    }
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

void GUIClient::executionSuccessfull()
{
    Action* action = dynamic_cast< Action* >( QObject::sender() );
    emit executionFinished(action);
}

void GUIClient::executionFailed(const QString& errormessage, const QString& tracedetails)
{
    Action* action = dynamic_cast< Action* >( QObject::sender() );
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
    connect(action.data(), SIGNAL( activated(Kross::Action*) ), SIGNAL( executionStarted(Kross::Action*)));

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
    const KArchiveEntry* entry = archivedir->entry("install.rc");
    if(! entry || ! entry->isFile()) {
        KMessageBox::sorry(0, i18n("The package \"%1\" does not contain a valid install.rc file.", scriptpackagefile));
        return false;
    }

    QString xml = static_cast< const KArchiveFile* >(entry)->data();
    QDomDocument domdoc;
    if(! domdoc.setContent(xml)) {
        KMessageBox::sorry(0, i18n("Failed to parse the install.rc file at package \"%1\".", scriptpackagefile));
        return false;
    }

    QString destination = KGlobal::dirs()->saveLocation("appdata", "scripts/", true);
    if(destination.isNull()) {
        KMessageBox::sorry(0, i18n("Failed to determinate location where the package \"%1\" should be installed to.", scriptpackagefile));
        return false;
    }

    QString packagename = QFileInfo(scriptpackagefile).baseName();
    destination += packagename; // add the packagename to the name of the destination-directory.

    QDir packagepath(destination);
    if( packagepath.exists() ) {
        if( KMessageBox::warningContinueCancel(0,
            i18n("A script package with the name \"%1\" already exists. Replace this package?", packagename),
            i18n("Replace")) != KMessageBox::Continue )
                return false;
        if(! KIO::NetAccess::del(destination, 0) ) {
            KMessageBox::sorry(0, i18n("Could not uninstall this script package. You may not have sufficient permissions to delete the folder \"%1\".", destination));
            return false;
        }
    }

    krossdebug( QString("Copy script-package to destination directory: %1").arg(destination) );
    archivedir->copyTo(destination, true);

    QDomNodeList nodelist = domdoc.elementsByTagName("ScriptAction");
    int nodelistcount = nodelist.count();
    for(int i = 0; i < nodelistcount; ++i) {
        QDomElement element = nodelist.item(i).toElement();
        Action* action = new Action(d->actions, element, packagepath);
        connect(action, SIGNAL( failed(const QString&, const QString&) ), this, SLOT( executionFailed(const QString&, const QString&) ));
        connect(action, SIGNAL( success() ), this, SLOT( executionSuccessfull() ));
        connect(action, SIGNAL( activated(Kross::Action*) ), SIGNAL( executionStarted(Kross::Action*)));
        d->scriptsmenu->addAction(action);
    }

    writeConfig();
    return true;
}

bool GUIClient::uninstallPackage(Action* action)
{
    const QString name = action->objectName();

    KUrl url = action->getFile();
    if(! url.isValid() || ! url.isLocalFile()) {
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

    writeConfig();
    return true;
}

void GUIClient::showManager()
{
    GUIManagerDialog* dialog = new GUIManagerDialog(this, d->parent);
    dialog->show();
}

#include "guiclient.moc"
