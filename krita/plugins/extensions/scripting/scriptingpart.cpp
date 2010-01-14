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

#include <KToggleAction>
#include <kactioncollection.h>
#include <kis_debug.h>
#include <kfiledialog.h>
#include <kpluginfactory.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>

// kdelibs/kross
#include <kross/core/manager.h>
#include <kross/ui/model.h>
#include <kross/core/action.h>
#include <kross/core/actioncollection.h>

// krita
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view2.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_doc2.h>
#include <kis_paint_device.h>
#include <canvas/kis_canvas2.h>
// kritacore
#include "kritacore/krs_module.h"
#include "kritacore/krs_monitor.h"
#include "kritacore/krs_progress.h"
#include "kis_layer_manager.h"
#include "filter/kis_filter_registry.h"

#include "kis_script_dock.h"
#include "kis_script_filter.h"
#include "kis_script_decoration.h"

K_PLUGIN_FACTORY(KritaScriptingFactory, registerPlugin<ScriptingPart>();)
K_EXPORT_PLUGIN(KritaScriptingFactory("krita"))

class ScriptingPart::Private
{
public:
    KisView2* view;
};

ScriptingPart::ScriptingPart(QObject *parent, const QVariantList &)
        : KoScriptingPart(new Scripting::Module(dynamic_cast<KisView2*>(parent)), QStringList())
        , d(new Private())
{
    d->view = dynamic_cast<KisView2*>(parent);
    dbgScript << "ScriptingPart Ctor";
    setComponentData(ScriptingPart::componentData());
    setXMLFile(KStandardDirs::locate("data", "kritaplugins/scripting.rc"), true);
#if 1
    // Add filters
    Kross::ActionCollection* actioncollection = Kross::Manager::self().actionCollection();
    Kross::ActionCollection* actioncollection2;
    if (actioncollection && (actioncollection2 = actioncollection->collection("filters"))) {
        foreach(Kross::Action* action, actioncollection2->actions()) {
            Q_ASSERT(action);
            if (Kross::Manager::self().hasInterpreterInfo(action->interpreter())) {
                action->addObject(module());
                KisScriptFilter* sf = new KisScriptFilter(action);
                KisFilterRegistry::instance()->add(sf);
                dbgScript << "Adding scripting filters with id=" << sf->id();
            } else {
                dbgScript << "No such interpreter as " << action->interpreter();
            }
        }
    }
    if (actioncollection && (actioncollection2 = actioncollection->collection("dockers"))) {
        foreach(Kross::Action* action, actioncollection2->actions()) {
            Q_ASSERT(action);
            if (Kross::Manager::self().hasInterpreterInfo(action->interpreter())) {
                action->addObject(module());
                dbgScript << "Start Adding scripting dockers with id=" << action->name();
                KisScriptDockFactory ksdf(action);
                d->view->createDockWidget(&ksdf);
            } else {
                dbgScript << "No such interpreter as " << action->interpreter();
            }
        }
    }
    if (actioncollection && (actioncollection2 = actioncollection->collection("decorations"))) {
        foreach(Kross::Action* action, actioncollection2->actions()) {
            Q_ASSERT(action);
            if (Kross::Manager::self().hasInterpreterInfo(action->interpreter())) {
                action->addObject(module());
                dbgScript << "Start Adding scripting decoration with id=" << action->name();
                KisScriptDecoration* ksd = new KisScriptDecoration(action, d->view);
                d->view->canvasBase()->addDecoration(ksd);
                QString menuActionName = "toggle" + action->name() + "Action";
                QString document = "<!DOCTYPE kpartgui SYSTEM \"kpartgui.dtd\"> \
<kpartgui library=\"kritascripting\" version=\"7\"> \
  <MenuBar> \
    <Menu name=\"View\"> \
      <Action name=\"";
                document += menuActionName ;
                document += "\"/> \
    </Menu> \
  </MenuBar> \
</kpartgui>";
                dbgScript << document;
                setXML(document, true);
                dbgScript << "Create an action " << menuActionName;
                KToggleAction *actionMenu  = new KToggleAction(i18n("&Show %1", action->text()), this);
                actionCollection()->addAction(menuActionName, actionMenu);
                connect(actionMenu, SIGNAL(triggered()), ksd, SLOT(toggleVisibility()));
            } else {
                dbgScript << "No such interpreter as " << action->interpreter();
            }
        }
    }
#endif
}

ScriptingPart::~ScriptingPart()
{
    dbgScript << "ScriptingPart Dtor";
    delete d;
}

void ScriptingPart::myStarted(Kross::Action*)
{
    Scripting::Monitor::instance()->started();
}

void ScriptingPart::myFinished(Kross::Action*)
{
#ifdef __GNUC__
#warning "ScriptingPart::myFinished: reimplement, somehow the view variable got lost here."
#endif
#if 0
//     dbgKrita <<"ScriptingPart::executionFinished";
    d->view->document()->setModified(true);

    d->view->layerManager()->activeLayer()->setDirty();

    static_cast< Scripting::Progress* >(d->module->progress())->progressDone();
    //d->module->deleteLater();
#endif
    QApplication::restoreOverrideCursor();
    Scripting::Monitor::instance()->finished();
}

#include "scriptingpart.moc"
