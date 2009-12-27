/***************************************************************************
 * KoScriptManager.cpp
 * This file is part of the KDE project
 * copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "KoScriptManager.h"
#include "KoScriptManagerAdd.h"

#include <kross/core/manager.h>
#include <kross/core/actioncollection.h>
#include <kross/ui/model.h>
#include <kross/ui/view.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <QtGui/QBoxLayout>

/******************************************************************************
 * KoScriptManagerCollection
 */

/*
/// \internal class that inherits \a KNewStuffSecure to implement the GHNS-functionality.
class KoScriptManagerNewStuff : public KNewStuffSecure
{
    public:
        KoScriptManagerNewStuff(KoScriptManagerCollection *collection, const QString& type, QWidget *parentWidget = 0)
            : KNewStuffSecure(type, parentWidget)
            , m_collection(collection) {}
        virtual ~KoScriptManagerNewStuff() {}
    private:
        KoScriptManagerCollection *m_collection;
        virtual void installResource() { m_collection->module()->installPackage(m_tarName); }
};
*/

class KoScriptManagerView : public Kross::ActionCollectionView
{
public:
    KoScriptManagerView(KoScriptManagerCollection *collection) : Kross::ActionCollectionView(collection)
    {
        setDragEnabled(true);
        setAcceptDrops(true);

        Kross::ActionCollectionModel::Mode modelmode = Kross::ActionCollectionModel::Mode(Kross::ActionCollectionModel::Icons | Kross::ActionCollectionModel::ToolTips | Kross::ActionCollectionModel::UserCheckable);
        Kross::ActionCollectionModel *model = new Kross::ActionCollectionModel(this, Kross::Manager::self().actionCollection(), modelmode);
        setModel(model);
        //selectionModel();
    }

    virtual ~KoScriptManagerView() {}

    Kross::ActionCollectionModel *model() const
    {
            return static_cast<Kross::ActionCollectionModel*>(Kross::ActionCollectionView::model());
    }

#ifndef DISABLE_ADD_REMOVE
    virtual void slotAdd()
    {
        Kross::ActionCollection *collection = model()->rootCollection();
        if (itemSelection().count() == 1) {
            collection = Kross::ActionCollectionModel::collection(itemSelection().indexes().first());
        }
        KoScriptManagerAddWizard wizard(this, collection);
        int result = wizard.exec();
        Q_UNUSED(result);
    }
    virtual void slotRemove()
    {
        if (itemSelection().isEmpty()) {
            return;
        }
        // get the actions/collections, indexes will change when things are removed
        QList<Kross::Action*> actions;
        QList<Kross::ActionCollection*> collections;
        foreach (const QModelIndex &idx, itemSelection().indexes()) {
            Kross::Action *a =  model()->action(idx);
            if (a) {
                actions << a;
            } else {
                Kross::ActionCollection *c = model()->collection(idx);
                if (c) {
                    collections << c;
                }
            }
        }
        foreach (Kross::Action *action, actions) {
            QModelIndex idx = model()->indexForAction(action);
            if (idx.isValid()) {
                //kDebug(32010)<<"action:"<<action->name();
                delete action;
            }
        }
        foreach (Kross::ActionCollection *collection, collections) {
            QModelIndex idx = model()->indexForCollection(collection);
            if (idx.isValid()) {
                //kDebug(32010)<<"collection:"<<collection->name();
                delete collection;
            }
        }
    }
#endif
};

KoScriptManagerCollection::KoScriptManagerCollection(QWidget *parent)
    : QWidget(parent),
    m_modified(false)
{
    QHBoxLayout *mainlayout = new QHBoxLayout();
    mainlayout->setMargin(0);
    setLayout(mainlayout);

    m_view = new KoScriptManagerView(this);
    mainlayout->addWidget(m_view);

    QWidget *btnwidget = new QWidget(this);
    QVBoxLayout *btnlayout = new QVBoxLayout();
    btnlayout->setMargin(0);
    btnwidget->setLayout(btnlayout);
    mainlayout->addWidget(btnwidget);

    //KActionCollection *collection = m_view->actionCollection();

    m_view->createButton(btnwidget, "run");
    m_view->createButton(btnwidget, "stop");

    QFrame *hr1 = new QFrame(btnwidget);
    hr1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    btnlayout->addWidget(hr1, 0);

    m_view->createButton(btnwidget, "edit");
    m_view->createButton(btnwidget, "add");
    m_view->createButton(btnwidget, "remove");

    btnlayout->addStretch(1);
    m_view->expandAll();
}

