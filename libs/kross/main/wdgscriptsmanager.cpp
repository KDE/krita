/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "wdgscriptsmanager.h"

#include <QFile>
#include <QFileInfo>
#include <q3header.h>
#include <QObject>
#include <QMenu>
//#include <QToolTip>
#include <QPixmap>
#include <Q3ValueList>
#include <QVBoxLayout>

#include <kapplication.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <ktoolbar.h>
#include <kactioncollection.h>

#include <knewstuff/provider.h>
#include <knewstuff/engine.h>
#include <knewstuff/downloaddialog.h>
#include <knewstuff/knewstuffsecure.h>

#include "scriptguiclient.h"
#include "scriptaction.h"

namespace Kross { namespace Api {

class ScriptNewStuff : public KNewStuffSecure
{
    public:
        ScriptNewStuff(ScriptGUIClient* scripguiclient, const QString& type, QWidget *parentWidget = 0)
            : KNewStuffSecure(type, parentWidget)
            , m_scripguiclient(scripguiclient) {}
        virtual ~ScriptNewStuff() {}
    private:
        ScriptGUIClient* m_scripguiclient;
        virtual void installResource() { m_scripguiclient->installScriptPackage( m_tarName ); }
};

class ListItem : public Q3ListViewItem
{
    private:
        ScriptActionCollection* m_collection;
        ScriptAction::Ptr m_action;
    public:
        ListItem(Q3ListView* parentview, ScriptActionCollection* collection)
            : Q3ListViewItem(parentview), m_collection(collection), m_action(0) {}

        ListItem(ListItem* parentitem, Q3ListViewItem* afteritem, ScriptAction::Ptr action)
            : Q3ListViewItem(parentitem, afteritem), m_collection( parentitem->collection() ), m_action(action) {}

        ScriptAction::Ptr action() const { return m_action; }
        ScriptActionCollection* collection() const { return m_collection; }
        //ScriptActionMenu* actionMenu() const { return m_menu; }
};

/*
class ToolTip : public QToolTip
{
    public:
        ToolTip(K3ListView* parent)
            : QToolTip(parent->viewport()), m_parent(parent) {}
        virtual ~ToolTip () { remove(m_parent->viewport()); }
    protected:
        virtual void maybeTip(const QPoint& p) {
            ListItem* item = dynamic_cast<ListItem*>( m_parent->itemAt(p) );
            if(item) {
                QRect r( m_parent->itemRect(item) );
                if(r.isValid() && item->action()) {
                    tip(r, QString("<qt>%1</qt>").arg(item->action()->toolTip()));
                }
            }
        }
    private:
        K3ListView* m_parent;
};
*/

class WdgScriptsManagerPrivate
{
    friend class WdgScriptsManager;

    K3ListView* m_listview;

    ScriptGUIClient* m_scripguiclient;
    //ToolTip* m_tooltip;
    ScriptNewStuff* newstuff;

