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
// kritacore
#include "kritacore/krs_module.h"
#include "kritacore/krs_monitor.h"
#include "kritacore/krs_progress.h"
#include "kis_layer_manager.h"

#include "kis_script_filter.h"

typedef KGenericFactory<ScriptingPart> KritaScriptingFactory;
K_EXPORT_COMPONENT_FACTORY( kritascripting, KritaScriptingFactory( "krita" ) )

class ScriptingPart::Private
{
    public:
};

ScriptingPart::ScriptingPart(QObject *parent, const QStringList &list)
    : KoScriptingPart(new Scripting::Module( dynamic_cast<KisView2*>(parent) ), list)
    , d(new Private())
{
    kDebug(41011) << "ScriptingPart Ctor" << endl;
    setComponentData(ScriptingPart::componentData());
    setXMLFile(KStandardDirs::locate("data","kritaplugins/scripting.rc"), true);

    // Add filters
    Kross::ActionCollection* actioncollection = Kross::Manager::self().actionCollection();
    if( actioncollection && (actioncollection = actioncollection->collection("filters")) ) {
        foreach(Kross::Action* action, actioncollection->actions()) {
            Q_ASSERT(action);
            KisScriptFilter* sf = new KisScriptFilter(action);
            KisFilterRegistry::instance()->add( KisFilterSP(sf) );
//             Scripting::VariableFactory* factory = Scripting::VariableFactory::create(action);
//             if( ! factory ) continue;
           kDebug(41011) << "Adding scripting filters with id=" << sf->id() << endl;
        }
    }

}

ScriptingPart::~ScriptingPart()
{
    kDebug(41011) << "ScriptingPart Dtor" << endl;
    delete d;
}

void ScriptingPart::myStarted(Kross::Action*)
{
    kDebug(41011) << "1 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    kDebug(41011) << "ScriptingPart::myStarted" << endl;
    Scripting::Monitor::instance()->started();
}

void ScriptingPart::myFinished(Kross::Action*)
{
    kDebug(41011) << "222 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    kDebug(41011) << "ScriptingPart::myFinished" << endl;
#if 0
//     kDebug() << "ScriptingPart::executionFinished" << endl;
    d->view->document()->setModified(true);

//FIXME sebsauer, 20070601, who did remove it without providing an replacment?
//d->view->layerManager()->activeLayer()->setDirty();

    static_cast< Scripting::Progress* >( d->module->progress() )->progressDone();
    QApplication::restoreOverrideCursor();
    //d->module->deleteLater();
#endif

    Scripting::Monitor::instance()->finished();
}

#include "scriptingpart.moc"
