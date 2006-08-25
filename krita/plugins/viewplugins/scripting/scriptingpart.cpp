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
#include "scriptingprogress.h"
#include "scriptingmodule.h"

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

#define KROSS_MAIN_EXPORT KDE_EXPORT

#include <core/manager.h>
#include <core/guiclient.h>
#include <core/guimanager.h>

#include <kopalettemanager.h>

#include <kis_doc.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_image.h>
#include <kis_layer.h>

typedef KGenericFactory<ScriptingPart> KritaScriptingFactory;
K_EXPORT_COMPONENT_FACTORY( kritascripting, KritaScriptingFactory( "krita" ) )

class ScriptingViewWidget : public Kross::GUIManagerView
{
    public:
        ScriptingViewWidget(Kross::GUIClient* guiclient, QWidget* parent)
            : Kross::GUIManagerView(guiclient, parent) {}
        virtual ~ScriptingViewWidget() {}
};

ScriptingPart::ScriptingPart(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    //setInstance(KritaScriptingFactory::instance());
    setInstance(ScriptingPart::instance());

    m_view = dynamic_cast< KisView* >(parent);
    Q_ASSERT(m_view);

    m_scriptguiclient = new Kross::GUIClient( m_view, m_view );

    //m_scriptguiclient ->setXMLFile(locate("data","kritaplugins/scripting.rc"), true);
    //BEGIN TODO: understand why the ScriptGUIClient doesn't "link" its actions to the menu
    setXMLFile(KStandardDirs::locate("data","kritaplugins/scripting.rc"), true);

    // Setup the actions Kross provides and KSpread likes to have.
    KAction* execaction = new KAction(i18n("Execute Script File..."), actionCollection(), "executescriptfile");
    connect(execaction, SIGNAL(triggered(bool)), m_scriptguiclient, SLOT(executeFile()));

    KAction* manageraction = new KAction(i18n("Script Manager..."), actionCollection(), "configurescripts");
    connect(manageraction, SIGNAL(triggered(bool)), m_scriptguiclient, SLOT(showManager()));

    KAction* scriptmenuaction = m_scriptguiclient->action("scripts");
    actionCollection()->insert(scriptmenuaction);

#if 0
    QWidget* w = new Kross::WdgScriptsManager(m_scriptguiclient, m_view);
    m_view->canvasSubject()->paletteManager()->addWidget(w, "Scripts Manager", krita::LAYERBOX, 10,  PALETTE_DOCKER, false);
#else
    QWidget* w = new ScriptingViewWidget(m_scriptguiclient, m_view);
    m_view->canvasSubject()->paletteManager()->addWidget(w, "Scripts Manager", krita::LAYERBOX, 10,  PALETTE_DOCKER, false);
#endif

    connect(m_scriptguiclient, SIGNAL(executionFinished( const Kross::ScriptAction* )), this, SLOT(executionFinished(const Kross::ScriptAction*)));
    connect(m_scriptguiclient, SIGNAL(executionStarted( const Kross::ScriptAction* )), this, SLOT(executionStarted(const Kross::ScriptAction*)));

    ScriptingMonitor::instance()->monitor( m_scriptguiclient );
    m_scriptProgress = new ScriptingProgress(m_view);
    m_module = new ScriptingModule(m_view, m_scriptProgress);
    Kross::Manager::self().addObject(m_module, "Krita");
}

ScriptingPart::~ScriptingPart()
{
    //Kross::Manager::self().removeObject(m_module);
    delete m_module;
    m_module = 0;
    delete m_scriptProgress;
    m_scriptProgress = 0;
}

void ScriptingPart::executionFinished(const Kross::Action*)
{
    m_view->canvasSubject()->document()->setModified(true);
    m_view->canvasSubject()->document()->currentImage()->activeLayer()->setDirty();
    m_scriptProgress->progressDone();
    QApplication::restoreOverrideCursor();
}

void ScriptingPart::executionStarted(const Kross::Action* act)
{
#if 0
    kDebug(41011) << act->getPackagePath() << endl;
    m_scriptProgress->setPackagePath( act->getPackagePath() );
#endif
}


#include "scriptingpart.moc"
