/*
 *  kis_tool_curve.cc -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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


#include <qpainter.h>
#include <qlayout.h>
#include <qrect.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kis_cmb_composite.h"
#include "kis_colorspace.h"
#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_global.h"
#include "kis_image.h"
#include "kis_int_spinbox.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_point.h"
#include "kis_tool_controller.h"
#include "kis_tool_paint.h"

#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_canvas_subject.h"

#include "kis_curve_framework.h"
#include "kis_tool_curve_paint.h"

KisToolCurvePaint::KisToolCurvePaint(const QString& UIName)
    : super(UIName)
{
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

KisToolCurvePaint::~KisToolCurvePaint()
{

}

void KisToolCurvePaint::paintCurve()
{
    KisPaintDeviceSP device = m_currentImage->activeDevice ();
    if (!device) return;
    
    KisPainter painter (device);
    if (m_currentImage->undo()) painter.beginTransaction (i18n (m_transactionMessage.latin1()));

    painter.setPaintColor(m_subject->fgColor());
    painter.setBrush(m_subject->currentBrush());
    painter.setOpacity(m_opacity);
    painter.setCompositeOp(m_compositeOp);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_subject->currentPaintop(), m_subject->currentPaintopSettings(), &painter);
    painter.setPaintOp(op); // Painter takes ownership

// Call paintPoint
    KisCurve::iterator it = m_curve->begin();
    while (it != m_curve->end())
        it = paintPoint(painter,it);
// Finish

    device->setDirty( painter.dirtyRect() );
    notifyModified();

    if (m_currentImage->undo()) {
        m_currentImage->undoAdapter()->addCommand(painter.endTransaction());
    }

    draw();
}

KisCurve::iterator KisToolCurvePaint::paintPoint (KisPainter& painter, KisCurve::iterator point)
{
    KisCurve::iterator next = point; next+=1;
    switch ((*point).hint()) {
    case POINTHINT:
        painter.paintAt((*point++).point(), PRESSURE_DEFAULT, 0, 0);
        break;
    case LINEHINT:
        if (next != m_curve->end() && (*next).hint() <= LINEHINT)
            painter.paintLine((*point++).point(), PRESSURE_DEFAULT, 0, 0, (*next).point(), PRESSURE_DEFAULT, 0, 0);
        else
            painter.paintAt((*point++).point(), PRESSURE_DEFAULT, 0, 0);
        break;
    default:
        point += 1;
    }

    return point;
}

QWidget* KisToolCurvePaint::createOptionWidget(QWidget* parent)
{
    m_optionWidget = new QWidget(parent);
    m_optionWidget->setCaption(i18n(m_UIName.latin1()));

    m_lbOpacity = new QLabel(i18n("Opacity: "), m_optionWidget);
    m_slOpacity = new KisIntSpinbox( m_optionWidget, "int_m_optionwidget");
    m_slOpacity->setRange( 0, 100);
    m_slOpacity->setValue(m_opacity / OPACITY_OPAQUE * 100);
    connect(m_slOpacity, SIGNAL(valueChanged(int)), this, SLOT(slotSetOpacity(int)));

    m_lbComposite = new QLabel(i18n("Mode: "), m_optionWidget);
    m_cmbComposite = new KisCmbComposite(m_optionWidget);
    connect(m_cmbComposite, SIGNAL(activated(const KisCompositeOp&)), this, SLOT(slotSetCompositeMode(const KisCompositeOp&)));

    QVBoxLayout* verticalLayout = new QVBoxLayout(m_optionWidget);
    verticalLayout->setMargin(0);
    verticalLayout->setSpacing(3);
    
    m_optionWidgetLayout = new QGridLayout(verticalLayout, 2, 3, 6);

    m_optionWidgetLayout->addWidget(m_lbOpacity, 0, 0);
    m_optionWidgetLayout->addWidget(m_slOpacity, 0, 1);

    m_optionWidgetLayout->addWidget(m_lbComposite, 1, 0);
    m_optionWidgetLayout->addWidget(m_cmbComposite, 1, 1);

    verticalLayout->addItem(new QSpacerItem(0,0,QSizePolicy::Fixed,QSizePolicy::Expanding));

    if (!quickHelp().isEmpty()) {
        QPushButton* push = new QPushButton(SmallIconSet( "help" ), "", m_optionWidget);
        connect(push, SIGNAL(clicked()), this, SLOT(slotPopupQuickHelp()));

        QHBoxLayout* hLayout = new QHBoxLayout(m_optionWidget);
        hLayout->addWidget(push);
        hLayout->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Fixed));
        verticalLayout->addLayout(hLayout);
    }
    return m_optionWidget;
}

QWidget* KisToolCurvePaint::optionWidget()
{
    return m_optionWidget;
}

void KisToolCurvePaint::addOptionWidgetLayout(QLayout *layout)
{
    Q_ASSERT(m_optionWidget != 0);
    Q_ASSERT(m_optionWidgetLayout != 0);
    int rowCount = m_optionWidgetLayout->numRows();
    m_optionWidgetLayout->addMultiCellLayout(layout, rowCount, rowCount, 0, 1);
}

void KisToolCurvePaint::addOptionWidgetOption(QWidget *control, QWidget *label)
{
    Q_ASSERT(m_optionWidget != 0);
    Q_ASSERT(m_optionWidgetLayout != 0);
    if(label)
    {
        m_optionWidgetLayout->addWidget(label, m_optionWidgetLayout->numRows(), 0);
        m_optionWidgetLayout->addWidget(control, m_optionWidgetLayout->numRows()-1, 1);
    }
    else
        m_optionWidgetLayout->addMultiCellWidget(control, m_optionWidgetLayout->numRows(), m_optionWidgetLayout->numRows(), 0, 1);
}

void KisToolCurvePaint::slotSetOpacity(int opacityPerCent)
{
    m_opacity = opacityPerCent * OPACITY_OPAQUE / 100;
}

void KisToolCurvePaint::slotSetCompositeMode(const KisCompositeOp& compositeOp)
{
    m_compositeOp = compositeOp;
}

void KisToolCurvePaint::notifyModified() const
{
    if (m_subject && m_subject->currentImg()) {
        m_subject->currentImg()->setModified();
    }
}

void KisToolCurvePaint::updateCompositeOpComboBox()
{
    if (m_optionWidget && m_subject) {
        KisImageSP img = m_subject->currentImg();

        if (img) {
            KisPaintDeviceSP device = img->activeDevice();

            if (device) {
                KisCompositeOpList compositeOps = device->colorSpace()->userVisiblecompositeOps();
                m_cmbComposite->setCompositeOpList(compositeOps);

                if (compositeOps.find(m_compositeOp) == compositeOps.end()) {
                    m_compositeOp = COMPOSITE_OVER;
                }
                m_cmbComposite->setCurrentItem(m_compositeOp);
            }
        }
    }
}

void KisToolCurvePaint::activate()
{
    if (m_subject) {
        KisToolControllerInterface *controller = m_subject->toolController();

        if (controller)
            controller->setCurrentTool(this);

        updateCompositeOpComboBox();

        KisConfig cfg;
        m_paintOutline = (cfg.cursorStyle() == CURSOR_STYLE_OUTLINE);
    }
}

void KisToolCurvePaint::slotPopupQuickHelp() {
    QWhatsThis::display(quickHelp());
}

#include "kis_tool_curve_paint.moc"
