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
#include <kcomponentdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>

// krita
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view2.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_doc2.h>
// koffice/libs/kokross
#include <KoScriptingDocker.h>
// kdelibs/kross
#include <kross/core/manager.h>
#include <kross/core/model.h>
#include <kross/core/guiclient.h>
// kritacore
#include "kritacore/krs_module.h"
#include "kritacore/krs_progress.h"

typedef KGenericFactory<ScriptingPart> KritaScriptingFactory;
K_EXPORT_COMPONENT_FACTORY( kritascripting, KritaScriptingFactory( "krita" ) )

class ScriptingPart::Private
{
    public:
        KisView2* view;
        Kross::GUIClient* guiclient;
        QPointer< Scripting::Module > module;
};

ScriptingPart::ScriptingPart(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
    , d(new Private())
{
    setComponentData(ScriptingPart::componentData());

    d->view = dynamic_cast< KisView2* >(parent);
    Q_ASSERT(d->view);

    d->guiclient = new Kross::GUIClient( d->view, d->view );

    //d->guiclient ->setXMLFile(locate("data","kritaplugins/scripting.rc"), true);
    //BEGIN TODO: understand why the ScriptGUIClient doesn't "link" its actions to the menu
    setXMLFile(KStandardDirs::locate("data","kritaplugins/scripting.rc"), true);

    d->module = new Scripting::Module(d->view);

    // Setup the actions Kross provides and Krita likes to have.
    KAction* execaction  = new KAction(i18n("Execute Script File..."), this);
    actionCollection()->addAction("executescriptfile", execaction );
    connect(execaction, SIGNAL(triggered(bool)), d->guiclient, SLOT(executeFile()));

    KAction* manageraction  = new KAction(i18n("Script Manager..."), this);
    actionCollection()->addAction("configurescripts", manageraction );
    connect(manageraction, SIGNAL(triggered(bool)), d->guiclient, SLOT(showManager()));

    QAction* scriptmenuaction = d->guiclient->action("scripts");
    actionCollection()->addAction("scripts", scriptmenuaction);

    KoScriptingDockerFactory factory(d->view, d->guiclient);
    QDockWidget* dock = d->view->createDockWidget(&factory);
    Q_UNUSED(dock);

    connect(&Kross::Manager::self(), SIGNAL(started(Kross::Action*)), this, SLOT(started(Kross::Action*)));
    connect(&Kross::Manager::self(), SIGNAL(finished(Kross::Action*)), this, SLOT(finished(Kross::Action*)));
}

ScriptingPart::~ScriptingPart()
{
    delete d->module;
    delete d;
}

void ScriptingPart::started(Kross::Action*)
{
    kDebug() << "ScriptingPart::executionStarted" << endl;
    Kross::Manager::self().addObject(d->module, "Krita");
}

void ScriptingPart::finished(Kross::Action*)
{
    kDebug() << "ScriptingPart::executionFinished" << endl;
    d->view->document()->setModified(true);
    d->view->image()->activeLayer()->setDirty();
    static_cast< Scripting::Progress* >( d->module->progress() )->progressDone();
    QApplication::restoreOverrideCursor();
    //d->module->deleteLater();
}

#include "scriptingpart.moc"
