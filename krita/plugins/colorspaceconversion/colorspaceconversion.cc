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
#include <qbuttongroup.h>

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
#include <kis_paint_device.h>
#include <kis_colorspace_registry.h>
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

	(void) new KAction(i18n("&Convert Image Type..."), 0, 0, this, SLOT(slotImgColorspaceConversion()), actionCollection(), "imgcolorspaceconversion");
	(void) new KAction(i18n("&Convert Layer Type..."), 0, 0, this, SLOT(slotLayerColorspaceConversion()), actionCollection(), "layercolorspaceconversion");

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

	dlgColorspaceConversion -> setCaption(i18n("Convert All Layers From ") + image -> colorStrategy() -> id().name());

	dlgColorspaceConversion -> fillCmbSrcProfile(image -> colorStrategy() -> id());

	if (image -> profile()) {
		dlgColorspaceConversion -> m_page -> cmbSourceProfile -> setCurrentText(image -> profile() -> productName());
	}

	if (dlgColorspaceConversion -> exec() == QDialog::Accepted) {
		kdDebug() << "Going to convert image\n";
		// XXX: Do the rest of the stuff
		KisID cspace = dlgColorspaceConversion -> m_page -> cmbColorSpaces -> currentItem();
		KisStrategyColorSpaceSP cs = KisColorSpaceRegistry::instance() -> get(cspace);

		image -> setProfile(image -> colorStrategy() -> getProfileByName(dlgColorspaceConversion -> m_page -> cmbSourceProfile -> currentText()));
		image -> convertTo(cs,
				   cs -> getProfileByName(dlgColorspaceConversion -> m_page -> cmbDestProfile -> currentText()),
				   dlgColorspaceConversion -> m_page -> grpIntent -> selectedId());
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
	dlgColorspaceConversion -> setCaption(i18n("Convert Current Layer From") + dev -> colorStrategy() -> id().name());
	dlgColorspaceConversion -> fillCmbSrcProfile(dev -> colorStrategy() -> id());

	KisProfileSP p = dev -> profile();
	if ( !p ) return;

	dlgColorspaceConversion -> m_page -> cmbSourceProfile -> setCurrentText(p -> productName());

	if (dlgColorspaceConversion -> exec() == QDialog::Accepted) {
		kdDebug() << "Going to convert layer\n";
		KisID cspace = dlgColorspaceConversion -> m_page -> cmbColorSpaces -> currentItem();
		KisStrategyColorSpaceSP cs = KisColorSpaceRegistry::instance() -> get(cspace);
		dev -> setProfile(dev -> colorStrategy() -> getProfileByName(dlgColorspaceConversion -> m_page -> cmbSourceProfile -> currentText()));
		dev -> convertTo(cs,
				   cs -> getProfileByName(dlgColorspaceConversion -> m_page -> cmbDestProfile -> currentText()),
				   dlgColorspaceConversion -> m_page -> grpIntent -> selectedId());

	}
	delete dlgColorspaceConversion;
}

#include "colorspaceconversion.moc"
