/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
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

#include "kis_kjsembed.h"

// Qt
#include <qpopupmenu.h>

// KDE
#include <kdebug.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kxmlguifactory.h>


// KJSEmbed
#include <kjsembed/jsconsolewidget.h>
#include <kjsembed/kjsembedpart.h>

// Krita
#include "core/kis_view.h"

// KisKJSEmbed
#include "kis_script.h"
#include "functions/kis_functions_factory.h"
#include "objects/kis_objects_factory.h"

namespace Krita {
namespace Plugins {
namespace KisKJSEmbed {

using namespace Bindings;

typedef KGenericFactory<KisKJSEmbed> KritaKJSEmbedFactory;
K_EXPORT_COMPONENT_FACTORY( kritakjsembed, KritaKJSEmbedFactory( "krita" ) )


KisKJSEmbed::KisKJSEmbed(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name), m_scriptMenu(0)
{
    setInstance(KritaKJSEmbedFactory::instance());

    kdDebug(DBG_AREA_PLUGINS) << "KSJEmbed plugin. Class: "
          << className()
          << ", Parent: "
          << parent -> className()
          << "\n";

    if ( !parent->inherits("KisView") )
    {
        return;
    } else {
        m_view = (KisView*) parent;
    }
    m_fileDialog = new KFileDialog( "~", "*.krajs", m_view, "Load a krita JavaScript", true);
    m_jsEmbedPart = new KJSEmbed::KJSEmbedPart(this, "krita KJSEmbed Part");

    initBindings();
#if 0
//      XXX: The console should be a docker. Enabling this causes a crash at the moment, though.
#if KIVIO_STYLE_DOCKERS
     m_consoleDocker = m_view -> toolDockManager() -> createTabbedToolDock("scripting");
     m_consoleDocker -> setCaption(i18n("Scripting console"));
     m_consoleDocker -> restore();
#endif
#endif
     m_jsConsoleWidget = new KJSEmbed::JSConsoleWidget(m_jsEmbedPart);
#if 0
     m_jsConsoleWidget -> show();
     m_jsConsoleWidget -> setCaption("Console");
     m_consoleDocker -> plug( m_jsConsoleWidget );
#endif
      KToggleAction * show = new KToggleAction(i18n( "&Script Console" ), 0, 0,
                          actionCollection(), "view_krita_script_console" );
#if 0
      connect(show, SIGNAL(toggled(bool)), m_consoleDocker, SLOT(makeVisible(bool)));
      connect(m_consoleDocker, SIGNAL(visibleChange(bool)), SLOT(viewConsoleDocker(bool)));
#endif
    connect(show, SIGNAL(toggled(bool)), SLOT(viewConsoleDocker(bool)));

    (void) new KAction(i18n("&Load Script..."), 0, 0, this, SLOT(slotLoadScript()), actionCollection(), "krita_scripts_load");

}

KisKJSEmbed::~KisKJSEmbed()
{
    delete m_functionsFactory;
    delete m_objectsFactory;
}

void KisKJSEmbed::viewConsoleDocker(bool v)
{
    ((KToggleAction*)actionCollection()->action("view_krita_script_console")) -> setChecked(v);
    if (v) {
        m_jsConsoleWidget -> show();
    }
    else {
        m_jsConsoleWidget -> hide();
    }
}

void KisKJSEmbed::initBindings()
{
    m_functionsFactory = new FunctionsFactory( m_jsEmbedPart, m_view );
    m_objectsFactory = new ObjectsFactory( m_jsEmbedPart, m_view );
}

void KisKJSEmbed::slotLoadScript()
{
    if(m_scriptMenu == 0)
    { // This can't be done in the constructor because when it is called the factory is not already available
        m_scriptMenu = static_cast<QPopupMenu*>(factory()->container("LoadedScripts",this));
    }
    if(m_fileDialog->exec() == QDialog::Rejected)
        return;
    QStringList fileList = m_fileDialog->selectedFiles();
    for ( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it ) {
        Script* script = Script::loadFromFile(m_jsEmbedPart, *it );
        if(script != 0)
        {
            m_vScripts.push_back( script );
            m_scriptMenu->insertItem( *it, script, SLOT(execute()) );
        }
    }
}

}; }; };

#include "kis_kjsembed.moc"
