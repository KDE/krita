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

	kdDebug() << "KSJEmbed plugin. Class: " 
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
	m_fileDialog = new KFileDialog( "~", "*.krajs",m_view, "Load a krita JavaScript", true);
	m_jsEmbedPart = new KJSEmbed::KJSEmbedPart(this, "krita KJSEmbed Part");

		
	m_jsConsoleWidget = new KJSEmbed::JSConsoleWidget(m_jsEmbedPart, 0);
	m_jsConsoleWidget->show();
	
	initBindings();
	(void) new KAction(i18n("&Load Script..."), 0, 0, this, SLOT(slotLoadScript()), actionCollection(), "krita_scripts_load");
	
}

KisKJSEmbed::~KisKJSEmbed()
{
	delete m_functionsFactory;
	delete m_objectsFactory;
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
			kdDebug() << "KisKJSEmbed::slotLoadScript() script " << *it << " successfully loaded." << endl;
			m_vScripts.push_back( script );
			m_scriptMenu->insertItem( *it, script, SLOT(execute()) );
		}
	}
}

}; }; };

#include "kis_kjsembed.moc"
