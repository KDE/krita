/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
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

#include "scriptingpart.h"
#include "scriptingmonitor.h"

#include <stdlib.h>
#include <vector>

#include <QApplication>
#include <QPoint>
#include <QPointer>
#include <QToolBar>

#include <kdebug.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include <kopalettemanager.h>

#include <kis_doc.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_image.h>
#include <kis_layer.h>

#define KROSS_MAIN_EXPORT KDE_EXPORT

#include <core/manager.h>
#include <core/guiclient.h>
#include <core/guimanager.h>

#include "kritacore/kritacoremodule.h"
#include "kritacore/kritacoreprogress.h"

typedef KGenericFactory<ScriptingPart> KritaScriptingFactory;
K_EXPORT_COMPONENT_FACTORY( kritascripting, KritaScriptingFactory( "krita" ) )

class ScriptingPart::Private
{
    public:
        KisView* view;
        Kross::GUIClient* guiclient;
        QPointer< Kross::KritaCore::KritaCoreModule > module;

        Private() : view(0), guiclient(0), module(0) {}
        virtual ~Private() {
            //Kross::Manager::self().removeObject(module);
            delete module; module = 0;
        }
};

class ScriptingDocker : public QWidget
{
    public:
        ScriptingDocker(QWidget* parent, Kross::GUIClient* guiclient) : QWidget(parent)
        {
            QBoxLayout* layout = new QVBoxLayout(this);
            layout->setMargin(0);
            setLayout(layout);

            Kross::GUIManagerView* view = new Kross::GUIManagerView(guiclient, this, false);
            layout->addWidget(view, 1);
            KActionCollection* collection = view->actionCollection();

            QToolBar* tb = new QToolBar(this);
            layout->addWidget(tb);
            tb->setMovable(false);
            //tb->setOrientation(Qt::Vertical);
            //tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            tb->addAction( collection->action("runscript") );
            tb->addAction( collection->action("stopscript") );
        }

        virtual ~ScriptingDocker() {}
};

ScriptingPart::ScriptingPart(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
    , d(new Private())
{
    setInstance(ScriptingPart::instance());

    d->view = dynamic_cast< KisView* >(parent);
    Q_ASSERT(d->view);

    d->guiclient = new Kross::GUIClient( d->view, d->view );

    //d->guiclient ->setXMLFile(locate("data","kritaplugins/scripting.rc"), true);
    //BEGIN TODO: understand why the ScriptGUIClient doesn't "link" its actions to the menu
    setXMLFile(KStandardDirs::locate("data","kritaplugins/scripting.rc"), true);

    // Setup the actions Kross provides and KSpread likes to have.
    KAction* execaction = new KAction(i18n("Execute Script File..."), actionCollection(), "executescriptfile");
    connect(execaction, SIGNAL(triggered(bool)), d->guiclient, SLOT(executeFile()));

    KAction* manageraction = new KAction(i18n("Script Manager..."), actionCollection(), "configurescripts");
    connect(manageraction, SIGNAL(triggered(bool)), d->guiclient, SLOT(showManager()));

    KAction* scriptmenuaction = d->guiclient->action("scripts");
    actionCollection()->insert(scriptmenuaction);

    d->view->createDock(i18n("Scripts"), new ScriptingDocker(d->view, d->guiclient));

    connect(d->guiclient, SIGNAL(executionFinished(Kross::Action*)), this, SLOT(executionFinished(Kross::Action*)));
    connect(d->guiclient, SIGNAL(executionStarted(Kross::Action*)), this, SLOT(executionStarted(Kross::Action*)));

    // do we still need the monitor?
    ScriptingMonitor::instance()->monitor( d->guiclient );
}

ScriptingPart::~ScriptingPart()
{
    if(d->module) {
        kDebug() << "ScriptingPart::~ScriptingPart Execution is still running. Trying to free it." << endl;
        d->module->deleteLater();
    }
    delete d;
}

void ScriptingPart::executionFinished(Kross::Action*)
{
    kDebug() << "ScriptingPart::executionFinished" << endl;
    d->view->canvasSubject()->document()->setModified(true);
    d->view->canvasSubject()->document()->currentImage()->activeLayer()->setDirty();
    static_cast< Kross::KritaCore::KritaCoreProgress* >( d->module->progress() )->progressDone();
    QApplication::restoreOverrideCursor();
    //d->module->deleteLater();
}

void ScriptingPart::executionStarted(Kross::Action* action)
{
    Q_UNUSED(action);
    kDebug() << "ScriptingPart::executionStarted" << endl;

    if(d->module)
        delete d->module;
    d->module = new Kross::KritaCore::KritaCoreModule(d->view);
    Kross::Manager::self().addObject(d->module, "Krita");
#if 0
    kDebug(41011) << act->getPackagePath() << endl;
    progress->setPackagePath( act->getPackagePath() );
#endif
}


#include "scriptingpart.moc"
