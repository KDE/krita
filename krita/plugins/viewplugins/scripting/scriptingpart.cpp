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
#include "scriptingdocker.h"

#include <stdlib.h>
#include <vector>

#include <QApplication>
#include <QPoint>
#include <QPointer>

#include <kactioncollection.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>

#include <kis_global.h>
#include <kis_types.h>
#include <kis_view2.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_doc2.h>

#define KROSS_MAIN_EXPORT KDE_EXPORT

#include <kross/core/manager.h>
#include <kross/core/model.h>
#include <kross/core/guiclient.h>

#include "kritacore/kritacoremodule.h"
#include "kritacore/kritacoreprogress.h"

typedef KGenericFactory<ScriptingPart> KritaScriptingFactory;
K_EXPORT_COMPONENT_FACTORY( kritascripting, KritaScriptingFactory( "krita" ) )

class ScriptingPart::Private
{
    public:
        KisView2* view;
        Kross::GUIClient* guiclient;
        QPointer< Kross::KritaCore::KritaCoreModule > module;

        Private() : view(0), guiclient(0), module(0) {}
        virtual ~Private() {
            //Kross::Manager::self().removeObject(module);
            delete module; module = 0;
        }
};

ScriptingPart::ScriptingPart(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
    , d(new Private())
{
    setInstance(ScriptingPart::instance());

    d->view = dynamic_cast< KisView2* >(parent);
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

    connect(&Kross::Manager::self(), SIGNAL(started(Kross::Action*)), this, SLOT(started(Kross::Action*)));
    connect(&Kross::Manager::self(), SIGNAL(finished(Kross::Action*)), this, SLOT(finished(Kross::Action*)));

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

void ScriptingPart::started(Kross::Action* action)
{
    Q_UNUSED(action);
    kDebug() << "ScriptingPart::executionStarted" << endl;

    if(d->module)
        delete d->module;
    d->module = new Kross::KritaCore::KritaCoreModule(d->view);
    Kross::Manager::self().addObject(d->module, "Krita");

    //kDebug(41011) << act->getPackagePath() << endl;
    //progress->setPackagePath( act->getPackagePath() );
}

void ScriptingPart::finished(Kross::Action*)
{
    kDebug() << "ScriptingPart::executionFinished" << endl;
    d->view->document()->setModified(true);
    d->view->image()->activeLayer()->setDirty();
    static_cast< Kross::KritaCore::KritaCoreProgress* >( d->module->progress() )->progressDone();
    QApplication::restoreOverrideCursor();
    //d->module->deleteLater();
}

#include "scriptingpart.moc"
