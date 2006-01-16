/*
 *  Copyright (c) 2003 Boudewijn Rempt
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

#include <qwidget.h>
#include <qrect.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>

#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>

#include "kis_colorspace.h"
#include "kis_global.h"
#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_canvas_subject.h"
#include "kis_tool_controller.h"
#include "kis_tool_paint.h"
#include "kis_cmb_composite.h"
#include "kis_image.h"
#include "kis_paint_device_impl.h"

KisToolPaint::KisToolPaint(const QString& UIName)
    : super(UIName)
{
    m_subject = 0;

    m_UIName = UIName;

    m_optionWidget = 0;
    m_optionWidgetLayout = 0;

    m_lbOpacity = 0;
    m_slOpacity = 0;
    m_lbComposite= 0;
    m_cmbComposite = 0;

    m_opacity = OPACITY_OPAQUE;
    m_compositeOp = COMPOSITE_OVER;
}

KisToolPaint::~KisToolPaint()
{
}

void KisToolPaint::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    updateCompositeOpComboBox();
}

void KisToolPaint::paint(KisCanvasPainter&)
{
}

void KisToolPaint::paint(KisCanvasPainter&, const QRect&)
{
}

void KisToolPaint::clear()
{
}

void KisToolPaint::clear(const QRect&)
{
}

void KisToolPaint::enter(QEvent *)
{
}

void KisToolPaint::leave(QEvent *)
{
}

void KisToolPaint::buttonPress(KisButtonPressEvent *)
{
}

void KisToolPaint::move(KisMoveEvent *)
{
}

void KisToolPaint::buttonRelease(KisButtonReleaseEvent *)
{
}

void KisToolPaint::doubleClick(KisDoubleClickEvent *)
{
}

void KisToolPaint::keyPress(QKeyEvent *)
{
}

void KisToolPaint::keyRelease(QKeyEvent *)
{
}

QWidget* KisToolPaint::createOptionWidget(QWidget* parent)
{
    m_optionWidget = new QWidget(parent);
    m_optionWidget -> setCaption(m_UIName);

    m_lbOpacity = new QLabel(i18n("Opacity: "), m_optionWidget);
    m_slOpacity = new KIntNumInput( m_optionWidget, "int_m_optionwidget");
    m_slOpacity -> setRange( 0, 100);
    m_slOpacity -> setValue(m_opacity / OPACITY_OPAQUE * 100);
    m_slOpacity -> setSuffix("%");
    connect(m_slOpacity, SIGNAL(valueChanged(int)), this, SLOT(slotSetOpacity(int)));

    m_lbComposite = new QLabel(i18n("Mode: "), m_optionWidget);
    m_cmbComposite = new KisCmbComposite(m_optionWidget);
    connect(m_cmbComposite, SIGNAL(activated(const KisCompositeOp&)), this, SLOT(slotSetCompositeMode(const KisCompositeOp&)));

   if (!quickHelp().isEmpty()) {
        m_optionWidgetLayout = new QGridLayout(m_optionWidget, 5, 3, 0, 6);

        m_optionWidgetLayout -> addWidget(m_lbOpacity, 0, 0);
        m_optionWidgetLayout -> addWidget(m_slOpacity, 0, 1);

        m_optionWidgetLayout -> addWidget(m_lbComposite, 1, 0);
        m_optionWidgetLayout -> addMultiCellWidget(m_cmbComposite, 1, 1, 1, 2);

        m_optionWidgetLayout -> addItem(new QSpacerItem(0,0,QSizePolicy::Fixed,QSizePolicy::Expanding), 4, 0);

        // XXX make this a picture of a '?', like you see everywhere
        QPushButton* push = new QPushButton("?", m_optionWidget);
        connect(push, SIGNAL(clicked()), this, SLOT(slotPopupQuickHelp()));
        m_optionWidgetLayout -> addWidget(push, 0, 2);
    } else {
        m_optionWidgetLayout = new QGridLayout(m_optionWidget, 5, 2, 0, 6);

        m_optionWidgetLayout -> addWidget(m_lbOpacity, 0, 0);
        m_optionWidgetLayout -> addWidget(m_slOpacity, 0, 1);

        m_optionWidgetLayout -> addWidget(m_lbComposite, 1, 0);
        m_optionWidgetLayout -> addWidget(m_cmbComposite, 1, 1);

        m_optionWidgetLayout -> addItem(new QSpacerItem(0,0,QSizePolicy::Fixed,QSizePolicy::Expanding), 4, 0);
    }

    m_optionWidgetLayout -> setRowSpacing(3, 6);

    return m_optionWidget;
}

QWidget* KisToolPaint::optionWidget()
{
    return m_optionWidget;
}

void KisToolPaint::addOptionWidgetLayout(QLayout *layout)
{
    Q_ASSERT(m_optionWidget != 0);
    Q_ASSERT(m_optionWidgetLayout != 0);
    m_optionWidgetLayout -> addMultiCellLayout(layout, 3, 3, 0, 1);
}

void KisToolPaint::slotSetOpacity(int opacityPerCent)
{
    m_opacity = opacityPerCent * OPACITY_OPAQUE / 100;
}

void KisToolPaint::slotSetCompositeMode(const KisCompositeOp& compositeOp)
{
    m_compositeOp = compositeOp;
}

QCursor KisToolPaint::cursor()
{
    return m_cursor;
}

void KisToolPaint::setCursor(const QCursor& cursor)
{
    m_cursor = cursor;

    if (m_subject) {
        KisToolControllerInterface *controller = m_subject -> toolController();

        if (controller && controller -> currentTool() == this) {
            m_subject->canvasController()->setCanvasCursor(m_cursor);
        }
    }
}

void KisToolPaint::activate()
{
    if (m_subject) {
        KisToolControllerInterface *controller = m_subject -> toolController();

        if (controller)
            controller -> setCurrentTool(this);
            
        updateCompositeOpComboBox();

        KisConfig cfg;
        m_paintOutline = (cfg.cursorStyle() == CURSOR_STYLE_OUTLINE);
    }
}

void KisToolPaint::notifyModified() const
{
    if (m_subject && m_subject->currentImg()) {
        m_subject->currentImg()->setModified();
    }
}

void KisToolPaint::updateCompositeOpComboBox()
{
    if (m_optionWidget && m_subject) {
        KisImageSP img = m_subject -> currentImg();

        if (img) {
            KisPaintDeviceImplSP device = img -> activeDevice();

            if (device) {
                KisCompositeOpList compositeOps = device -> colorSpace() -> userVisiblecompositeOps();
                m_cmbComposite -> setCompositeOpList(compositeOps);

                if (compositeOps.find(m_compositeOp) == compositeOps.end()) {
                    m_compositeOp = COMPOSITE_OVER;
                }
                m_cmbComposite -> setCurrentItem(m_compositeOp);
            }
        }
    }
}

void KisToolPaint::slotPopupQuickHelp() {
    QWhatsThis::display(quickHelp());
}

#include "kis_tool_paint.moc"
