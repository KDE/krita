/*
 *  Copyright (c) 2003-2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2015 Moritz Molch <kde@moritzmolch.de>
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

#include <algorithm>

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
#include <kis_debug.h>
#include <QPoint>

#include <klocalizedstring.h>
#include <kactioncollection.h>

#include <kis_icon.h>
#include <KoShape.h>
#include <KoCanvasResourceProvider.h>
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
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <kis_cubic_curve.h>
#include "kis_display_color_converter.h"
#include <KisDocument.h>
#include <KisReferenceImagesLayer.h>

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_cursor.h"
#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_slider_spin_box.h"
#include "kis_canvas_resource_provider.h"
#include "kis_tool_utils.h"
#include <brushengine/kis_paintop.h>
#include <brushengine/kis_paintop_preset.h>
#include <kis_action_manager.h>
#include <kis_action.h>
#include "strokes/kis_color_picker_stroke_strategy.h"


KisToolPaint::KisToolPaint(KoCanvasBase *canvas, const QCursor &cursor)
    : KisTool(canvas, cursor),
      m_showColorPreview(false),
      m_colorPreviewShowComparePlate(false),
      m_colorPickerDelayTimer(),
      m_isOutlineEnabled(true)
{
    m_specialHoverModifier = false;
    m_optionsWidgetLayout = 0;

    m_opacity = OPACITY_OPAQUE_U8;

    m_supportOutline = false;

    {
        int maxSize = KisConfig(true).readEntry("maximumBrushSize", 1000);

        int brushSize = 1;
        do {
            m_standardBrushSizes.push_back(brushSize);
            int increment = qMax(1, int(std::ceil(qreal(brushSize) / 15)));
            brushSize += increment;
        } while (brushSize < maxSize);

        m_standardBrushSizes.push_back(maxSize);
    }

    KisCanvas2 *kiscanvas = dynamic_cast<KisCanvas2*>(canvas);

    connect(this, SIGNAL(sigPaintingFinished()), kiscanvas->viewManager()->canvasResourceProvider(), SLOT(slotPainting()));

    m_colorPickerDelayTimer.setSingleShot(true);
    connect(&m_colorPickerDelayTimer, SIGNAL(timeout()), this, SLOT(activatePickColorDelayed()));

    using namespace std::placeholders; // For _1 placeholder
    std::function<void(PickingJob)> callback =
        std::bind(&KisToolPaint::addPickerJob, this, _1);
    m_colorPickingCompressor.reset(
        new PickingCompressor(100, callback, KisSignalCompressor::FIRST_ACTIVE));
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

    switch(key) {
    case(KisCanvasResourceProvider::Opacity):
        setOpacity(v.toDouble());
        break;
    default: //nothing
        break;
    }

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(resetCursorStyle()), Qt::UniqueConnection);

}


void KisToolPaint::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    if (currentPaintOpPreset()) {
        QString formattedBrushName = currentPaintOpPreset()->name().replace("_", " ");
        emit statusTextChanged(formattedBrushName);
    }

    KisTool::activate(toolActivation, shapes);
    if (flags() & KisTool::FLAG_USES_CUSTOM_SIZE) {
        connect(action("increase_brush_size"), SIGNAL(triggered()), SLOT(increaseBrushSize()), Qt::UniqueConnection);
        connect(action("decrease_brush_size"), SIGNAL(triggered()), SLOT(decreaseBrushSize()), Qt::UniqueConnection);
    }

    KisCanvasResourceProvider *provider = qobject_cast<KisCanvas2*>(canvas())->viewManager()->canvasResourceProvider();
    m_oldOpacity = provider->opacity();
    provider->setOpacity(m_localOpacity);
}

void KisToolPaint::deactivate()
{
    if (flags() & KisTool::FLAG_USES_CUSTOM_SIZE) {
        disconnect(action("increase_brush_size"), 0, this, 0);
        disconnect(action("decrease_brush_size"), 0, this, 0);
    }

    KisCanvasResourceProvider *provider = qobject_cast<KisCanvas2*>(canvas())->viewManager()->canvasResourceProvider();
    m_localOpacity = provider->opacity();
    provider->setOpacity(m_oldOpacity);

    KisTool::deactivate();
}

QPainterPath KisToolPaint::tryFixBrushOutline(const QPainterPath &originalOutline)
{
    KisConfig cfg(true);
    if (cfg.newOutlineStyle() == OUTLINE_NONE) return originalOutline;

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
    } else if (sum < minThresholdSize && !outline.isEmpty()) {
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

    if (m_showColorPreview) {
        QRectF viewRect = converter.documentToView(m_oldColorPreviewRect);
        gc.fillRect(viewRect, m_colorPreviewCurrentColor);

        if (m_colorPreviewShowComparePlate) {
            QRectF baseColorRect = viewRect.translated(viewRect.width(), 0);
            gc.fillRect(baseColorRect, m_colorPreviewBaseColor);
        }
    }
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

void KisToolPaint::activatePickColor(AlternateAction action)
{
    m_showColorPreview = true;

    requestUpdateOutline(m_outlineDocPoint, 0);

    int resource = colorPreviewResourceId(action);
    KoColor color = canvas()->resourceManager()->koColorResource(resource);

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);

    m_colorPreviewCurrentColor = kisCanvas->displayColorConverter()->toQColor(color);

    if (!m_colorPreviewBaseColor.isValid()) {
        m_colorPreviewBaseColor = m_colorPreviewCurrentColor;
    }
}

void KisToolPaint::deactivatePickColor(AlternateAction action)
{
    Q_UNUSED(action);

    m_showColorPreview = false;
    m_oldColorPreviewRect = QRect();
    m_oldColorPreviewUpdateRect = QRect();
    m_colorPreviewCurrentColor = QColor();
}

void KisToolPaint::pickColorWasOverridden()
{
    m_colorPreviewShowComparePlate = false;
    m_colorPreviewBaseColor = QColor();
}

void KisToolPaint::activateAlternateAction(AlternateAction action)
{
    switch (action) {
    case PickFgNode:
        Q_FALLTHROUGH();
    case PickBgNode:
        Q_FALLTHROUGH();
    case PickFgImage:
        Q_FALLTHROUGH();
    case PickBgImage:
        delayedAction = action;
        m_colorPickerDelayTimer.start(100);
        Q_FALLTHROUGH();
    default:
        pickColorWasOverridden();
        KisTool::activateAlternateAction(action);
    };
}

void KisToolPaint::activatePickColorDelayed()
{
    switch (delayedAction) {
        case PickFgNode:
        useCursor(KisCursor::pickerLayerForegroundCursor());
        activatePickColor(delayedAction);
        break;
    case PickBgNode:
        useCursor(KisCursor::pickerLayerBackgroundCursor());
        activatePickColor(delayedAction);
        break;
    case PickFgImage:
        useCursor(KisCursor::pickerImageForegroundCursor());
        activatePickColor(delayedAction);
        break;
    case PickBgImage:
        useCursor(KisCursor::pickerImageBackgroundCursor());
        activatePickColor(delayedAction);
        break;
    default:
        break;
    };

    repaintDecorations();
}

bool KisToolPaint::isPickingAction(AlternateAction action) {
    return action == PickFgNode ||
        action == PickBgNode ||
        action == PickFgImage ||
        action == PickBgImage;
}

void KisToolPaint::deactivateAlternateAction(AlternateAction action)
{
    if (!isPickingAction(action)) {
        KisTool::deactivateAlternateAction(action);
        return;
    }

    delayedAction = KisTool::NONE;
    m_colorPickerDelayTimer.stop();

    resetCursorStyle();
    deactivatePickColor(action);
}

void KisToolPaint::addPickerJob(const PickingJob &pickingJob)
{
    /**
     * The actual picking is delayed by a compressor, so we can get this
     * event when the stroke is already closed
     */
    if (!m_pickerStrokeId) return;

    KIS_ASSERT_RECOVER_RETURN(isPickingAction(pickingJob.action));

    const QPoint imagePoint = image()->documentToImagePixelFloored(pickingJob.documentPixel);
    const bool fromCurrentNode = pickingJob.action == PickFgNode || pickingJob.action == PickBgNode;
    m_pickingResource = colorPreviewResourceId(pickingJob.action);

    if (!fromCurrentNode) {
        auto *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
        KIS_SAFE_ASSERT_RECOVER_RETURN(kisCanvas);
        KisSharedPtr<KisReferenceImagesLayer> referencesLayer = kisCanvas->imageView()->document()->referenceImagesLayer();
        if (referencesLayer && kisCanvas->referenceImagesDecoration()->visible()) {
            QColor color = referencesLayer->getPixel(imagePoint);
            if (color.isValid() && color.alpha() != 0) {
                slotColorPickingFinished(KoColor(color, image()->colorSpace()));
                return;
            }
        }
    }

    KisPaintDeviceSP device = fromCurrentNode ?
        currentNode()->colorPickSourceDevice() : image()->projection();

    // Used for color picker blending.
    KoColor currentColor = canvas()->resourceManager()->foregroundColor();
    if( pickingJob.action == PickBgNode || pickingJob.action == PickBgImage ){
        currentColor = canvas()->resourceManager()->backgroundColor();
    }

    image()->addJob(m_pickerStrokeId,
                    new KisColorPickerStrokeStrategy::Data(device, imagePoint, currentColor));
}

