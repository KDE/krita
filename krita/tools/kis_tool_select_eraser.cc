/*
 *  kis_tool_select_eraser.cc - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_view.h"
#include "kis_tool_select_eraser.h"
#include "kis_brush.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"


KisToolSelectEraser::KisToolSelectEraser()
        : super(),
          m_mode( HOVER ),
	  m_dragDist ( 0 )
{
	setName("tool_select_eraser");
	// XXX: create cursors in the shape of the brush -- very
	// important for this tool
	setCursor(KisCursor::selectCursor());

//         m_painter = 0;
	m_currentImage = 0;
	m_optWidget = 0;
}

KisToolSelectEraser::~KisToolSelectEraser()
{
}

void KisToolSelectEraser::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}

void KisToolSelectEraser::buttonPress(KisButtonPressEvent *e)
{
        if (!m_subject) return;

        if (!m_subject -> currentBrush()) return;

	if (!m_currentImage || !m_currentImage -> activeDevice()) return;

        if (e -> button() == QMouseEvent::LeftButton) {
//                 m_mode = PAINT;
//                 initPaint(e -> pos());
//                 m_painter -> penAt(e -> pos(), e -> pressure(), e -> xTilt(), e -> yTilt());
//                 // XXX: get the rect that should be notified
//                 m_currentImage -> notify( m_painter -> dirtyRect() );
         }
}

void KisToolSelectEraser::buttonRelease(KisButtonReleaseEvent* e)
{
	if (e -> button() == QMouseEvent::LeftButton && m_mode == PAINT) {
		endPaint();
        }
}

void KisToolSelectEraser::move(KisMoveEvent *e)
{
	if (m_mode == PAINT) {
		paintLine(m_dragStart, e -> pos(), e -> pressure(), e -> xTilt(), e -> yTilt());
	}
}

void KisToolSelectEraser::initPaint(const KisPoint & pos)
{

	if (!m_currentImage -> activeDevice()) return;
	m_dragStart = pos;
	m_dragDist = 0;

// 	// Create painter
// 	KisPaintDeviceSP device;
// 	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
// 		if (m_painter)
// 			delete m_painter;
// 		m_painter = new KisPainter( device );
// 		m_painter -> beginTransaction(i18n("pen"));
// 	}

// 	m_painter -> setPaintColor(m_subject -> fgColor());
// 	m_painter -> setEraser(m_subject -> currentEraser());
// 	m_painter -> setOpacity(m_opacity);
// 	m_painter -> setCompositeOp(m_compositeOp);

}

void KisToolSelectEraser::endPaint() 
{
	m_mode = HOVER;
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
		KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
// 		if (adapter && m_painter) {
// 			// If painting in mouse release, make sure painter
// 			// is destructed or end()ed
// 			adapter -> addCommand(m_painter->endTransaction());
// 		}
// 		delete m_painter;
// 		m_painter = 0;

	}
}

void KisToolSelectEraser::paintLine(const KisPoint & pos1,
				   const KisPoint & pos2,
				   const double pressure,
				   const double xtilt,
				   const double ytilt)
{
	if (!m_currentImage -> activeDevice()) return;

// 	m_dragDist = m_painter -> paintLine(PAINTOP_PEN, pos1, pos2, pressure, xtilt, ytilt, m_dragDist);
// 	m_currentImage -> notify( m_painter -> dirtyRect() );
	m_dragStart = pos2;
}


void KisToolSelectEraser::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool Eraser &Select"),
					    "selecteraser", 0, this,
					    SLOT(activate()), collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

QWidget* KisToolSelectEraser::createOptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	m_optWidget -> setCaption(i18n("Selecteraser"));
	
	return m_optWidget;
}

QWidget* KisToolSelectEraser::optionWidget()
{
	return m_optWidget;
}

#include "kis_tool_select_eraser.moc"

