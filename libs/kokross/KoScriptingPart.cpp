/***************************************************************************
 * KoScriptingPart.cpp
 * This file is part of the KDE project
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
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

#include "KoScriptingPart.h"
#include "KoScriptingModule.h"
#include "KoScriptManager.h"

// qt
#include <QApplication>
// kde
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kdialog.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kfiledialog.h>
// #include <kmimetype.h>
// #include <kmessagebox.h>
// #include <kurl.h>
#include <kdebug.h>
// koffice
#include <KoMainWindow.h>
#include <KoScriptingDocker.h>
//#include <KoApplicationAdaptor.h>
//#include <KoDocumentAdaptor.h>
//#include <KoView.h>
#include <kross/core/manager.h>
#include <kross/core/interpreter.h>
//#include <kross/core/model.h>
#include <kross/core/actioncollection.h>
#include <kross/ui/view.h>

/// \internal d-pointer class.
class KoScriptingPart::Private
{
    public:
        KoScriptingModule* module;
        KActionMenu* scriptsmenu;
        QList< Kross::Action* > actions;
};

KoScriptingPart::KoScriptingPart(KoScriptingModule* module, const QStringList&)
    : KParts::Plugin(module)
    , d(new Private())
{
    d->module = module;
    Q_ASSERT(d->module);

    KAction* execaction  = new KAction(i18n("Execute Script File..."), this);
    actionCollection()->addAction("executescriptfile", execaction);
    connect(execaction, SIGNAL(triggered(bool)), this, SLOT(slotShowExecuteScriptFile()));

    d->scriptsmenu = new KActionMenu(i18n("Scripts"), this);
    actionCollection()->addAction("scripts", d->scriptsmenu);
    connect(d->scriptsmenu->menu(), SIGNAL(aboutToShow()), this, SLOT(slotMenuAboutToShow()));

    KAction* manageraction  = new KAction(i18n("Script Manager..."), this);
    actionCollection()->addAction("scriptmanager", manageraction);
    connect(manageraction, SIGNAL(triggered(bool)), this, SLOT(slotShowScriptManager()));

    connect(&Kross::Manager::self(), SIGNAL(started(Kross::Action*)), this, SLOT(slotStarted(Kross::Action*)));
    connect(&Kross::Manager::self(), SIGNAL(finished(Kross::Action*)), this, SLOT(slotFinished(Kross::Action*)));

    if( Kross::Manager::self().actionCollection()->actions().size() <= 0) {
        QByteArray partname = componentData().componentName(); //KApplication::kApplication()->objectName()
        if( ! partname.isNull() )
            Kross::Manager::self().actionCollection()->readXmlResources("data", partname + "/scripts/*.rc");
    }

    KoView* view = d->module->view();
    Q_ASSERT( view );
    KoMainWindow* mainwindow = view->shell();
    Q_ASSERT( mainwindow );
    KoScriptingDockerFactory factory( mainwindow );
    QDockWidget* docker = mainwindow->createDockWidget(&factory);
    Q_UNUSED(docker);
}

KoScriptingPart::~KoScriptingPart()
{
    foreach(Kross::Action* action, d->actions)
        if( action )
            action->finalize();
    delete d;
}

KoScriptingModule* KoScriptingPart::module() const
{
    return d->module;
}

bool KoScriptingPart::showExecuteScriptFile()
{
    QStringList mimetypes;
    foreach(QString interpretername, Kross::Manager::self().interpreters()) {
        Kross::InterpreterInfo* info = Kross::Manager::self().interpreterInfo(interpretername);
        Q_ASSERT( info );
        mimetypes.append( info->mimeTypes().join(" ").trimmed() );
    }
    KFileDialog* filedialog = new KFileDialog(
        KUrl("kfiledialog:///KrossExecuteScript"), // startdir
        mimetypes.join(" "), // filter
        0, // custom widget
        0 // parent
    );
    filedialog->setCaption( i18n("Execute Script File") );
    filedialog->setOperationMode( KFileDialog::Opening );
    filedialog->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
    return filedialog->exec() ? Kross::Manager::self().executeScriptFile( filedialog->selectedUrl().path() ) : false;
}

KDialog* KoScriptingPart::showScriptManager()
{
    KDialog* dialog = new KDialog();
    dialog->setCaption( i18n("Script Manager") );
    dialog->setButtons( KDialog::Ok | KDialog::Cancel );
    dialog->setMainWidget( new KoScriptManagerCollection(dialog->mainWidget()) );
    dialog->resize( QSize(520, 380).expandedTo( dialog->minimumSizeHint() ) );
    return dialog;
}

void addMenu(QMenu* menu, Kross::ActionCollection* collection)
{
    foreach(Kross::Action* a, collection->actions())
        menu->addAction(a);
    foreach(QString collectionname, collection->collections()) {
        Kross::ActionCollection* c = collection->collection(collectionname);
        if( c->isEnabled() )
            addMenu(menu->addMenu( c->text() ), c);
    }
}

void KoScriptingPart::slotMenuAboutToShow()
{
    d->scriptsmenu->menu()->clear();
    addMenu(d->scriptsmenu->menu(), Kross::Manager::self().actionCollection());
}

void KoScriptingPart::slotShowExecuteScriptFile()
{
    showExecuteScriptFile();
}

void KoScriptingPart::slotShowScriptManager()
{
    KDialog* dialog = showScriptManager();
    int result = dialog->exec();
    /*
    if ( view->isModified() ) {
        if( result == QDialog::Accepted ) { //&& dialog->result() == KDialog::Ok ) {
            // save new config
            Manager::self().writeConfig();
        }
        else {
            // restore old config
            Manager::self().readConfig();
        }
        QMetaObject::invokeMethod(&Manager::self(), "configChanged");
    }
    */
    Q_UNUSED(result);
    dialog->delayedDestruct();
}

void KoScriptingPart::slotStarted(Kross::Action* action)
{
    kDebug(32010) << "KoScriptingPart::slotStarted action=" << action->objectName() << endl;
    KoMainWindow* mainwin = dynamic_cast< KoMainWindow* >( qApp->activeWindow() );
    KoView* view = mainwin ? d->module->view() : 0;
    if( view && view->shell() == mainwin && view == mainwin->rootView() ) {
        action->addObject(d->module);
        d->actions.append( action );
        connect(action, SIGNAL(finalized(Kross::Action*)), this, SLOT(slotFinalized(Kross::Action*)));
    }
}

void KoScriptingPart::slotFinished(Kross::Action* action)
{
    kDebug(32010) << "KoScriptingPart::slotFinished action=" << action->objectName() << endl;
    KoView* view = d->module->view();
    KoMainWindow* mainwindow = view->shell();
    if( view && mainwindow && view == mainwindow->rootView() ) {
        if( action->hadError() ) {
            if( action->errorTrace().isNull() )
                KMessageBox::error(0, action->errorMessage());
            else
                KMessageBox::detailedError(0, action->errorMessage(), action->errorTrace());
        }
    }
    //d->view->document()->setModified(true);
    //QApplication::restoreOverrideCursor();
}

void KoScriptingPart::slotFinalized(Kross::Action* action)
{
    kDebug(32010) << "KoScriptingPart::slotFinalized action=" << action->objectName() << endl;
    d->actions.removeAll(action);
}

#include "KoScriptingPart.moc"