void KisToolPaint::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (isPickingAction(action)) {
        KIS_ASSERT_RECOVER_RETURN(!m_pickerStrokeId);
        setMode(SECONDARY_PAINT_MODE);

        KisColorPickerStrokeStrategy *strategy = new KisColorPickerStrokeStrategy();
        connect(strategy, &KisColorPickerStrokeStrategy::sigColorUpdated,
                this, &KisToolPaint::slotColorPickingFinished);

        m_pickerStrokeId = image()->startStroke(strategy);
        m_colorPickingCompressor->start(PickingJob(event->point, action));
        requestUpdateOutline(event->point, event);
    } else {
        KisTool::beginAlternateAction(event, action);
    }
}

void KisToolPaint::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (isPickingAction(action)) {
        KIS_ASSERT_RECOVER_RETURN(m_pickerStrokeId);
        m_colorPickingCompressor->start(PickingJob(event->point, action));
        requestUpdateOutline(event->point, event);
    } else {
        KisTool::continueAlternateAction(event, action);
    }
}

void KisToolPaint::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (isPickingAction(action)) {
        KIS_ASSERT_RECOVER_RETURN(m_pickerStrokeId);
        image()->endStroke(m_pickerStrokeId);
        m_pickerStrokeId.clear();
        requestUpdateOutline(event->point, event);
        setMode(HOVER_MODE);
    } else {
        KisTool::endAlternateAction(event, action);
    }
}

