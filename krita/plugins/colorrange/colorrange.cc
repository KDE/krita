/*
 * variation.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
#include <kis_tile_command.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kistile.h>
#include <kistilemgr.h>
#include <kis_iterators.h>
#include <kis_selection.h>

#include "colorrange.h"
#include "dlg_colorrange.h"

typedef KGenericFactory<ColorRange> ColorRangeFactory;
K_EXPORT_COMPONENT_FACTORY( colorrange, ColorRangeFactory( "krita" ) )

ColorRange::ColorRange(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
	setInstance(ColorRangeFactory::instance());
	kdDebug() << "Colorrange\n";

	(void) new KAction(i18n("&ColorRange..."), 0, 0, this, SLOT(slotActivated()), actionCollection(), "colorrange");
	
	if ( !parent->inherits("KisView") )
	{
		m_view = 0;
	} else {
		m_view = (KisView*) parent;
	}
}

ColorRange::~ColorRange()
{
}

void ColorRange::slotActivated()
{
	DlgColorRange * dlgColorRange = new DlgColorRange(m_view, "ColorRange");

	// Render layer to a QIMage -- keep in mind possibility of selection
	KisLayerSP layer = m_view -> currentImg() -> activeLayer();
	QImage img = layer -> convertToImage();

	// Scale QImage to fit in preview

	// Set original QImage in dialog
		
	if (dlgColorRange -> exec() == QDialog::Accepted) {
		// Retrieve changes made by dialog
		// Apply changes to layer (selection)
		// Iterate through the pixels of the layer
		
		// If the pixel matches the criteria from the dialog, set the pixel selected
	}
	delete dlgColorRange;
}

#include "colorrange.moc"

