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
#include <kis_cubic_curve.h>
#include "kis_color_picker_utils.h"
#include <kis_paintop.h>


KisToolPaint::KisToolPaint(KoCanvasBase * canvas, const QCursor & cursor)
        : KisTool(canvas, cursor)
{
    m_optionWidgetLayout = 0;

    m_lbOpacity = 0;
    m_slOpacity = 0;

    m_opacity = OPACITY_OPAQUE_U8;
    m_compositeOp = 0;

    updateTabletPressureSamples();

    m_supportOutline = false;

    KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas);


    connect(kiscanvas->view()->resourceProvider(), SIGNAL(sigCompositeOpChanged(QString)), this, SLOT(slotSetCompositeMode(QString))); 
    connect(this, SIGNAL(sigFavoritePaletteCalled(const QPoint&)), kiscanvas, SIGNAL(favoritePaletteCalled(const QPoint&)));
    connect(this, SIGNAL(sigPaintingFinished()), kiscanvas->view()->resourceProvider(), SLOT(slotPainting()));
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
        default: //nothing
            break;
    }

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(resetCursorStyle()));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(updateTabletPressureSamples()));

}


void KisToolPaint::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisTool::activate(toolActivation, shapes);
    KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
    slotSetCompositeMode(kiscanvas->view()->resourceProvider()->currentCompositeOp());
    resetCursorStyle();
}


void KisToolPaint::paint(QPainter&, const KoViewConverter &)
{
}

void KisToolPaint::setMode(ToolMode mode)
{
    if(this->mode() == KisTool::PAINT_MODE &&
       mode != KisTool::PAINT_MODE) {

        // Let's add history information about recently used colors
        emit sigPaintingFinished();
    }

    KisTool::setMode(mode);
}

void KisToolPaint::mousePressEvent(KoPointerEvent *event)
{
    if(mode() == KisTool::HOVER_MODE &&
       (event->button() == Qt::LeftButton) &&
       event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier) &&
       !specialModifierActive()) {
        setMode(MIRROR_AXIS_SETUP_MODE);
        useCursor(KisCursor::crossCursor());
        canvas()->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxisCenter, convertToPixelCoord(event->point));
    }
    else if(mode() == KisTool::HOVER_MODE &&
       (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) &&
       event->modifiers() & Qt::ControlModifier &&
       !specialModifierActive()) {

        setMode(SECONDARY_PAINT_MODE);
        useCursor(KisCursor::pickerCursor());

        m_toForegroundColor = event->button() == Qt::LeftButton;
        pickColor(event->point, event->modifiers() & Qt::AltModifier,
                  m_toForegroundColor);
        event->accept();
    }
    else if(mode() == KisTool::HOVER_MODE &&
            event->button() == Qt::RightButton &&
            event->modifiers() == Qt::NoModifier &&
            !specialModifierActive()) {

        emit sigFavoritePaletteCalled(event->pos());
        event->accept();
    }
    else {
        KisTool::mousePressEvent(event);
    }
}

void KisToolPaint::mouseMoveEvent(KoPointerEvent *event)
{
    if(mode() == KisTool::SECONDARY_PAINT_MODE) {
        pickColor(event->point, event->modifiers() & Qt::AltModifier,
                  m_toForegroundColor);
        event->accept();
    }
    else if (mode() == KisTool::MIRROR_AXIS_SETUP_MODE){
        canvas()->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxisCenter, convertToPixelCoord(event->point));
    }
    else {
        KisTool::mouseMoveEvent(event);
    }
}

void KisToolPaint::mouseReleaseEvent(KoPointerEvent *event)
{
    if(mode() == KisTool::SECONDARY_PAINT_MODE || mode() == KisTool::MIRROR_AXIS_SETUP_MODE) {
        setMode(KisTool::HOVER_MODE);
        resetCursorStyle();
        event->accept();
    }
    else {
        KisTool::mouseReleaseEvent(event);
    }
}

void KisToolPaint::keyPressEvent(QKeyEvent *event)
{
    KisTool::keyPressEvent(event);
}

void KisToolPaint::keyReleaseEvent(QKeyEvent* event)
{
    KisTool::keyReleaseEvent(event);
}

void KisToolPaint::pickColor(const QPointF &documentPixel,
                             bool fromCurrentNode,
                             bool toForegroundColor)
{
    if(m_colorPickerDelayTimer.isActive()) {
        return;
    }
    else {
        m_colorPickerDelayTimer.setSingleShot(true);
        m_colorPickerDelayTimer.start(100);
    }

    int resource = toForegroundColor ?
        KoCanvasResource::ForegroundColor : KoCanvasResource::BackgroundColor;

    KisPaintDeviceSP device = fromCurrentNode ?
        currentNode()->paintDevice() : image()->projection();

    QPoint imagePoint = image()->documentToIntPixel(documentPixel);

    canvas()->resourceManager()->
        setResource(resource, KisToolUtils::pick(device, imagePoint));
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
    m_optionWidgetLayout->setColumnStretch(1, 1);

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
        if(isCanvasOpenGL()) {
            if (m_supportOutline) {
                useCursor(KisCursor::blankCursor());
            } else {
                useCursor(cursor());
            }
        } else {
            useCursor(KisCursor::arrowCursor());
        }
    }
#endif


}

void KisToolPaint::updateTabletPressureSamples()
{
    KisConfig cfg;
    KisCubicCurve curve;
    curve.fromString(cfg.pressureTabletCurve());
    m_pressureSamples = curve.floatTransfer(LEVEL_OF_PRESSURE_RESOLUTION + 1);
}

void KisToolPaint::setupPainter(KisPainter* painter)
{
    KisTool::setupPainter(painter);
    painter->setOpacity(m_opacity);
    painter->setCompositeOp(m_compositeOp);

    QPointF axisCenter = canvas()->resourceManager()->resource(KisCanvasResourceProvider::MirrorAxisCenter).toPointF();
    if (axisCenter.isNull()){
        axisCenter = QPointF(0.5 * image()->width(), 0.5 * image()->height());
    }
    bool mirrorMaskHorizontal = canvas()->resourceManager()->resource(KisCanvasResourceProvider::MirrorHorizontal).toBool();
    bool mirrorMaskVertical = canvas()->resourceManager()->resource(KisCanvasResourceProvider::MirrorVertical).toBool();
    painter->setMirrorInformation(axisCenter, mirrorMaskHorizontal, mirrorMaskVertical);
}

void KisToolPaint::setupPaintAction(KisRecordedPaintAction* action)
{
    KisTool::setupPaintAction(action);
    action->setOpacity(m_opacity / qreal(255.0));
    if (m_compositeOp) {
        action->setCompositeOp(m_compositeOp->id());
    }
}

KisToolPaint::NodePaintAbility KisToolPaint::nodePaintAbility()
{
    KisNodeSP node = currentNode();
    if (!node || node->systemLocked() || node->inherits("KisSelectionMask")) {
        return NONE;
    }
    if (node->inherits("KisShapeLayer")) {
        return VECTOR;
    }
    if (node->paintDevice()) {
        return PAINT;
    }
    return NONE;
}


#include "kis_tool_paint.moc"
