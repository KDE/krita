/***************************************************************************
 * guimanager.h
 * This file is part of the KDE project
 * copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 * copyright (C) 2006 Sebastian Sauer <mail@dipe.org>
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

#include "guimanager.h"
#include "manager.h"
#include "action.h"
#include "guiclient.h"
#include "model.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileInfo>
#include <QDir>

#include <kapplication.h>
//#include <kdeversion.h>
#include <kconfig.h>
#include <klocale.h>
#include <kicon.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kfiledialog.h>
#include <kmenu.h>

#include <ktar.h>
#include <kio/netaccess.h>
#include <knewstuff/provider.h>
#include <knewstuff/engine.h>
#include <knewstuff/downloaddialog.h>
#include <knewstuff/knewstuffsecure.h>

using namespace Kross;

/******************************************************************************
 * GUIManagerView
 */

namespace Kross {

    /// \internal class that inherits \a KNewStuffSecure to implement the GHNS-functionality.
    class GUIManagerNewStuff : public KNewStuffSecure
    {
        public:
            GUIManagerNewStuff(GUIManagerModule* module, const QString& type, QWidget *parentWidget = 0)
                : KNewStuffSecure(type, parentWidget)
                , m_module(module) {}
            virtual ~GUIManagerNewStuff() {}
        private:
            GUIManagerModule* m_module;
            virtual void installResource() { m_module->installPackage( m_tarName ); }
    };

    /// \internal d-pointer class.
    class GUIManagerView::Private
    {
        public:
            GUIManagerModule* module;
            QItemSelectionModel* selectionmodel;
            KActionCollection* collection;
            GUIManagerNewStuff* newstuff;

            Private(GUIManagerModule* m) : module(m), newstuff(0) {}
    };

}

GUIManagerView::GUIManagerView(GUIManagerModule* module, QWidget* parent)
    : QTreeView(parent)
    , d(new Private(module))
{
    setAlternatingRowColors(true);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setItemsExpandable(false);
    header()->hide();

    ActionCollectionModel* model = new ActionCollectionModel(this, Kross::Manager::self().actionCollection());
    setModel(model);

    d->selectionmodel = new QItemSelectionModel(model, this);
    connect(d->selectionmodel, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(slotSelectionChanged()));
    setSelectionModel(d->selectionmodel);

    d->collection = new KActionCollection(this);

    KAction* runaction = new KAction(KIcon("player_play"), i18n("Run"), d->collection, "runscript");
    runaction->setToolTip( i18n("Execute the selected script.") );
    connect(runaction, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotRun()) );

    KAction* stopaction = new KAction(KIcon("player_stop"), i18n("Stop"), d->collection, "stopscript");
    stopaction->setToolTip( i18n("Stop execution of the selected script.") );
    connect(stopaction, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotStop()) );

    KAction* installaction = new KAction(KIcon("fileimport"), i18n("Install"), d->collection, "installscript");
    installaction->setToolTip( i18n("Install a script-package.") );
    connect(installaction, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotInstall()) );

    KAction* uninstallaction = new KAction(KIcon("fileclose"), i18n("Uninstall"), d->collection, "uninstallscript");
    uninstallaction->setToolTip( i18n("Uninstall the selected script-package.") );
    connect(uninstallaction, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotUninstall()) );

    KAction* newstuffaction = new KAction(KIcon("knewstuff"), i18n("Get New Scripts"), d->collection, "newscript");
    newstuffaction->setToolTip( i18n("Get new scripts from the internet.") );
    connect(newstuffaction, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotNewScripts()) );

    //new KAction(KIcon(""), i18n("About"), d->collection);
    //new KAction(KIcon(""), i18n("Configure"), d->collection);
}

GUIManagerView::~GUIManagerView()
{
    delete d;
}

KActionCollection* GUIManagerView::actionCollection() const
{
    return d->collection;
}

void GUIManagerView::slotSelectionChanged()
{
    bool isselected = d->selectionmodel->hasSelection();
    d->collection->action("runscript")->setEnabled(isselected);
    d->collection->action("stopscript")->setEnabled(isselected);
    d->collection->action("uninstallscript")->setEnabled(isselected);
}

void GUIManagerView::slotRun()
{
    foreach(QModelIndex index, d->selectionmodel->selectedIndexes())
        if(index.isValid())
            static_cast< Action* >(index.internalPointer())->trigger();
}

void GUIManagerView::slotStop()
{
    foreach(QModelIndex index, d->selectionmodel->selectedIndexes())
        if(index.isValid())
            static_cast< Action* >(index.internalPointer())->finalize();
}

void GUIManagerView::slotInstall()
{
    d->module->showInstallPackageDialog();
}