KoScriptManagerCollection::~KoScriptManagerCollection()
{
}

/*
bool KoScriptManagerCollection::isModified() const
{
    return d->modified;
}
*/

#if 0
bool KoScriptManagerCollection::slotInstall() {
    KFileDialog* filedialog = new KFileDialog(
        KUrl("kfiledialog:///KrossInstallPackage"), // startdir
        "*.tar.gz *.tgz *.bz2", // filter
        0, // custom widget
        0 // parent
    );
    filedialog->setCaption(i18n("Install Script Package"));
    return filedialog->exec() ? module()->installPackage(filedialog->selectedUrl().path()) : false;
}
void KoScriptManagerView::slotUninstall() {
    foreach(QModelIndex index, d->selectionmodel->selectedIndexes())
        if(index.isValid())
            if(! uninstallPackage( static_cast< Action* >(index.internalPointer()) ))
                break;
}
void KoScriptManagerView::slotNewScripts() {
    const QString appname = KApplication::kApplication()->objectName();
    const QString type = QString("%1/script").arg(appname);
    krossdebug( QString("ScriptManagerView::slotNewScripts %1").arg(type) );
    if(! d->newstuff) {
        d->newstuff = new KoScriptManagerNewStuff(this, type);
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
void KoScriptManagerView::slotNewScriptsInstallFinished() {
    // Delete KNewStuff's configuration entries. These entries reflect what has
    // already been installed. As we cannot yet keep them in sync after uninstalling
    // scripts, we deactivate the check marks entirely.
    KGlobal::config()->deleteGroup("KNewStuffStatus");
}
#endif

#if 0
bool KoScriptManagerModule::installPackage(const QString& scriptpackagefile)
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
        const QString name = element.attribute("name");
        Action* action = new Action(Manager::self().actionCollection(), name, packagepath);
        action->fromDomElement(element);
        //connect(action, SIGNAL( failed(const QString&, const QString&) ), this, SLOT( executionFailed(const QString&, const QString&) ));
        //connect(action, SIGNAL( success() ), this, SLOT( executionSuccessful() ));
        //connect(action, SIGNAL( activated(Kross::Action*) ), SIGNAL( executionStarted(Kross::Action*)));
    }

    //d->modified = true;
    return true;
}

bool KoScriptManagerModule::uninstallPackage(Action* action)
{
    const QString name = action->objectName();
    QString file = action->file();
    QFileInfo fi(file);
    if(file.isNull() || ! fi.exists()) {
        KMessageBox::sorry(0, i18n("Could not uninstall the script package \"%1\" since the script is not installed.",action->objectName()));
        return false;
    }
    const QString scriptpackagepath = fi.absolutePath();
    krossdebug( QString("Uninstall script-package with destination directory: %1").arg(scriptpackagepath) );
    if(! KIO::NetAccess::del(scriptpackagepath, 0) ) {
        KMessageBox::sorry(0, i18n("Could not uninstall the script package \"%1\". You may not have sufficient permissions to delete the folder \"%1\".",action->objectName()).arg(scriptpackagepath));
        return false;
    }
    delete action; action = 0; // removes the action from d->actions as well
    d->modified = true;
    return true;
}
#endif

KoScriptManagerDialog::KoScriptManagerDialog()
    : KDialog()
{
    setCaption(i18n("Script Manager"));
    setButtons(KDialog::Ok | KDialog::Cancel);
    setButtonText(KDialog::Ok, i18n("Save"));
    setButtonIcon(KDialog::Ok, KIcon("document-save"));
    m_collection = new KoScriptManagerCollection(mainWidget());
    setMainWidget(m_collection);
    resize(QSize(520, 380).expandedTo(minimumSizeHint()));
    connect(this, SIGNAL(accepted()), this, SLOT(slotAccepted()));
}

KoScriptManagerDialog::~KoScriptManagerDialog()
{
}

void KoScriptManagerDialog::slotAccepted()
{
    const QString dir = KGlobal::dirs()->saveLocation("appdata", "scripts/");
    if (! dir.isEmpty()) {
        const QString file = QFileInfo(dir, "scripts.rc").absoluteFilePath();
        QFile f(file);
        if (f.open(QIODevice::WriteOnly))
            if (Kross::Manager::self().actionCollection()->writeXml(&f))
                kDebug(32010)<<"Successfully saved file:"<<file;
        f.close();
    }
}

#include <KoScriptManager.moc>
