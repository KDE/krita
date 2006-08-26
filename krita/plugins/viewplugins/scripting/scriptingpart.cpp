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
        Kross::KritaCore::KritaCoreModule* module;

        Private() : view(0), guiclient(0), module(0) {}
        virtual ~Private() {
            //Kross::Manager::self().removeObject(module);
            delete module; module = 0;
        }
};

class ScriptingViewWidget : public Kross::GUIManagerView
{
    public:
        ScriptingViewWidget(Kross::GUIClient* guiclient, QWidget* parent)
            : Kross::GUIManagerView(guiclient, parent) {}
        virtual ~ScriptingViewWidget() {}
};

ScriptingPart::ScriptingPart(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
    , d(new Private())
{
    //setInstance(KritaScriptingFactory::instance());
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

#if 0
    QWidget* w = new Kross::WdgScriptsManager(d->guiclient, d->view);
    d->view->canvasSubject()->paletteManager()->addWidget(w, "Scripts Manager", krita::LAYERBOX, 10,  PALETTE_DOCKER, false);
#else
    QWidget* w = new ScriptingViewWidget(d->guiclient, d->view);
    d->view->canvasSubject()->paletteManager()->addWidget(w, "Scripts Manager", krita::LAYERBOX, 10,  PALETTE_DOCKER, false);
#endif

    connect(d->guiclient, SIGNAL(executionFinished( const Kross::ScriptAction* )), this, SLOT(executionFinished(const Kross::ScriptAction*)));
    connect(d->guiclient, SIGNAL(executionStarted( const Kross::ScriptAction* )), this, SLOT(executionStarted(const Kross::ScriptAction*)));

    // do we need the monitor?
    ScriptingMonitor::instance()->monitor( d->guiclient );

    d->module = new Kross::KritaCore::KritaCoreModule(d->view);
    Kross::Manager::self().addObject(d->module, "Krita");
}

ScriptingPart::~ScriptingPart()
{
    delete d;
}

void ScriptingPart::executionFinished(const Kross::Action*)
{
    d->view->canvasSubject()->document()->setModified(true);
    d->view->canvasSubject()->document()->currentImage()->activeLayer()->setDirty();
    static_cast< Kross::KritaCore::KritaCoreProgress* >( d->module->progress() )->progressDone();
    QApplication::restoreOverrideCursor();
}

void ScriptingPart::executionStarted(const Kross::Action* act)
{
#if 0
    kDebug(41011) << act->getPackagePath() << endl;
    progress->setPackagePath( act->getPackagePath() );
#endif
}


#include "scriptingpart.moc"