int KisToolPaint::colorPreviewResourceId(AlternateAction action)
{
    bool toForegroundColor = action == PickFgNode || action == PickFgImage;
    int resource = toForegroundColor ?
        KoCanvasResourceProvider::ForegroundColor : KoCanvasResourceProvider::BackgroundColor;

    return resource;
}

void KisToolPaint::slotColorPickingFinished(const KoColor &color)
{
    canvas()->resourceManager()->setResource(m_pickingResource, color);

    if (!m_showColorPreview) return;

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    QColor previewColor = kisCanvas->displayColorConverter()->toQColor(color);

    m_colorPreviewShowComparePlate = true;
    m_colorPreviewCurrentColor = previewColor;

    requestUpdateOutline(m_outlineDocPoint, 0);
}

void KisToolPaint::mousePressEvent(KoPointerEvent *event)
{
    KisTool::mousePressEvent(event);
    if (mode() == KisTool::HOVER_MODE) {
        requestUpdateOutline(event->point, event);
    }
}

void KisToolPaint::mouseMoveEvent(KoPointerEvent *event)
{
    KisTool::mouseMoveEvent(event);
    if (mode() == KisTool::HOVER_MODE) {
        requestUpdateOutline(event->point, event);
    }
}

void KisToolPaint::mouseReleaseEvent(KoPointerEvent *event)
{
    KisTool::mouseReleaseEvent(event);
    if (mode() == KisTool::HOVER_MODE) {
        requestUpdateOutline(event->point, event);
    }
}