    KAction *btnExec, *btnLoad, *btnUnload, *btnInstall, *btnUninstall, *btnNewStuff;
};

WdgScriptsManager::WdgScriptsManager(ScriptGUIClient* scr, QWidget* parent, const char* name)
    : QWidget(parent, name)
    , d( new WdgScriptsManagerPrivate() )
{
    d->m_scripguiclient = scr;
    //d->m_tooltip = new ToolTip(d->m_listview);
    d->newstuff = 0;

    QVBoxLayout* layout = new QVBoxLayout(this);
    d->m_listview = new K3ListView(this);
    layout->addWidget(d->m_listview);
    d->m_listview->header()->hide();
    d->m_listview->setAllColumnsShowFocus(true);
    d->m_listview->setSorting(-1);
    d->m_listview->addColumn("");

    KActionCollection* collection = new KActionCollection(this);
    QMenu* menu = new QMenu(d->m_listview);

    d->btnExec = new KAction(KIcon("player_play"), i18n("Execute"), collection, "execute");
    menu->addAction(d->btnExec);
    connect(d->btnExec, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
            this, SLOT(slotExecuteScript()));

    d->btnLoad = new KAction(KIcon("fileopen"), i18n("Load"), collection, "load");
    menu->addAction(d->btnLoad);
    connect(d->btnLoad, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
            this, SLOT(slotLoadScript()));

    d->btnUnload = new KAction(KIcon("fileclose"), i18n("Unload"), collection, "unload");
    menu->addAction(d->btnUnload);
    connect(d->btnUnload, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
            this, SLOT(slotUnloadScript()));

    d->btnInstall = new KAction(KIcon("fileimport"), i18n("Install"), collection, "install");
    menu->addAction(d->btnInstall);
    connect(d->btnInstall, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
            this, SLOT(slotInstallScript()));

    d->btnUninstall = new KAction(KIcon("fileclose"), i18n("Uninstall"), collection, "uninstall");
    menu->addAction(d->btnUninstall);
    connect(d->btnUninstall, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
            this, SLOT(slotUninstallScript()));

    d->btnNewStuff = new KAction(KIcon("knewstuff"), i18n("Get New Scripts"), collection, "newscripts");
    menu->addAction(d->btnNewStuff);
    connect(d->btnNewStuff, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
            this, SLOT(slotGetNewScript()));

    slotFillScriptsList();
    slotSelectionChanged(0);
    connect(d->m_listview, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotSelectionChanged(Q3ListViewItem*)));
    connect(scr, SIGNAL( collectionChanged(ScriptActionCollection*) ), this, SLOT( slotFillScriptsList() ));
}

WdgScriptsManager::~WdgScriptsManager()
{
    //delete d->m_tooltip;
    delete d;
}

void WdgScriptsManager::slotFillScriptsList()
{
    d->m_listview->clear();

    addItem( d->m_scripguiclient->getActionCollection("executedscripts") );
    addItem( d->m_scripguiclient->getActionCollection("loadedscripts") );
    addItem( d->m_scripguiclient->getActionCollection("installedscripts") );
}

void WdgScriptsManager::addItem(ScriptActionCollection* collection)
{
    if(! collection)
        return;

    ListItem* i = new ListItem(d->m_listview, collection);
    i->setText(0, collection->actionMenu()->text());
    i->setOpen(true);

    Q3ValueList<ScriptAction::Ptr> list = collection->actions();
    Q3ListViewItem* lastitem = 0;
    for(Q3ValueList<ScriptAction::Ptr>::Iterator it = list.begin(); it != list.end(); ++it)
        lastitem = addItem(*it, i, lastitem);
}

Q3ListViewItem* WdgScriptsManager::addItem(ScriptAction::Ptr action, Q3ListViewItem* parentitem, Q3ListViewItem* afteritem)
{
    if(! action)
        return 0;

    ListItem* i = new ListItem(dynamic_cast<ListItem*>(parentitem), afteritem, action);
    i->setText(0, action->text()); // FIXME: i18nise it for ko2.0
    //i->setText(1, action->getDescription()); // FIXME: i18nise it for ko2.0
    //i->setText(2, action->name());

/*
    QPixmap pm;
    if(action->hasIcon()) {
        KIconLoader* icons = KGlobal::iconLoader();
        pm = icons->loadIconSet(action->icon().name(), KIcon::Small).pixmap(QIcon::Small, QIcon::Active);
    }
    else {
        pm = action->iconSet(KIcon::Small, 16).pixmap(QIcon::Small, QIcon::Active);
    }
    if(! pm.isNull())
        i->setPixmap(0, pm); // display the icon
*/

    return i;
}

void WdgScriptsManager::slotSelectionChanged(Q3ListViewItem* item)
{
    ListItem* i = dynamic_cast<ListItem*>(item);
    Kross::Api::ScriptActionCollection* installedcollection = d->m_scripguiclient->getActionCollection("installedscripts");
    d->btnExec->setEnabled(i && i->action());
    d->btnUnload->setEnabled(i && i->action() && i->collection() != installedcollection);
    d->btnUninstall->setEnabled(i && i->action() && i->collection() == installedcollection);
}

void WdgScriptsManager::slotLoadScript()
{
    if(d->m_scripguiclient->loadScriptFile())
        slotFillScriptsList();
}

void WdgScriptsManager::slotInstallScript()
{
    KFileDialog* filedialog = new KFileDialog(
        KUrl(""), // startdir
        "*.tar.gz *.tgz *.bz2", // filter
        this, // widget
        this // parent
    );
    filedialog->setCaption( i18n("Install Script Package") );

    if(! filedialog->exec())
        return;

    if(! d->m_scripguiclient->installScriptPackage( filedialog->selectedURL().path() )) {
        krosswarning("Failed to install scriptpackage");
        return;
    }

    slotFillScriptsList();
}

void WdgScriptsManager::slotUninstallScript()
{
    ListItem* item = dynamic_cast<ListItem*>( d->m_listview->currentItem() );
    if( !item || !item->action() )
        return;

    Kross::Api::ScriptActionCollection* installedcollection = d->m_scripguiclient->getActionCollection("installedscripts");
    if( !item->collection() || item->collection() != installedcollection)
        return;

    const QString packagepath = item->action()->getPackagePath();
    if(packagepath.isEmpty())
        return;

    if( KMessageBox::warningContinueCancel(0,
        i18n("Uninstall the script package \"%1\" and delete the package's folder \"%2\"?"
            ,item->action()->text(), packagepath),
        i18n("Uninstall")) != KMessageBox::Continue )
    {
        return;
    }

    if(! d->m_scripguiclient->uninstallScriptPackage(packagepath)) {
        krosswarning("Failed to uninstall scriptpackage");
        return;
    }

    slotFillScriptsList();
}

void WdgScriptsManager::slotExecuteScript()
{
    ListItem* item = dynamic_cast<ListItem*>( d->m_listview->currentItem() );
    if(item && item->action())
        item->action()->activate();
}

void WdgScriptsManager::slotUnloadScript()
{
    ListItem* item = dynamic_cast<ListItem*>( d->m_listview->currentItem() );
    if(item && item->action()) {
        item->collection()->detach( item->action() );
        slotFillScriptsList();
    }
}

void WdgScriptsManager::slotGetNewScript()
{
    const QString appname = KApplication::kApplication()->objectName();
    const QString type = QString("%1/script").arg(appname);

    if(! d->newstuff) {
        d->newstuff = new ScriptNewStuff(d->m_scripguiclient, type);
        connect(d->newstuff, SIGNAL(installFinished()), this, SLOT(slotResourceInstalled()));
    }

    KNS::Engine *engine = new KNS::Engine(d->newstuff, type, this);
    KNS::DownloadDialog *d = new KNS::DownloadDialog( engine, this );
    d->setType(type);

    KNS::ProviderLoader *p = new KNS::ProviderLoader(this);
    QObject::connect(p, SIGNAL(providersLoaded(Provider::List*)),
                     d, SLOT(slotProviders(Provider::List*)));

    p->load(type, QString("http://download.kde.org/khotnewstuff/%1scripts-providers.xml").arg(appname));
    d->exec();
}

void WdgScriptsManager::slotResourceInstalled()
{
    // Delete KNewStuff's configuration entries. These entries reflect what has
    // already been installed. As we cannot yet keep them in sync after uninstalling
    // scripts, we deactivate the check marks entirely.
    KGlobal::config()->deleteGroup("KNewStuffStatus");
}

}}

#include "wdgscriptsmanager.moc"
