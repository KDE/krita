/***************************************************************************
 * guimanager.h
 * This file is part of the KDE project
 * copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 * copyright (C) 2006 by Sebastian Sauer (mail@dipe.org)
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
//#include <kicon.h>
//#include <kapplication.h>
//#include <kdeversion.h>
//#include <kfiledialog.h>
//#include <kiconloader.h>

#include <klocale.h>
#include <kicon.h>
#include <kmessagebox.h>
//#include <kpushbutton.h>
//#include <kstandarddirs.h>
#include <kpushbutton.h>

#include <ktoolbar.h>

//#include <knewstuff/provider.h>
//#include <knewstuff/engine.h>
//#include <knewstuff/downloaddialog.h>
//#include <knewstuff/knewstuffsecure.h>

using namespace Kross;

/******************************************************************************
 * GUIManagerModel
 */

namespace Kross {

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
    /*
    // Delete KNewStuff's configuration entries. These entries reflect what has
    // already been installed. As we cannot yet keep them in sync after uninstalling
    // scripts, we deactivate the check marks entirely.
    KGlobal::config()->deleteGroup("KNewStuffStatus");
    */
    delete d;
}

int GUIManagerModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 2;
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
        default:
            break;
    }
    return QVariant();
}

QModelIndex GUIManagerModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column, d->collection->actions().value(row));
}

QModelIndex GUIManagerModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int GUIManagerModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->collection->actions(QString::null).count();
}

/******************************************************************************
 * GUIManagerView
 */

namespace Kross {

    class GUIManagerView::Private
    {
        public:
            GUIClient* guiclient;
            GUIManagerModel* model;
            QItemSelectionModel* selectionmodel;
            KActionCollection* collection;
    };

}

GUIManagerView::GUIManagerView(GUIClient* guiclient, QWidget* parent)
    : QListView(parent)
    , d(new Private())
{
    d->model = new GUIManagerModel(guiclient->scriptsActionCollection(), this);
    setModel(d->model);

    d->selectionmodel = new QItemSelectionModel(d->model, this);
    connect(d->selectionmodel, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(slotSelectionChanged()));
    setSelectionModel(d->selectionmodel);

    d->collection = new KActionCollection(this);
    connect(new KAction(KIcon("player_play"), i18n("Run"), d->collection, "runscript"),
            SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotRun()) );
    connect(new KAction(KIcon("player_stop"), i18n("Stop"), d->collection, "stopscript"),
            SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotStop()) );

    connect(new KAction(KIcon("fileimport"), i18n("Install"), d->collection, "installscript"),
            SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotInstall()) );
    connect(new KAction(KIcon("fileclose"), i18n("Uninstall"), d->collection, "uninstallscript"),
            SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotUninstall()) );
    connect(new KAction(KIcon("knewstuff"), i18n("Get New Scripts"), d->collection, "newscript"),
            SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotNewScripts()) );

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
    KMessageBox::information(0, "unimplemented yet");
    /*
    const QString appname = KApplication::kApplication()->objectName();
    const QString type = QString("%1/script").arg(appname);
    if(! d->newstuff) {
        d->newstuff = new ScriptNewStuff(d->m_scripguiclient, type);
        connect(d->newstuff, SIGNAL(installFinished()), this, SLOT(slotResourceInstalled()));
    }
    KNS::Engine *engine = new KNS::Engine(d->newstuff, type, this);
    KNS::DownloadDialog *d = new KNS::DownloadDialog( engine, this );
    d->setCategory(type);
    KNS::ProviderLoader *p = new KNS::ProviderLoader(this);
    QObject::connect(p, SIGNAL(providersLoaded(Provider::List*)), d, SLOT(slotProviders(Provider::List*)));
    p->load(type, QString("http://download.kde.org/khotnewstuff/%1scripts-providers.xml").arg(appname));
    d->exec();
    */
}

/******************************************************************************
 * GUIManagerDialog
 */

namespace Kross {

    class GUIManagerDialog::Private
    {
        public:
            GUIManagerView* view;
    };

}

GUIManagerDialog::GUIManagerDialog(GUIClient* guiclient, QWidget* parent)
    : KDialog(parent)
    , d(new Private())
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
        btnlayout->addWidget(btn);
        connect(btn, SIGNAL(clicked()), action, SLOT(trigger()));
        //connect(action, SLOT(setEnabled(bool)), btn, SLOT(setEnabled(bool)));
    }

    btnlayout->addStretch(1);
}

GUIManagerDialog::~GUIManagerDialog()
{
    delete d;
}

#include "guimanager.moc"
