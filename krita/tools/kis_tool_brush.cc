/*
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
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
#include <qpainter.h>
#include <qpen.h>
#include <qcolor.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <koColor.h>

#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_doc.h"
#include "kis_view.h"
#include "kis_tool_brush.h"
#include "kis_layer.h"

KisToolBrush::KisToolBrush(KisView *view, KisDoc *doc) 
	: super(view, doc),
	  m_mode( HOVER )
{
	m_view = view;
	m_doc = doc;
}

KisToolBrush::~KisToolBrush()
{
}

void KisToolBrush::mouseRelease(QMouseEvent* /*e*/)
{
	m_mode = HOVER;
}

void KisToolBrush::mousePress(QMouseEvent *e)
{
	if (!m_brush) return;

	if (e->button() == QMouseEvent::LeftButton) {
		m_mode = PAINT;
	} else if (e->button() == QMouseEvent::MidButton) {
		m_mode = SMUDGE;
	} else if (e->button() == QMouseEvent::RightButton) {
		m_mode = ERASE;
	} else {
		m_mode = HOVER;
	}
}

void KisToolBrush::mouseMove(QMouseEvent *e)
{
	if (m_mode == PAINT) {
		paint(e->pos(), 128, 0, 0);
	} else if (m_mode == SMUDGE) {
		smudge(e->pos(), 128, 0, 0);
	} else if (m_mode == ERASE) {
		erase(e->pos(), 128, 0, 0);
	}
	
}


void KisToolBrush::tabletEvent(QTabletEvent *e)
{
	if (e->device() == QTabletEvent::Stylus) {
		paint(e->pos(), e->pressure(), e->xTilt(), e->yTilt());
	} else if (e->device() == QTabletEvent::Eraser) {
		smudge(e->pos(), e->pressure(), e->xTilt(), e->yTilt());
	}
}


void KisToolBrush::paint(const QPoint & pos, 
			 const Q_INT32 /*pressure*/,
			 const Q_INT32 /*xTilt*/,
			 const Q_INT32 /*yTilt*/)
{
#if 0
	kdDebug() << "paint: " << pos.x() << ", " << pos.y() << endl;
#endif
	KisImageSP currentImage = m_view -> currentImg();

	if (!currentImage) return;

	QImage mask;
	// Retrieve the mask of the brush used
	QImage img = m_brush->img();
	if (img.hasAlphaBuffer()) {
		mask = img.createAlphaMask();
	}
	else {
		mask = img.createHeuristicMask(true);
	}
#if 0
	kdDebug() << "mask depth: " << mask.depth() << endl;
#endif
	// Mess with the default mask according to pressure, tilt and whatnot


	// Create a temporary, transparent KisPaintDevice (see http://ww.levien.com/gimp/brush-arch.html)
	// Maybe create a special KisPaintDevice for this, and not use the generic KisLayer
	KisLayerSP tmpLayer = new KisLayer(img.width(), img.height(), currentImage->imgType(), "dab");

	// Position the brush mask inside the temporary buffer; this means computing alpha values
	// for the edges, to give it a 'soft' appearance

	// Put the pixels into the temporary KisPaintDevice with the current color
	// We will use this paintdevice as an image to composite with, so we can
	// use setPixel with impunity.
	// XXX: cache this, because it's always the same.
	for (int x = 0; x < img.width(); x++) {
		for (int y = 0; y < img.height(); y++) {
#if 0
			kdDebug() << "pixel: " << x << ", " << y << ", color: " 
				  << "R:" << qRed(img.pixel(x, y))
				  << " G:" << qGreen(img.pixel(x, y))
				  << " B:" << qBlue(img.pixel(x, y))
				  << " A:" << qAlpha(img.pixel(x, y))
				  << endl;
#endif
			// The brushes are mostly grayscale on a white background, although some do
			// have a colors. The alpha channel is seldom used, so we take the average
			// gray value of this pixel of the brush as the setting for the opacitiy. 
			// We need to invert it, because 255, 255, 255 is white, which is completely
			// transparent, but 255 corresponds to OPACITY_OPAQUE. 255 + 2.
			//
			// If the alpha value is not 255, or the r,g and b values are not the same,
			// we have a real coloured brush, and are knackered for the nonce.
			QRgb c = img.pixel(x,y);
			QUANTUM a = (255 - qRed(c) + 255 - qGreen(c) + 255 - qBlue(c)) / 3;
			tmpLayer->setPixel(x, y, m_color, a);
		}
	}
	// Blit the temporary KisPaintDevice onto the current layer

        KisPaintDeviceSP device = currentImage -> activeDevice();
        if (device) {
		KisPainter p( device );
		p.beginTransaction( "Brush" );
		p.bitBlt( pos.x(),  pos.y(),  COMPOSITE_OVER, tmpLayer.data());
		p.endTransaction();
		device->anchor();
	}
        currentImage->invalidate( pos.x(),  pos.y(),  tmpLayer->width(),  tmpLayer->height() );
	m_view->updateCanvas(pos.x(),  pos.y(),  tmpLayer->width(),  tmpLayer->height());
}

void KisToolBrush::smudge(const QPoint & /*pos*/, 
			  const Q_INT32 /*pressure*/,
			  const Q_INT32 /*xTilt*/,
			  const Q_INT32 /*yTilt*/)
{
	kdDebug() << "smudge" << endl;
}
	
void KisToolBrush::erase(const QPoint & /*pos*/, 
			 const Q_INT32 /*pressure*/,
			 const Q_INT32 /*xTilt*/,
			 const Q_INT32 /*yTilt*/)
{
	kdDebug() << "erase" << endl;
}

void KisToolBrush::setup()
{
	KToggleAction *toggle;
	toggle = new KToggleAction(i18n("&Brush"), "Brush", 0, this,
			SLOT(activateSelf()), m_view -> actionCollection(), "tool_brush");
	toggle -> setExclusiveGroup("tools");
}