void GUIManagerView::slotUninstall()
{
    foreach(QModelIndex index, d->selectionmodel->selectedIndexes())
        if(index.isValid())
            if(! d->module->uninstallPackage( static_cast< Action* >(index.internalPointer()) ))
                break;
}

void GUIManagerView::slotNewScripts()
{
    const QString appname = KApplication::kApplication()->objectName();
    const QString type = QString("%1/script").arg(appname);
    krossdebug( QString("GUIManagerView::slotNewScripts %1").arg(type) );
    if(! d->newstuff) {
        d->newstuff = new GUIManagerNewStuff(d->module, type);
        connect(d->newstuff, SIGNAL(installFinished()), this, SLOT(slotNewScriptsInstallFinished()));
    }
    KNS::Engine *engine = new KNS::Engine(d->newstuff, type, this);
    KNS::DownloadDialog *d = new KNS::DownloadDialog(engine, this);
    d->setCategory(type);
    KNS::ProviderLoader *p = new KNS::ProviderLoader(this);
    QObject::connect(p, SIGNAL(providersLoaded(Provider::List*)), d, SLOT(slotProviders(Provider::List*)));
    p->load(type, QString("http://download.kde.org/khotnewstuff/%1scripts-providers.xml").arg(appname));
    d->exec();
}

void GUIManagerView::slotNewScriptsInstallFinished()
{
    // Delete KNewStuff's configuration entries. These entries reflect what has
    // already been installed. As we cannot yet keep them in sync after uninstalling
    // scripts, we deactivate the check marks entirely.
    KGlobal::config()->deleteGroup("KNewStuffStatus");
}

/******************************************************************************
 * GUIManagerModule
 */

namespace Kross {

    /// \internal d-pointer class.
    class GUIManagerModule::Private
    {
        public:
    };

}

GUIManagerModule::GUIManagerModule()
    : QObject()
    , d(new Private())
{
}

GUIManagerModule::~GUIManagerModule()
{
    delete d;
}

bool GUIManagerModule::installPackage(const QString& scriptpackagefile)
{
    KTar archive( scriptpackagefile );
    if(! archive.open(QIODevice::ReadOnly)) {
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

        Action* action = new Action(Manager::self().actionCollection(), element, packagepath);
        connect(action, SIGNAL( failed(const QString&, const QString&) ), this, SLOT( executionFailed(const QString&, const QString&) ));
        connect(action, SIGNAL( success() ), this, SLOT( executionSuccessful() ));
        connect(action, SIGNAL( activated(Kross::Action*) ), SIGNAL( executionStarted(Kross::Action*)));
        Manager::self().actionMenu()->addAction(action);
    }

    Manager::self().writeConfig();
    return true;
}

bool GUIManagerModule::uninstallPackage(Action* action)
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

    Manager::self().actionMenu()->removeAction(action);
    delete action; action = 0; // removes the action from d->actions as well

    Manager::self().writeConfig();
    return true;
}

bool GUIManagerModule::showInstallPackageDialog()
{
    KFileDialog* filedialog = new KFileDialog(
        KUrl("kfiledialog:///KrossInstallPackage"), // startdir
        "*.tar.gz *.tgz *.bz2", // filter
        0, // custom widget
        0 // parent
    );
    filedialog->setCaption(i18n("Install Script Package"));
    return filedialog->exec() ? installPackage(filedialog->selectedUrl().path()) : false;
}

void GUIManagerModule::showManagerDialog()
{
    KDialog* dialog = new KDialog();
    dialog->setCaption( i18n("Script Manager") );
    dialog->setButtons( KDialog::Close );

    QWidget* mainwidget = dialog->mainWidget();
    QHBoxLayout* mainlayout = new QHBoxLayout();
    mainlayout->setMargin(0);
    mainwidget->setLayout(mainlayout);

    GUIManagerView* view = new GUIManagerView(this, mainwidget);
    mainlayout->addWidget(view);

    QWidget* btnwidget = new QWidget(mainwidget);
    QVBoxLayout* btnlayout = new QVBoxLayout();
    btnlayout->setMargin(0);
    btnwidget->setLayout(btnlayout);
    mainlayout->addWidget(btnwidget);

    foreach(KAction* action, view->actionCollection()->actions(QString::null)) {
        KPushButton* btn = new KPushButton(action->icon(), action->text(), btnwidget);
        btn->setToolTip( action->toolTip() );
        btnlayout->addWidget(btn);
        connect(btn, SIGNAL(clicked()), action, SLOT(trigger()));
        //connect(action, SLOT(setEnabled(bool)), btn, SLOT(setEnabled(bool)));
    }

    btnlayout->addStretch(1);
    dialog->resize( QSize(460, 340).expandedTo( dialog->minimumSizeHint() ) );

    dialog->exec();
    Manager::self().writeConfig();
    dialog->delayedDestruct();
}

#include "guimanager.moc"
