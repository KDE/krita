/*
 *  Copyright (c) 2003-2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_tool_paint.h"

#include <QWidget>
#include <QRect>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QWhatsThis>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QEvent>
#include <QVariant>
#include <QAction>
#include <QDebug>
#include <QPoint>

#include <kis_debug.h>
#include <kicon.h>
#include <klocale.h>
#include <kiconloader.h>

#include <KoShape.h>
#include <KoShapeManager.h>
#include <KoResourceManager.h>
#include <KoColorSpace.h>
#include <KoPointerEvent.h>
#include <KoColor.h>
#include <KoCanvasBase.h>

#include <opengl/kis_opengl.h>
#include <kis_types.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_view2.h>
#include <kis_canvas2.h>

#include "kis_config.h"
#include "kis_config_notifier.h"

#include "kis_cursor.h"
#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_slider_spin_box.h"
#include "kis_canvas_resource_provider.h"
#include <recorder/kis_recorded_paint_action.h>


KisToolPaint::KisToolPaint(KoCanvasBase * canvas, const QCursor & cursor)
        : KisTool(canvas, cursor)
{
    m_optionWidgetLayout = 0;

    m_lbOpacity = 0;
    m_slOpacity = 0;

    m_opacity = OPACITY_OPAQUE_U8;
    m_compositeOp = 0;
    
    m_supportOutline = false;
}


KisToolPaint::~KisToolPaint()
{
}

void KisToolPaint::resourceChanged(int key, const QVariant & v)
{
    KisTool::resourceChanged(key, v);

    switch(key){
        case(KisCanvasResourceProvider::CurrentKritaNode):
            slotSetCompositeMode(canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentCompositeOp).toString());
            break;
        case(KisCanvasResourceProvider::CurrentCompositeOp):
            slotSetCompositeMode(v.toString());
            break;
        default: //nothing
            break;
    }

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(resetCursorStyle()));

}


void KisToolPaint::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisTool::activate(toolActivation, shapes);
    resetCursorStyle();
}


void KisToolPaint::paint(QPainter&, const KoViewConverter &)
{
}


void KisToolPaint::mouseReleaseEvent(KoPointerEvent *e)
{
    if (e->button() == Qt::RightButton) {
        //CALLING POP UP PALETTE
        emit sigFavoritePaletteCalled(e->pos());

//        KoResourceManager * resourceProvider = 0;
//        if (canvas() && (resourceProvider = canvas()->resourceProvider())) {
//            QVariant fg = resourceProvider->resource(KoCanvasResource::ForegroundColor);
//            if (!fg.isValid()) return;
//            QVariant bg = resourceProvider->resource(KoCanvasResource::BackgroundColor);
//            if (!bg.isValid()) return;
//            resourceProvider->setResource(KoCanvasResource::ForegroundColor, bg);
//            resourceProvider->setResource(KoCanvasResource::BackgroundColor, fg);
//        }
    }
}


QWidget * KisToolPaint::createOptionWidget()
{

    QWidget * optionWidget = new QWidget();
    optionWidget->setObjectName(toolId());

    m_lbOpacity = new QLabel(i18n("Opacity: "), optionWidget);
    m_lbOpacity->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_slOpacity = new KisSliderSpinBox(optionWidget);
    m_slOpacity->setRange(0, 100);
    m_slOpacity->setValue(m_opacity / OPACITY_OPAQUE_U8 * 100);
    connect(m_slOpacity, SIGNAL(valueChanged(int)), this, SLOT(slotSetOpacity(int)));

    QVBoxLayout* verticalLayout = new QVBoxLayout(optionWidget);
    verticalLayout->setObjectName("KisToolPaint::OptionWidget::VerticalLayout");
    verticalLayout->setMargin(0);
    verticalLayout->setSpacing(1);

    m_optionWidgetLayout = new QGridLayout();
    verticalLayout->addLayout(m_optionWidgetLayout);
    m_optionWidgetLayout->setSpacing(1);
    m_optionWidgetLayout->setMargin(0);

    m_optionWidgetLayout->addWidget(m_lbOpacity, 1, 0);
    m_optionWidgetLayout->addWidget(m_slOpacity, 1, 1);

    verticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    if (!quickHelp().isEmpty()) {
        QPushButton* push = new QPushButton(KIcon("help-contents"), "", optionWidget);
        connect(push, SIGNAL(clicked()), this, SLOT(slotPopupQuickHelp()));

        QHBoxLayout* hLayout = new QHBoxLayout(optionWidget);
        hLayout->addWidget(push);
        hLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        verticalLayout->addLayout(hLayout);
    }

    return optionWidget;
}


void KisToolPaint::addOptionWidgetLayout(QLayout *layout)
{
    Q_ASSERT(m_optionWidgetLayout != 0);
    int rowCount = m_optionWidgetLayout->rowCount();
    m_optionWidgetLayout->addLayout(layout, rowCount, 0, 1, 2);
}


void KisToolPaint::addOptionWidgetOption(QWidget *control, QWidget *label)
{
    Q_ASSERT(m_optionWidgetLayout != 0);
    if (label) {
        if (QLabel *lbl = qobject_cast<QLabel*>(label)) {
            lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        }
        m_optionWidgetLayout->addWidget(label, m_optionWidgetLayout->rowCount(), 0);
        m_optionWidgetLayout->addWidget(control, m_optionWidgetLayout->rowCount() - 1, 1);
    } else {
        m_optionWidgetLayout->addWidget(control, m_optionWidgetLayout->rowCount(), 0, 1, 2);
    }
}


void KisToolPaint::slotSetOpacity(int opacityPerCent)
{
    m_opacity = (int)(qreal(opacityPerCent) * OPACITY_OPAQUE_U8 / 100);
}


void KisToolPaint::slotSetCompositeMode(const QString& compositeOp)
{
    Q_ASSERT(!compositeOp.isEmpty());
    if (currentNode()) {
        KisPaintDeviceSP device = currentNode()->paintDevice();

        if (device) {
            m_compositeOp = device->colorSpace()->compositeOp(compositeOp);
        }
    }
}

void KisToolPaint::slotPopupQuickHelp()
{
    QWhatsThis::showText(QCursor::pos(), quickHelp());
}


void KisToolPaint::resetCursorStyle()
{
    KisTool::resetCursorStyle();
    KisConfig cfg;
    if (cfg.cursorStyle() == CURSOR_STYLE_OUTLINE) {
        if (m_supportOutline) {
            // do not show cursor, tool will paint outline
            useCursor(KisCursor::blankCursor());
        } else {
            // if the tool does not support outline, use tool icon cursor
            useCursor(cursor());
        }
    }

#if defined(HAVE_OPENGL)
    // TODO: maybe m_support 3D outline would be cooler. So far just freehand tool support 3D_MODEL cursor style
    if (cfg.cursorStyle() == CURSOR_STYLE_3D_MODEL) {
        if (m_supportOutline) {
            useCursor(KisCursor::blankCursor());
        } else {
            useCursor(cursor());
        }
    }
#endif


}

void KisToolPaint::setupPaintAction(KisRecordedPaintAction* action)
{
    KisTool::setupPaintAction(action);
    action->setOpacity(m_opacity / 255.0);
    if (m_compositeOp) {
        action->setCompositeOp(m_compositeOp->id());
    }
}

#include "kis_tool_paint.moc"
