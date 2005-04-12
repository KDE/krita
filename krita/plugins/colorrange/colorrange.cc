/*
 * colorrange.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include <math.h>

#include <stdlib.h>

#include <qslider.h>
#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_tool_registry.h>
#include "colorrange.h"
#include "dlg_colorrange.h"
#include "kis_tool_selectpicker.h"

typedef KGenericFactory<ColorRange> ColorRangeFactory;
K_EXPORT_COMPONENT_FACTORY( kritacolorrange, ColorRangeFactory( "krita" ) )

ColorRange::ColorRange(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
	setInstance(ColorRangeFactory::instance());

 	kdDebug() << "Colorrange plugin. Class: "
 		  << className()
 		  << ", Parent: "
 		  << parent -> className()
 		  << "\n";


	if ( !parent->inherits("KisView") )
	{
		m_view = 0;
	} else {
		m_view = dynamic_cast<KisView*>(parent);

		m_view -> selectionManager() -> addSelectionAction( new KAction(i18n("&Color Range..."), 0, 0, this, SLOT(slotActivated()), actionCollection(), "colorrange") );

		m_view -> toolRegistry() -> add(new KisToolSelectPickerFactory(actionCollection()));
	}
}

ColorRange::~ColorRange()
{
}

void ColorRange::slotActivated()
{
	KisLayerSP layer = m_view -> currentImg() -> activeLayer();
	if (!layer) return;

	DlgColorRange * dlgColorRange = new DlgColorRange(m_view, layer, m_view, "ColorRange");
	Q_CHECK_PTR(dlgColorRange);

	dlgColorRange -> exec();
}

#include "colorrange.moc"

