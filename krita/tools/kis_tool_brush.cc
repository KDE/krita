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
#include "kis_alpha_mask.h"


KisToolBrush::KisToolBrush()
        : super(),
          m_mode( HOVER ),
	  m_hotSpotX ( 0 ),
	  m_hotSpotY ( 0 ),
	  m_brushWidth ( 0 ),
	  m_brushHeight ( 0 ),
	  m_spacing ( 0 )
{
        m_painter = 0;
	m_dab = 0;
}

KisToolBrush::~KisToolBrush()
{
}

void KisToolBrush::mousePress(QMouseEvent *e)
{
        if (!m_subject) return;

        if (!m_subject->currentBrush()) return;

        if (e->button() == QMouseEvent::LeftButton) {
		initPaint();
                paint(e->pos(), 128, 0, 0);
         }
}


void KisToolBrush::mouseRelease(QMouseEvent* e)
{
	if (e->button() == QMouseEvent::LeftButton && m_mode == PAINT) {
		endPaint();
        }
}

void KisToolBrush::mouseMove(QMouseEvent *e)
{
         if (m_mode == PAINT) {
                 paint(e->pos(), 128, 0, 0);
         }

}


void KisToolBrush::tabletEvent(QTabletEvent *e)
{
         if (e->device() == QTabletEvent::Stylus) {
                 paint(e->pos(), e->pressure(), e->xTilt(), e->yTilt());
         }
}


void KisToolBrush::initPaint() 
{
	m_mode = PAINT;
	KisImageSP currentImage = m_subject -> currentImg();
	KisPaintDeviceSP device;
	if (currentImage && (device = currentImage -> activeDevice())) {
		if (m_painter)
			delete m_painter;
		m_painter = new KisPainter( device );
		m_painter->beginTransaction("brush");
	}
	KisAlphaMask *mask = m_subject -> currentBrush() -> mask();

	m_dab = new KisLayer(mask -> width(), 
			     mask -> height(),
			     currentImage -> imgType(),
			     "dab");
        m_dab -> opacity(OPACITY_TRANSPARENT);
	for (int y = 0; y < mask -> height(); y++) {
		for (int x = 0; x < mask -> width(); x++) {
                        m_dab -> setPixel(x, y, m_subject -> fgColor(), mask -> alphaAt(x, y));
                }
        }
}

void KisToolBrush::endPaint() 
{
	m_mode = HOVER;
	KisImageSP currentImage = m_subject -> currentImg();
	KisPaintDeviceSP device;
	if (currentImage && (device = currentImage -> activeDevice())) {
		KisUndoAdapter *adapter = currentImage -> undoAdapter();
		if (adapter && m_painter) {
			// If painting in mouse release, make sure painter
			// is destructed or end()ed
			adapter -> addCommand(m_painter->endTransaction());
		}
		delete m_painter;
		m_painter = 0;
		m_dab = 0; // XXX: No need to delete m_dab because shared pointer?
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
        KisImageSP currentImage = m_subject -> currentImg();

        if (!currentImage) return;

        // Blit the temporary KisPaintDevice onto the current layer
        KisPaintDeviceSP device = currentImage -> activeDevice();
        if (device) {
                m_painter->bitBlt( pos.x(),  pos.y(),  COMPOSITE_NORMAL, m_dab.data() );
        }
        currentImage->invalidate( pos.x(),  pos.y(),
                                  m_dab -> width(),
                                  m_dab -> height() );
        currentImage -> notify(pos.x(),
                        pos.y(),
                        m_dab -> width(),
                        m_dab -> height());
}


void KisToolBrush::setup(KActionCollection *collection)
{
        KToggleAction *toggle;
        toggle = new KToggleAction(i18n("&Brush"), "Brush", 0, this,
                                   SLOT(activate()), collection,
                                   "tool_brush");
        toggle -> setExclusiveGroup("tools");
}

