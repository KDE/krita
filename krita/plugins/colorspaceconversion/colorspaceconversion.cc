/*
 * colorspaceconversion.cc -- Part of Krita
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
#include <stdlib.h>

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcombobox.h>

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
#include <kis_paint_device.h>

#include "colorspaceconversion.h"
#include "dlg_colorspaceconversion.h"

typedef KGenericFactory<ColorspaceConversion> ColorspaceConversionFactory;
K_EXPORT_COMPONENT_FACTORY( kritacolorspaceconversion, ColorspaceConversionFactory( "krita" ) )


ColorspaceConversion::ColorspaceConversion(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{

	setInstance(ColorspaceConversionFactory::instance());

 	kdDebug() << "Colorspaceconversion plugin. Class: " 
 		  << className() 
 		  << ", Parent: " 
 		  << parent -> className()
 		  << "\n";

	(void) new KAction(i18n("&Convert image type..."), 0, 0, this, SLOT(slotImgColorspaceConversion()), actionCollection(), "imgcolorspaceconversion");
	(void) new KAction(i18n("&Convert layer type..."), 0, 0, this, SLOT(slotLayerColorspaceConversion()), actionCollection(), "layercolorspaceconversion");

	if ( !parent->inherits("KisView") )
	{
		m_view = 0;
	} else {
		m_view = (KisView*) parent;
	}
}

ColorspaceConversion::~ColorspaceConversion()
{
	m_view = 0;
}

void ColorspaceConversion::slotImgColorspaceConversion()
{
	KisImageSP image = m_view -> currentImg();

	if (!image) return;

	DlgColorspaceConversion * dlgColorspaceConversion = new DlgColorspaceConversion(m_view, "ColorspaceConversion");
	dlgColorspaceConversion -> setCaption(i18n("Convert all layers"));
	dlgColorspaceConversion -> m_page -> chkAlpha -> setChecked(image -> alpha());

	if (dlgColorspaceConversion -> exec() == QDialog::Accepted) {
		kdDebug() << "Going to convert image\n";
	}
	delete dlgColorspaceConversion;
}

void ColorspaceConversion::slotLayerColorspaceConversion()
{

	KisImageSP image = m_view -> currentImg();
	if (!image) return;

	KisPaintDeviceSP dev = image -> activeDevice();
	if (!dev) return;
	
	DlgColorspaceConversion * dlgColorspaceConversion = new DlgColorspaceConversion(m_view, "ColorspaceConversion");
	dlgColorspaceConversion -> setCaption(i18n("Convert current layer"));
	dlgColorspaceConversion -> m_page -> chkAlpha -> setChecked(dev -> alpha());

	if (dlgColorspaceConversion -> exec() == QDialog::Accepted) {
		kdDebug() << "Going to convert layer\n";
	}
	delete dlgColorspaceConversion;
}

#include "colorspaceconversion.moc"
