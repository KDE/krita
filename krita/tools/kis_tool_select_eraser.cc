/*
 *  kis_tool_select_brush.cc - part of Krita
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

#include "integerwidget.h"
#include "kis_brush.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_cmb_composite.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_move_event.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_tool_select_eraser.h"
#include "kis_types.h"
#include "kis_view.h"
#include "wdgselectionoptions.h"

KisToolSelectEraser::KisToolSelectEraser()
        : super(i18n("SelectEraser"))
{
	setName("tool_select_eraser");
	setCursor(KisCursor::eraserCursor());
}

KisToolSelectEraser::~KisToolSelectEraser()
{
}

void KisToolSelectEraser::initPaint(KisEvent */*e*/) 
{
	if (!m_currentImage || !m_currentImage -> activeDevice()) return;

	m_mode = PAINT;
	m_dragDist = 0;

	// Create painter
	KisLayerSP layer;
	if (m_currentImage && (layer = m_currentImage -> activeLayer())) {
		if (m_painter)
			delete m_painter;
		KisSelectionSP selection = layer -> selection();
		m_painter = new KisPainter(selection.data());
		m_painter -> beginTransaction(i18n("selectioneraser"));
		m_painter -> setPaintColor(KoColor::white()); // XXX: the mask color!
		m_painter -> setBrush(m_subject -> currentBrush());
		m_painter -> setOpacity(OPACITY_OPAQUE);
		m_painter -> setCompositeOp(COMPOSITE_OVER);

		// XXX: Yes, the selection eraser is a brush and the
		// selection brush is an eraser. That's because
		// transparent == selected in KisSelection until we
		// have a proper alpha colour model.
		KisPaintOp * op = KisPaintOpRegistry::singleton() -> paintOp("paintbrush", painter());
		painter() -> setPaintOp(op); // And now the painter owns the op and will destroy it.
	}
	// Set the cursor -- ideally. this should be a mask created from the brush,
	// now that X11 can handle colored cursors.
#if 0
	// Setting cursors has no effect until the tool is selected again; this
	// should be fixed.
	setCursor(KisCursor::eraserCursor());
#endif
}

void KisToolSelectEraser::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Eraser Select"),
					    "selecteraser", Qt::Key_B, this,
					    SLOT(activate()), collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

QWidget* KisToolSelectEraser::createOptionWidget(QWidget* parent)
{
	m_optWidget = new WdgSelectionOptions(parent);
	m_optWidget -> setCaption(i18n("Selection eraser"));
	return m_optWidget;
}

QWidget* KisToolSelectEraser::optionWidget()
{
	return m_optWidget;
}

#include "kis_tool_select_eraser.moc"

