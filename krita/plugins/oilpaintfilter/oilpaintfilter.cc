/*
 * rotateimage.cc -- Part of Krita
 *
 * Copyright (c) 2004 Michael Thaler
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
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kistile.h>
#include <kistilemgr.h>
#include <kis_iterators_quantum.h>
#include <kis_selection.h>

#include "oilpaintfilter.h"
#include "dlg_oilpaintfilter.h"

typedef KGenericFactory<OilPaintFilter> OilPaintFilterFactory;
K_EXPORT_COMPONENT_FACTORY( kritaoilpaintfilter, OilPaintFilterFactory( "krita" ) )

OilPaintFilter::OilPaintFilter(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
	setInstance(OilPaintFilterFactory::instance());

	(void) new KAction(i18n("&Oil Paint..."), 0, 0, this, SLOT(slotOilPaintFilter()), actionCollection(), "oilpaintfilter");
	
	if ( !parent->inherits("KisView") )
	{
		m_view = 0;
	} else {
		m_view = (KisView*) parent;
	}
}

OilPaintFilter::~OilPaintFilter()
{
	m_view = 0;
}

void OilPaintFilter::slotOilPaintFilter()
{
	KisImageSP image = m_view -> currentImg();

	if (!image) return;

	DlgOilPaintFilter * dlgOilPaintFilter = new DlgOilPaintFilter(m_view, "OilPaint");
	dlgOilPaintFilter -> setCaption("Oil Paint Filter");
	
	if (dlgOilPaintFilter -> exec() == QDialog::Accepted) {
                Q_UINT32 brushSize = dlgOilPaintFilter -> brushSize();
                Q_UINT32 smooth = dlgOilPaintFilter -> smooth();
                m_view -> oilPaintFilter(brushSize, smooth);
	}
	delete dlgOilPaintFilter;
}

#include "oilpaintfilter.moc"
