/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kis_kjsembed.h"

// Qt
#include <qpopupmenu.h>

// KDE
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kxmlguifactory.h>

// KJSEmbed
#include <kjsembed/jsconsolewidget.h> 
#include <kjsembed/kjsembedpart.h>

// Krita
#include "kis_view.h"

// KisKJSEmbed
#include "kis_kjsembed_script.h"

typedef KGenericFactory<KisKJSEmbed> KritaKJSEmbedFactory;
K_EXPORT_COMPONENT_FACTORY( kritakjsembed, KritaKJSEmbedFactory( "krita" ) )

KisKJSEmbed::KisKJSEmbed(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
	setInstance(KritaKJSEmbedFactory::instance());

	//kdDebug() << "Example plugin. Class: " 
	//	  << className() 
	//	  << ", Parent: " 
	//	  << parent -> className()
	//	  << "\n";

	if ( !parent->inherits("KisView") )
	{
		return;
	} else {
		m_view = (KisView*) parent;
	}
	m_fileDialog = new KFileDialog( "~", "*.krajs",m_view, "Load a krita JavaScript", true);
	m_jsEmbedPart = new KJSEmbed::KJSEmbedPart(this, "krita KJSEmbed Part");
	
	m_jsConsoleWidget = new KJSEmbed::JSConsoleWidget(m_jsEmbedPart, m_view);
// 	win->show();
	
	initBindings();
	(void) new KAction(i18n("&Load script..."), 0, 0, this, SLOT(slotLoadScript()), actionCollection(), "krita_scripts_load");
	
	m_scriptMenu = dynamic_cast<QPopupMenu*>(factory()->container("LoadedScripts",this));
}

KisKJSEmbed::~KisKJSEmbed()
{
}

void KisKJSEmbed::initBindings()
{
	
}

void KisKJSEmbed::slotLoadScript()
{
	m_fileDialog->show();
	if(m_fileDialog->result() == QDialog::Rejected)
		return;
	QStringList fileList = m_fileDialog->selectedFiles();
	for ( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it ) {
		KisKJSEmbedScript* script = KisKJSEmbedScript::loadFromFile(m_jsEmbedPart, *it );
		if(script != 0)
		{
			m_vScripts.push_back( script );
			m_scriptMenu->insertItem( *it, script, SLOT(execute()) );
		}
	}
}

KisPaintDeviceKJSImp::KisPaintDeviceKJSImp( KJS::ExecState *exec, int id  ) : JSProxyImp(exec)
{

}
KisPaintDeviceKJSImp::~KisPaintDeviceKJSImp()
{

}
void KisPaintDeviceKJSImp::addBindings( KJS::ExecState *exec, KJS::Object &object )
{

}

