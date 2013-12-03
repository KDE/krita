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
#include <klocale.h>
#include <kactioncollection.h>
#include <kaction.h>

#include <KoIcon.h>
#include <KoShape.h>
#include <KoShapeManager.h>
#include <KoCanvasResourceManager.h>
#include <KoColorSpace.h>
#include <KoPointerEvent.h>
#include <KoColor.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>

#include <kis_types.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_view2.h>
#include <kis_canvas2.h>
#include <kis_cubic_curve.h>

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_cursor.h"
#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_slider_spin_box.h"
#include "kis_canvas_resource_provider.h"
#include <recorder/kis_recorded_paint_action.h>
#include "kis_tool_utils.h"
#include <kis_paintop.h>
#include <kis_paintop_preset.h>

KisToolPaint::KisToolPaint(KoCanvasBase * canvas, const QCursor & cursor)
    : KisTool(canvas, cursor)
{
    m_specialHoverModifier = false;
    m_optionWidgetLayout = 0;

    m_opacity = OPACITY_OPAQUE_U8;

    updateTabletPressureSamples();

    m_supportOutline = false;

    KActionCollection *collection = this->canvas()->canvasController()->actionCollection();

    if (!collection->action("increase_brush_size")) {
        KAction *increaseBrushSize = new KAction(i18n("Increase Brush Size"), collection);
        increaseBrushSize->setShortcut(Qt::Key_BracketRight);
        collection->addAction("increase_brush_size", increaseBrushSize);
    }

    if (!collection->action("decrease_brush_size")) {
        KAction *decreaseBrushSize = new KAction(i18n("Decrease Brush Size"), collection);
        decreaseBrushSize->setShortcut(Qt::Key_BracketLeft);
        collection->addAction("decrease_brush_size", decreaseBrushSize);
    }

    addAction("increase_brush_size", dynamic_cast<KAction*>(collection->action("increase_brush_size")));
    addAction("decrease_brush_size", dynamic_cast<KAction*>(collection->action("decrease_brush_size")));

    KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas);
    connect(this, SIGNAL(sigFavoritePaletteCalled(const QPoint&)), kiscanvas, SIGNAL(favoritePaletteCalled(const QPoint&)));
    connect(this, SIGNAL(sigPaintingFinished()), kiscanvas->view()->resourceProvider(), SLOT(slotPainting()));
}


KisToolPaint::~KisToolPaint()
{
}

int KisToolPaint::flags() const
{
    return KisTool::FLAG_USES_CUSTOM_COMPOSITEOP;
}

void KisToolPaint::canvasResourceChanged(int key, const QVariant& v)
{
    KisTool::canvasResourceChanged(key, v);

    switch(key){
    case(KisCanvasResourceProvider::Opacity):
        slotSetOpacity(v.toDouble());
        break;
    default: //nothing
        break;
    }

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(resetCursorStyle()), Qt::UniqueConnection);
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(updateTabletPressureSamples()), Qt::UniqueConnection);

}


void KisToolPaint::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisTool::activate(toolActivation, shapes);
    connect(actions().value("increase_brush_size"), SIGNAL(triggered()), SLOT(increaseBrushSize()), Qt::UniqueConnection);
    connect(actions().value("decrease_brush_size"), SIGNAL(triggered()), SLOT(decreaseBrushSize()), Qt::UniqueConnection);
}

void KisToolPaint::deactivate()
{
    disconnect(actions().value("increase_brush_size"), 0, this, 0);
    disconnect(actions().value("decrease_brush_size"), 0, this, 0);
    KisTool::deactivate();
}

QPainterPath KisToolPaint::tryFixBrushOutline(const QPainterPath &originalOutline)
{
    KisConfig cfg;
    if (cfg.cursorStyle() != CURSOR_STYLE_OUTLINE) return originalOutline;

    const qreal minThresholdSize = cfg.outlineSizeMinimum();

    /**
     * If the brush outline is bigger than the canvas itself (which
     * would make it invisible for a user in most of the cases) just
     * add a cross in the center of it
     */

    QSize widgetSize = canvas()->canvasWidget()->size();
    const int maxThresholdSum = widgetSize.width() + widgetSize.height();

    QPainterPath outline = originalOutline;
    QRectF boundingRect = outline.boundingRect();
    const qreal sum = boundingRect.width() + boundingRect.height();

    QPointF center = boundingRect.center();

    if (sum > maxThresholdSum) {
        const int hairOffset = 7;

        outline.moveTo(center.x(), center.y() - hairOffset);
        outline.lineTo(center.x(), center.y() + hairOffset);

        outline.moveTo(center.x() - hairOffset, center.y());
        outline.lineTo(center.x() + hairOffset, center.y());
    } else if (sum < minThresholdSize) {
        outline = QPainterPath();
        outline.addEllipse(center, 0.5 * minThresholdSize, 0.5 * minThresholdSize);
    }

    return outline;
}

