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

#include <kis_doc.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>


#include "kis_scripts_registry.h"
#include "kis_script.h"
#include "wdgscriptsmanager.h"

typedef KGenericFactory<Scripting> KritaScriptingFactory;
K_EXPORT_COMPONENT_FACTORY( kritascripting, KritaScriptingFactory( "krita" ) )

Scripting::Scripting(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(KritaScriptingFactory::instance());


    kdDebug(41006) << "Scripting plugin. Class: "
          << className()
          << ", Parent: "
          << parent -> className()
          << "\n";
    if ( parent->inherits("KisView") )
    {
        setInstance(Scripting::instance());
        setXMLFile(locate("data","kritaplugins/scripting.rc"), true);
        m_view = (KisView*) parent;

        (void) new KAction(i18n("&Load Script"), 0, 0, this, SLOT(slotLoadScript()), actionCollection(), "loadscript");
        (void) new KAction(i18n("Load && &Execute Script"), 0, 0, this, SLOT(slotLoadAndExecuteScript()), actionCollection(), "loadandexecutescript");
        (void) new KAction(i18n("&Show Script Manager"), 0, 0, this, SLOT(slotShowManager()), actionCollection(), "showmanager");
        
        Kross::Api::Manager::scriptManager()->addQObject(m_view->getCanvasSubject()->document(), "KritaDocument");
        Kross::Api::Manager::scriptManager()->addQObject(m_view, "KritaView");
    }

}

Scripting::~Scripting()
{
}

KisScript* Scripting::loadScript(bool exec)
{
    KFileDialog* kfd = new KFileDialog(QString::null, "*.py", m_view, "", true);
    kfd->setCaption("load a script");
    if(kfd->exec())
    {
        KURL fn = kfd->selectedURL();
        kdDebug() << fn << endl;
        KisScriptSP ks;
        if( KisScriptsRegistry::instance()->exists(fn.path()) )
        {
            // Scripts already exist, ask for reload
            ks = KisScriptsRegistry::instance()->get(fn.path());
            if( KMessageBox::questionYesNo( m_view, i18n("This scripts exists already, do you want to reload it ?"), i18n("Scripts already loaded") ) == KMessageBox::Yes)
            {
                ks->reload();
            }
        } else {
            KisScriptsRegistry::instance()->add( ks = new KisScript(fn, m_view, exec) );
        }
        return ks;
    }
    return 0;
}


void Scripting::slotLoadScript()
{
    loadScript(false);
}
void Scripting::slotLoadAndExecuteScript()
{
    loadScript(true);
}

void Scripting::slotShowManager()
{
    KDialogBase* kdb = new KDialogBase(m_view, "", true, i18n("Script Manager"), KDialogBase::Close);
    WdgScriptsManager* wsm = new WdgScriptsManager(this, kdb);
    kdb->setMainWidget(wsm);
    kdb->show();
}



#include "scripting.moc"
