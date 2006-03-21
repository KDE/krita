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
#include "scripting.h"

#include <stdlib.h>
#include <vector>

#include <qapplication.h>
#include <qpoint.h>

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
#include <main/manager.h>
#include <main/scriptguiclient.h>
#include <main/wdgscriptsmanager.h>

#include <kopalettemanager.h>

#include <kis_doc.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_image.h>
#include <kis_layer.h>

#include "kritascripting/kis_script_progress.h"
#include "kritascripting/kis_script_monitor.h"

typedef KGenericFactory<Scripting> KritaScriptingFactory;
K_EXPORT_COMPONENT_FACTORY( kritascripting, KritaScriptingFactory( "krita" ) )

Scripting::Scripting(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(KritaScriptingFactory::instance());


    if ( parent->inherits("KisView") )
    {
        setInstance(Scripting::instance());
        m_view = (KisView*) parent;
        m_scriptguiclient = new Kross::Api::ScriptGUIClient( m_view, m_view );
//         m_scriptguiclient ->setXMLFile(locate("data","kritaplugins/scripting.rc"), true);
        //BEGIN TODO: understand why the ScriptGUIClient doesn't "link" its actions to the menu
        setXMLFile(locate("data","kritaplugins/scripting.rc"), true);
        new KAction(i18n("Execute Script File..."), 0, 0, m_scriptguiclient, SLOT(executeScriptFile()), actionCollection(), "executescriptfile");
        new KAction(i18n("Script Manager..."), 0, 0, m_scriptguiclient, SLOT(showScriptManager()), actionCollection(), "configurescripts");
        //END TODO

        QWidget * w = new Kross::Api::WdgScriptsManager(m_scriptguiclient, m_view);

        m_view->canvasSubject()->paletteManager()->addWidget(w, "Scripts Manager", krita::LAYERBOX, 10,  PALETTE_DOCKER, false);

        connect(m_scriptguiclient, SIGNAL(executionFinished( const Kross::Api::ScriptAction* )), this, SLOT(executionFinished(const Kross::Api::ScriptAction*)));
        connect(m_scriptguiclient, SIGNAL(executionStarted( const Kross::Api::ScriptAction* )), this, SLOT(executionStarted(const Kross::Api::ScriptAction*)));
        KisScriptMonitor::instance()->monitor( m_scriptguiclient );

        Kross::Api::Manager::scriptManager()->addQObject(m_view->canvasSubject()->document(), "KritaDocument");
        Kross::Api::Manager::scriptManager()->addQObject(m_view, "KritaView");
        m_scriptProgress = new KisScriptProgress(m_view);
        Kross::Api::Manager::scriptManager()->addQObject(m_scriptProgress, "KritaScriptProgress");

    }

}

Scripting::~Scripting()
{
}

void Scripting::executionFinished(const Kross::Api::ScriptAction*)
{
    m_view->canvasSubject()->document()->setModified(true);
    m_view->canvasSubject()->document()->currentImage()->activeLayer()->setDirty();
    m_scriptProgress->progressDone();
    QApplication::restoreOverrideCursor();
}

void Scripting::executionStarted(const Kross::Api::ScriptAction* act)
{
    kdDebug(41011) << act->getPackagePath() << endl;
    m_scriptProgress->setPackagePath( act->getPackagePath() );
}


#include "scripting.moc"