void KisToolPaint::paint(QPainter &gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    QPainterPath path = tryFixBrushOutline(pixelToView(m_currentOutline));
    paintToolOutline(&gc, path);
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


void KisToolPaint::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (pickColor(event->point, action)) {
        setMode(SECONDARY_PAINT_MODE);
        requestUpdateOutline(event->point);
        useCursor(KisCursor::pickerCursor());
    } else {
        KisTool::beginAlternateAction(event, action);
    }

}

void KisToolPaint::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (!pickColor(event->point, action)) {
        KisTool::continueAlternateAction(event, action);
    }
}

void KisToolPaint::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (pickColor(event->point, action)) {
        setMode(KisTool::HOVER_MODE);
        resetCursorStyle();
        requestUpdateOutline(event->point);
    } else {
        KisTool::endAlternateAction(event, action);
    }
}

bool KisToolPaint::pickColor(const QPointF &documentPixel,
                             AlternateAction action)
{
    if (action != PickFgNode &&
        action != PickBgNode &&
        action != PickFgImage &&
        action != PickBgImage) {

        return false;
    }

    bool toForegroundColor = action == PickFgNode || action == PickFgImage;
    bool fromCurrentNode = action == PickFgNode || action == PickBgNode;

    int resource = toForegroundColor ?
        KoCanvasResourceManager::ForegroundColor : KoCanvasResourceManager::BackgroundColor;

    KisPaintDeviceSP device = fromCurrentNode ?
        currentNode()->paintDevice() : image()->projection();

    QPoint imagePoint = image()->documentToIntPixel(documentPixel);

    canvas()->resourceManager()->
        setResource(resource, KisToolUtils::pick(device, imagePoint));

    return true;
}

void KisToolPaint::mousePressEvent(KoPointerEvent *event)
{
    KisTool::mousePressEvent(event);
    if (mode() == KisTool::HOVER_MODE) {
        requestUpdateOutline(event->point);
    }
}

void KisToolPaint::mouseMoveEvent(KoPointerEvent *event)
{
    KisTool::mouseMoveEvent(event);
    if (mode() == KisTool::HOVER_MODE) {
        requestUpdateOutline(event->point);
    }
}

void KisToolPaint::mouseReleaseEvent(KoPointerEvent *event)
{
    KisTool::mouseReleaseEvent(event);
    if (mode() == KisTool::HOVER_MODE) {
        requestUpdateOutline(event->point);
    }
}

QWidget * KisToolPaint::createOptionWidget()
{

    QWidget * optionWidget = new QWidget();
    optionWidget->setObjectName(toolId());

    QVBoxLayout* verticalLayout = new QVBoxLayout(optionWidget);
    verticalLayout->setObjectName("KisToolPaint::OptionWidget::VerticalLayout");
    verticalLayout->setMargin(0);
    verticalLayout->setSpacing(1);

    m_optionWidgetLayout = new QGridLayout();
    m_optionWidgetLayout->setColumnStretch(1, 1);

    verticalLayout->addLayout(m_optionWidgetLayout);
    m_optionWidgetLayout->setSpacing(1);
    m_optionWidgetLayout->setMargin(0);

    QWidget *w = new QWidget();
    w->setObjectName("SpecialSpacer");

    verticalLayout->addWidget(w);

    if (!quickHelp().isEmpty()) {
        QPushButton* push = new QPushButton(koIcon("help-contents"), QString(), optionWidget);
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
    }
    else {
        m_optionWidgetLayout->addWidget(control, m_optionWidgetLayout->rowCount(), 0, 1, 2);
    }
}


void KisToolPaint::slotSetOpacity(qreal opacity)
{
    m_opacity = quint8(opacity * OPACITY_OPAQUE_U8);
}

const KoCompositeOp* KisToolPaint::compositeOp()
{
    if (currentNode()) {
        KisPaintDeviceSP device = currentNode()->paintDevice();
        if (device) {
            QString op = canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentCompositeOp).toString();
            return device->colorSpace()->compositeOp(op);
        }
    }
    return 0;
}

