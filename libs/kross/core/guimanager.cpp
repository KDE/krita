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
#include "guiclient.h"
//#include "action.h"

//#include <QFile>
//#include <QFileInfo>
//#include <QObject>
//#include <QMenu>
//#include <QToolTip>
//#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

#include <kapplication.h>
//#include <kdeversion.h>
#include <kconfig.h>
//#include <kicon.h>
//#include <kfiledialog.h>
//#include <kiconloader.h>
#include <klocale.h>
#include <kicon.h>
#include <kmessagebox.h>
//#include <kpushbutton.h>
//#include <kstandarddirs.h>
#include <kpushbutton.h>
//#include <ktoolbar.h>

#include <knewstuff/provider.h>
#include <knewstuff/engine.h>
#include <knewstuff/downloaddialog.h>
#include <knewstuff/knewstuffsecure.h>

using namespace Kross;

/******************************************************************************
 * GUIManagerModel
 */

namespace Kross {

    /// \internal d-pointer class.
    class GUIManagerModel::Private
    {
        public:
            KActionCollection* collection;
            Private(KActionCollection* collection) : collection(collection) {}
    };

}

GUIManagerModel::GUIManagerModel(KActionCollection* collection, QObject* parent)
    : QAbstractItemModel(parent)
    , d(new Private(collection))
{
}

GUIManagerModel::~GUIManagerModel()
{
    delete d;
}

int GUIManagerModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

int GUIManagerModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->collection->actions(QString::null).count();
}

QModelIndex GUIManagerModel::index(int row, int column, const QModelIndex& parent) const
{
    if(parent.isValid())
        return QModelIndex();
    return createIndex(row, column, d->collection->actions().value(row));
}

QModelIndex GUIManagerModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

Qt::ItemFlags GUIManagerModel::flags(const QModelIndex &index) const
{
    if(! index.isValid())
        return Qt::ItemIsEnabled;
    if(index.column() == 0)
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    return QAbstractItemModel::flags(index); // | Qt::ItemIsEditable;
}

QVariant GUIManagerModel::data(const QModelIndex& index, int role) const
{
    if(! index.isValid())
        return QVariant();
    Action* action = static_cast< Action* >( index.internalPointer() );
    switch( role ) {
        case Qt::DecorationRole:
            return action->icon();
        case Qt::DisplayRole:
            return action->text().replace("&","");
        case Qt::ToolTipRole: // fall through
        case Qt::WhatsThisRole:
            return action->description();
        case Qt::CheckStateRole:
            return action->isVisible();
        default:
            return QVariant();
    }
}

bool GUIManagerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(! index.isValid())
        return false;
    Action* action = static_cast< Action* >( index.internalPointer() );
    switch( role ) {
        case Qt::EditRole: {
            action->setText( value.toString() );
        } break;
        case Qt::CheckStateRole: {
            action->setVisible( ! action->isVisible() );
        } break;
        default:
            return false;
    }
    emit dataChanged(index, index);
    return true;
}

/******************************************************************************
 * GUIManagerView
 */

namespace Kross {

    /// \internal class that inherits \a KNewStuffSecure to implement the GHNS-functionality.
    class GUIManagerNewStuff : public KNewStuffSecure
    {
        public:
            GUIManagerNewStuff(GUIClient* guiclient, const QString& type, QWidget *parentWidget = 0)
                : KNewStuffSecure(type, parentWidget)
                , m_guiclient(guiclient) {}
            virtual ~GUIManagerNewStuff() {}
        private:
            GUIClient* m_guiclient;
            virtual void installResource() { m_guiclient->installPackage( m_tarName ); }
    };

    /// \internal d-pointer class.
    class GUIManagerView::Private
    {
        public:
            GUIClient* guiclient;
            GUIManagerModel* model;
            QItemSelectionModel* selectionmodel;
            KActionCollection* collection;
            GUIManagerNewStuff* newstuff;

            Private() : newstuff(0) {}
    };

}

GUIManagerView::GUIManagerView(GUIClient* guiclient, QWidget* parent)
    : QTreeView(parent)
    , d(new Private())
{
    setAlternatingRowColors(true);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setItemsExpandable(false);
    header()->hide();

    d->model = new GUIManagerModel(guiclient->scriptsActionCollection(), this);
    setModel(d->model);

    d->selectionmodel = new QItemSelectionModel(d->model, this);
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
    d->guiclient->installPackage();
}

void GUIManagerView::slotUninstall()
{
    foreach(QModelIndex index, d->selectionmodel->selectedIndexes())
        if(index.isValid())
            if(! d->guiclient->uninstallPackage( static_cast< Action* >(index.internalPointer()) ))
                break;
}

void GUIManagerView::slotNewScripts()
{
    const QString appname = KApplication::kApplication()->objectName();
    const QString type = QString("%1/script").arg(appname);
    krossdebug( QString("GUIManagerView::slotNewScripts %1").arg(type) );
    if(! d->newstuff) {
        d->newstuff = new GUIManagerNewStuff(d->guiclient, type);
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
 * GUIManagerDialog
 */

namespace Kross {

    /// \internal d-pointer class.
    class GUIManagerDialog::Private
    {
        public:
            GUIClient* guiclient;
            GUIManagerView* view;
            Private(GUIClient* gc) : guiclient(gc) {}
    };

}

GUIManagerDialog::GUIManagerDialog(GUIClient* guiclient, QWidget* parent)
    : KDialog(parent)
    , d(new Private(guiclient))
{
    setCaption( i18n("Scripts Manager") );
    setButtons( KDialog::Close );

    QWidget* mainwidget = new QWidget(this);
    QHBoxLayout* mainlayout = new QHBoxLayout();
    mainlayout->setMargin(0);
    mainwidget->setLayout(mainlayout);
    setMainWidget(mainwidget);

    d->view = new GUIManagerView(guiclient, mainwidget);
    mainlayout->addWidget(d->view);

    QWidget* btnwidget = new QWidget(mainwidget);
    QVBoxLayout* btnlayout = new QVBoxLayout();
    btnlayout->setMargin(0);
    btnwidget->setLayout(btnlayout);
    mainlayout->addWidget(btnwidget);

    foreach(KAction* action, d->view->actionCollection()->actions(QString::null)) {
        KPushButton* btn = new KPushButton(action->icon(), action->text(), btnwidget);
        btn->setToolTip( action->toolTip() );
        btnlayout->addWidget(btn);
        connect(btn, SIGNAL(clicked()), action, SLOT(trigger()));
        //connect(action, SLOT(setEnabled(bool)), btn, SLOT(setEnabled(bool)));
    }

    btnlayout->addStretch(1);
    resize( QSize(460, 340).expandedTo(minimumSizeHint()) );

    guiclient->readAllConfigs();
    connect(this, SIGNAL(closeClicked()), this, SLOT(saveChanges()));
}

GUIManagerDialog::~GUIManagerDialog()
{
    delete d;
}

void GUIManagerDialog::saveChanges()
{
    d->guiclient->writeConfig();
}

#include "guimanager.moc"
