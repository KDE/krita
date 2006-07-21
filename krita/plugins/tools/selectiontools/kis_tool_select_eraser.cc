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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_brush.h"
#include "kis_layer.h"
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
#include "kis_selection_options.h"

KisToolSelectEraser::KisToolSelectEraser()
        : super(i18n("SelectEraser"))
{
    setName("tool_select_eraser");
    setCursor(KisCursor::load("tool_eraser_selection_cursor.png", 5, 5));
    m_optWidget = 0;
    m_paintOnSelection = true;
}

KisToolSelectEraser::~KisToolSelectEraser()
{
}

void KisToolSelectEraser::activate()
{
    super::activate();

    if (!m_optWidget)
        return;

    m_optWidget->slotActivated();
}

void KisToolSelectEraser::initPaint(KisEvent */*e*/) 
{
    if (!m_currentImage || !m_currentImage->activeDevice()) return;

    m_mode = PAINT;
    m_dragDist = 0;

    // Create painter
    KisPaintDeviceSP dev = m_currentImage->activeDevice();
    
    if (dev == 0) return;
    
    if (m_painter)
        delete m_painter;
    if(! dev->hasSelection())
    {
        dev->selection()->clear();
        dev->emitSelectionChanged();
    }
    KisSelectionSP selection = dev->selection();

    m_target = selection;
    m_painter = new KisPainter(selection.data());
    Q_CHECK_PTR(m_painter);
    m_painter->beginTransaction(i18n("Selection Eraser"));
    m_painter->setPaintColor(KisColor(Qt::white, selection->colorSpace()));
    m_painter->setBrush(m_subject->currentBrush());
    m_painter->setOpacity(OPACITY_OPAQUE);
    m_painter->setCompositeOp(COMPOSITE_ERASE);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("eraser", 0, painter());
    painter()->setPaintOp(op); // And now the painter owns the op and will destroy it.

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
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("Selection &Eraser"),
                        "tool_eraser_selection", "Ctrl+Shift+E", this,
                        SLOT(activate()), collection,
                        name());
        Q_CHECK_PTR(m_action);
        m_action->setToolTip(i18n("Erase parts of a selection"));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

QWidget* KisToolSelectEraser::createOptionWidget(QWidget* parent)
{
    m_optWidget = new KisSelectionOptions(parent, m_subject);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setCaption(i18n("Selection Eraser"));

    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

    return m_optWidget;
}

QWidget* KisToolSelectEraser::optionWidget()
{
    return m_optWidget;
}

#include "kis_tool_select_eraser.moc"