void KisToolPaint::slotPopupQuickHelp()
{
    QWhatsThis::showText(QCursor::pos(), quickHelp());
}

void KisToolPaint::updateTabletPressureSamples()
{
    KisConfig cfg;
    KisCubicCurve curve;
    curve.fromString(cfg.pressureTabletCurve());
    m_pressureSamples = curve.floatTransfer(LEVEL_OF_PRESSURE_RESOLUTION + 1);
}

void KisToolPaint::setupPaintAction(KisRecordedPaintAction* action)
{
    KisTool::setupPaintAction(action);
    action->setOpacity(m_opacity / qreal(255.0));
    const KoCompositeOp* op = compositeOp();
    if (op) {
        action->setCompositeOp(op->id());
    }
}

KisToolPaint::NodePaintAbility KisToolPaint::nodePaintAbility()
{
    KisNodeSP node = currentNode();
    if (!node || node->systemLocked()) {
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

void KisToolPaint::increaseBrushSize()
{
    int paintopSize = currentPaintOpPreset()->settings()->paintOpSize().width();
    int increment = 1;
    if (paintopSize > 100) {
        increment = 30;
    } else if (paintopSize > 10){
        increment = 10;
    }
    currentPaintOpPreset()->settings()->changePaintOpSize(increment, 0);
    requestUpdateOutline(m_outlineDocPoint);
}

void KisToolPaint::decreaseBrushSize()
{
    int paintopSize = currentPaintOpPreset()->settings()->paintOpSize().width();
    int decrement = -1;
    if (paintopSize > 100) {
        decrement = -30;
    } else if (paintopSize > 20){
        decrement = -10;
    }
    currentPaintOpPreset()->settings()->changePaintOpSize(decrement, 0);
    requestUpdateOutline(m_outlineDocPoint);
}

void KisToolPaint::requestUpdateOutline(const QPointF &outlineDocPoint)
{
    if (!m_supportOutline) return;

    KisConfig cfg;
    KisPaintOpSettings::OutlineMode outlineMode;
    outlineMode = KisPaintOpSettings::CursorIsNotOutline;

    if (mode() == KisTool::GESTURE_MODE ||
        ((cfg.cursorStyle() == CURSOR_STYLE_OUTLINE || cfg.cursorStyle() == CURSOR_STYLE_OUTLINE_CENTER_DOT || cfg.cursorStyle() == CURSOR_STYLE_OUTLINE_CENTER_CROSS )&&
         ((mode() == HOVER_MODE) ||
          (mode() == PAINT_MODE && cfg.showOutlineWhilePainting())))) {

        outlineMode = KisPaintOpSettings::CursorIsOutline;
    }

    m_outlineDocPoint = outlineDocPoint;
    m_currentOutline = getOutlinePath(m_outlineDocPoint, outlineMode);

    QRectF outlinePixelRect = m_currentOutline.boundingRect();
    QRectF outlineDocRect = currentImage()->pixelToDocument(outlinePixelRect);

    // This adjusted call is needed as we paint with a 3 pixel wide brush and the pen is outside the bounds of the path
    // Pen uses view coordinates so we have to zoom the document value to match 2 pixel in view coordiates
    // See BUG 275829
    qreal zoomX;
    qreal zoomY;
    canvas()->viewConverter()->zoom(&zoomX, &zoomY);
    qreal xoffset = 2.0/zoomX;
    qreal yoffset = 2.0/zoomY;
    QRectF newOutlineRect = outlineDocRect.adjusted(-xoffset,-yoffset,xoffset,yoffset);

    if (!m_oldOutlineRect.isEmpty()) {
        canvas()->updateCanvas(m_oldOutlineRect);
    }

    if (!newOutlineRect.isEmpty()) {
        canvas()->updateCanvas(newOutlineRect);
    }

    m_oldOutlineRect = newOutlineRect;
}

QPainterPath KisToolPaint::getOutlinePath(const QPointF &documentPos,
                                          KisPaintOpSettings::OutlineMode outlineMode)
{
    QPointF imagePos = currentImage()->documentToPixel(documentPos);
    QPainterPath path = currentPaintOpPreset()->settings()->
        brushOutline(KisPaintInformation(imagePos), outlineMode);

    return path;
}

#include "kis_tool_paint.moc"
