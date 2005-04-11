/*
 * rotateimage.cc -- Part of Krita
 *
 * Copyright (c) 2004 Michael Thaler
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
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_selection.h>

#include "rotateimage.h"
#include "dlg_rotateimage.h"

typedef KGenericFactory<RotateImage> RotateImageFactory;
K_EXPORT_COMPONENT_FACTORY( kritarotateimage, RotateImageFactory( "krita" ) )

// XXX: this plugin could also provide layer scaling/resizing
RotateImage::RotateImage(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
	setInstance(RotateImageFactory::instance());

// 	kdDebug() << "RotateImage plugin. Class: "
// 		  << className()
// 		  << ", Parent: "
// 		  << parent -> className()
// 		  << "\n";


	if ( !parent->inherits("KisView") )
	{
		m_view = 0;
	} else {
		m_view = (KisView*) parent;
		(void) new KAction(i18n("&Rotate Image..."), 0, 0, this, SLOT(slotRotateImage()), actionCollection(), "rotateimage");
		
		(void) new KAction(i18n("&Rotate Layer..."), 0, 0, this, SLOT(slotRotateLayer()), actionCollection(), "rotatelayer");
		
		(void)new KAction(i18n("Rotate &180"), 0, m_view, SLOT(rotateLayer180()), actionCollection(), "rotateLayer180");
		(void)new KAction(i18n("Rotate &270"), "rotate_ccw", 0, m_view, SLOT(rotateLayerLeft90()), actionCollection(), "rotateLayerLeft90");
		(void)new KAction(i18n("Rotate &90"), "rotate_cw", 0, m_view, SLOT(rotateLayerRight90()), actionCollection(), "rotateLayerRight90");
	}
}

RotateImage::~RotateImage()
{
	m_view = 0;
}

void RotateImage::slotRotateImage()
{
	KisImageSP image = m_view -> currentImg();

	if (!image) return;

	DlgRotateImage * dlgRotateImage = new DlgRotateImage(m_view, "RotateImage");
	dlgRotateImage -> setCaption(i18n("Rotate Image"));

        if (dlgRotateImage -> exec() == QDialog::Accepted) {
		Q_INT32 angle = dlgRotateImage -> angle();
                m_view -> rotateCurrentImage(angle);
	}
        delete dlgRotateImage;
}

void RotateImage::slotRotateLayer()
{
	KisImageSP image = m_view -> currentImg();

	if (!image) return;

	DlgRotateImage * dlgRotateImage = new DlgRotateImage(m_view, "RotateLayer");
	dlgRotateImage -> setCaption("Rotate Layer");

	if (dlgRotateImage -> exec() == QDialog::Accepted) {
                Q_INT32 angle = dlgRotateImage -> angle();
		m_view -> rotateLayer(angle);

	}
	delete dlgRotateImage;
}

#include "rotateimage.moc"