QWidget * KisToolPaint::createOptionWidget()
{
    QWidget *optionWidget = new QWidget();
    optionWidget->setObjectName(toolId());

    QVBoxLayout *verticalLayout = new QVBoxLayout(optionWidget);
    verticalLayout->setObjectName("KisToolPaint::OptionWidget::VerticalLayout");
    verticalLayout->setContentsMargins(0,0,0,0);
    verticalLayout->setSpacing(5);

    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(optionWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    verticalLayout->addWidget(specialSpacer);
    verticalLayout->addWidget(specialSpacer);

    m_optionsWidgetLayout = new QGridLayout();
    m_optionsWidgetLayout->setColumnStretch(1, 1);
    verticalLayout->addLayout(m_optionsWidgetLayout);
    m_optionsWidgetLayout->setContentsMargins(0,0,0,0);
    m_optionsWidgetLayout->setSpacing(5);

    if (!quickHelp().isEmpty()) {
        QPushButton *push = new QPushButton(KisIconUtils::loadIcon("help-contents"), QString(), optionWidget);
        connect(push, SIGNAL(clicked()), this, SLOT(slotPopupQuickHelp()));
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(push);
        hLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        verticalLayout->addLayout(hLayout);
    }

    return optionWidget;
}

QWidget* findLabelWidget(QGridLayout *layout, QWidget *control)
{
    QWidget *result = 0;

    int index = layout->indexOf(control);

    int row, col, rowSpan, colSpan;
    layout->getItemPosition(index, &row, &col, &rowSpan, &colSpan);

    if (col > 0) {
        QLayoutItem *item = layout->itemAtPosition(row, col - 1);

        if (item) {
            result = item->widget();
        }
    } else {
        QLayoutItem *item = layout->itemAtPosition(row, col + 1);
        if (item) {
            result = item->widget();
        }
    }

    return result;
}

void KisToolPaint::showControl(QWidget *control, bool value)
{
    control->setVisible(value);
    QWidget *label = findLabelWidget(m_optionsWidgetLayout, control);
    if (label) {
        label->setVisible(value);
    }
}

void KisToolPaint::enableControl(QWidget *control, bool value)
{
    control->setEnabled(value);
    QWidget *label = findLabelWidget(m_optionsWidgetLayout, control);
    if (label) {
        label->setEnabled(value);
    }
}

void KisToolPaint::addOptionWidgetLayout(QLayout *layout)
{
    Q_ASSERT(m_optionsWidgetLayout != 0);
    int rowCount = m_optionsWidgetLayout->rowCount();
    m_optionsWidgetLayout->addLayout(layout, rowCount, 0, 1, 2);
}


void KisToolPaint::addOptionWidgetOption(QWidget *control, QWidget *label)
{
    Q_ASSERT(m_optionsWidgetLayout != 0);
    if (label) {
        m_optionsWidgetLayout->addWidget(label, m_optionsWidgetLayout->rowCount(), 0);
        m_optionsWidgetLayout->addWidget(control, m_optionsWidgetLayout->rowCount() - 1, 1);
    }
    else {
        m_optionsWidgetLayout->addWidget(control, m_optionsWidgetLayout->rowCount(), 0, 1, 2);
    }
}


void KisToolPaint::setOpacity(qreal opacity)
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

KisToolPaint::NodePaintAbility KisToolPaint::nodePaintAbility()
{
    KisNodeSP node = currentNode();
    if (!node) {
        return NONE;
    }
    if (node->inherits("KisShapeLayer")) {
        return VECTOR;
    }
    if (node->inherits("KisCloneLayer")) {
        return CLONE;
    }
    if (node->paintDevice()) {
        return PAINT;
    }
    return NONE;
}

void KisToolPaint::activatePrimaryAction()
{
    pickColorWasOverridden();
    setOutlineEnabled(true);
    KisTool::activatePrimaryAction();
}

void KisToolPaint::deactivatePrimaryAction()
{
    setOutlineEnabled(false);
    KisTool::deactivatePrimaryAction();
}

bool KisToolPaint::isOutlineEnabled() const
{
    return m_isOutlineEnabled;
}

void KisToolPaint::setOutlineEnabled(bool value)
{
    m_isOutlineEnabled = value;
    requestUpdateOutline(m_outlineDocPoint, 0);
}

void KisToolPaint::increaseBrushSize()
{
    qreal paintopSize = currentPaintOpPreset()->settings()->paintOpSize();

    std::vector<int>::iterator result =
        std::upper_bound(m_standardBrushSizes.begin(),
                         m_standardBrushSizes.end(),
                         qRound(paintopSize));

    int newValue = result != m_standardBrushSizes.end() ? *result : m_standardBrushSizes.back();

    currentPaintOpPreset()->settings()->setPaintOpSize(newValue);
    requestUpdateOutline(m_outlineDocPoint, 0);
}

void KisToolPaint::decreaseBrushSize()
{
    qreal paintopSize = currentPaintOpPreset()->settings()->paintOpSize();

    std::vector<int>::reverse_iterator result =
        std::upper_bound(m_standardBrushSizes.rbegin(),
                         m_standardBrushSizes.rend(),
                         (int)paintopSize,
                         std::greater<int>());

    int newValue = result != m_standardBrushSizes.rend() ? *result : m_standardBrushSizes.front();

    currentPaintOpPreset()->settings()->setPaintOpSize(newValue);
    requestUpdateOutline(m_outlineDocPoint, 0);
}

QRectF KisToolPaint::colorPreviewDocRect(const QPointF &outlineDocPoint)
{
    if (!m_showColorPreview) return QRect();

    KisConfig cfg(true);

    const QRectF colorPreviewViewRect = cfg.colorPreviewRect();
    const QRectF colorPreviewDocumentRect = canvas()->viewConverter()->viewToDocument(colorPreviewViewRect);

    return colorPreviewDocumentRect.translated(outlineDocPoint);
}

void KisToolPaint::requestUpdateOutline(const QPointF &outlineDocPoint, const KoPointerEvent *event)
{
    if (!m_supportOutline) return;

    KisConfig cfg(true);
    KisPaintOpSettings::OutlineMode outlineMode;

    if (isOutlineEnabled() &&
        (mode() == KisTool::GESTURE_MODE ||
         ((cfg.newOutlineStyle() == OUTLINE_FULL ||
           cfg.newOutlineStyle() == OUTLINE_CIRCLE ||
           cfg.newOutlineStyle() == OUTLINE_TILT) &&
          ((mode() == HOVER_MODE) ||
           (mode() == PAINT_MODE && cfg.showOutlineWhilePainting()))))) { // lisp forever!

        outlineMode.isVisible = true;

        if (cfg.newOutlineStyle() == OUTLINE_CIRCLE) {
            outlineMode.forceCircle = true;
        } else if(cfg.newOutlineStyle() == OUTLINE_TILT) {
            outlineMode.forceCircle = true;
            outlineMode.showTiltDecoration = true;
        } else {
            // noop
        }
    }

    outlineMode.forceFullSize = cfg.forceAlwaysFullSizedOutline();

    m_outlineDocPoint = outlineDocPoint;
    m_currentOutline = getOutlinePath(m_outlineDocPoint, event, outlineMode);

    QRectF outlinePixelRect = m_currentOutline.boundingRect();
    QRectF outlineDocRect = currentImage()->pixelToDocument(outlinePixelRect);

    // This adjusted call is needed as we paint with a 3 pixel wide brush and the pen is outside the bounds of the path
    // Pen uses view coordinates so we have to zoom the document value to match 2 pixel in view coordinates
    // See BUG 275829
    qreal zoomX;
    qreal zoomY;
    canvas()->viewConverter()->zoom(&zoomX, &zoomY);
    qreal xoffset = 2.0/zoomX;
    qreal yoffset = 2.0/zoomY;

    if (!outlineDocRect.isEmpty()) {
        outlineDocRect.adjust(-xoffset,-yoffset,xoffset,yoffset);
    }

    QRectF colorPreviewDocRect = this->colorPreviewDocRect(m_outlineDocPoint);
    QRectF colorPreviewDocUpdateRect;
    if (!colorPreviewDocRect.isEmpty()) {
        colorPreviewDocUpdateRect.adjust(-xoffset,-yoffset,xoffset,yoffset);
    }

    // DIRTY HACK ALERT: we should fetch the assistant's dirty rect when requesting
    //                   the update, instead of just dumbly update the entire canvas!

    KisCanvas2 * kiscanvas = dynamic_cast<KisCanvas2*>(canvas());
    KisPaintingAssistantsDecorationSP decoration = kiscanvas->paintingAssistantsDecoration();
    if (decoration && decoration->visible()) {
        kiscanvas->updateCanvas();
    } else {
        // TODO: only this branch should be present!
        if (!m_oldColorPreviewUpdateRect.isEmpty()) {
            canvas()->updateCanvas(m_oldColorPreviewUpdateRect);
        }

        if (!m_oldOutlineRect.isEmpty()) {
            canvas()->updateCanvas(m_oldOutlineRect);
        }

        if (!outlineDocRect.isEmpty()) {
            canvas()->updateCanvas(outlineDocRect);
        }

        if (!colorPreviewDocUpdateRect.isEmpty()) {
            canvas()->updateCanvas(colorPreviewDocUpdateRect);
        }
    }

    m_oldOutlineRect = outlineDocRect;
    m_oldColorPreviewRect = colorPreviewDocRect;
    m_oldColorPreviewUpdateRect = colorPreviewDocUpdateRect;
}

QPainterPath KisToolPaint::getOutlinePath(const QPointF &documentPos,
                                          const KoPointerEvent *event,
                                          KisPaintOpSettings::OutlineMode outlineMode)
{
    Q_UNUSED(event);

    QPointF imagePos = currentImage()->documentToPixel(documentPos);
    QPainterPath path = currentPaintOpPreset()->settings()->
        brushOutline(KisPaintInformation(imagePos), outlineMode);

    return path;
}

