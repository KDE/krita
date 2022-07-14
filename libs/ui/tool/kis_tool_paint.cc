/*
 *  SPDX-FileCopyrightText: 2003-2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2015 Moritz Molch <kde@moritzmolch.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_image_config.h"
#include "widgets/kis_cmb_composite.h"
#include "kis_slider_spin_box.h"
#include "kis_canvas_resource_provider.h"
#include "kis_tool_utils.h"
#include <brushengine/kis_paintop.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/KisOptimizedBrushOutline.h>
#include <kis_action_manager.h>
#include <kis_action.h>
#include "strokes/kis_color_sampler_stroke_strategy.h"
#include "kis_popup_palette.h"


KisToolPaint::KisToolPaint(KoCanvasBase *canvas, const QCursor &cursor)
    : KisTool(canvas, cursor),
      m_isOutlineEnabled(true),
      m_isOutlineVisible(true),
      m_colorSamplerHelper(dynamic_cast<KisCanvas2*>(canvas))
{

    {
        const int maxSize = KisImageConfig(true).maxBrushSize();

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

    connect(&m_colorSamplerHelper, SIGNAL(sigRequestCursor(QCursor)), this, SLOT(slotColorPickerRequestedCursor(QCursor)));
    connect(&m_colorSamplerHelper, SIGNAL(sigRequestCursorReset()), this, SLOT(slotColorPickerRequestedCursorReset()));
    connect(&m_colorSamplerHelper, SIGNAL(sigRequestUpdateOutline()), this, SLOT(slotColorPickerRequestedOutlineUpdate()));
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
    case(KoCanvasResource::Opacity):
        setOpacity(v.toDouble());
        break;
    case(KoCanvasResource::CurrentPaintOpPreset): {
        if (isActive()) {
            requestUpdateOutline(m_outlineDocPoint, 0);
        }
        break;
    }
    case KoCanvasResource::CurrentPaintOpPresetName: {
        if (isActive()) {
            const QString formattedBrushName = v.toString().replace("_", " ");
            emit statusTextChanged(formattedBrushName);
        }
        break;
    }
    default: //nothing
        break;
    }

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(resetCursorStyle()), Qt::UniqueConnection);

}

void KisToolPaint::tryRestoreOpacitySnapshot()
{
    /**
     * Here is a weird heuristics on when to restore
     * brush opacity and when not. Basically, we should
     * restore opacity to its saved if the brush preset
     * hasn't changed too much, that is, its version is
     * the same and it hasn't been reset into a clean
     * state since then. The latter condition is checked
     * in a fuzzy manner by just mangling the isDirty
     * state before and after.
     */

    KisCanvasResourceProvider *provider = qobject_cast<KisCanvas2*>(canvas())->viewManager()->canvasResourceProvider();

    boost::optional<qreal> opacityToRestore;

    KisPaintOpPresetSP newPreset = provider->currentPreset();

    if (newPreset) {
        if (newPreset == m_oldPreset && newPreset->version() == m_oldPresetVersion
            && (newPreset->isDirty() || !m_oldPresetIsDirty)) {

            opacityToRestore = m_oldOpacity;
        }

        m_oldPreset = newPreset;
        m_oldPresetIsDirty = newPreset->isDirty();
        m_oldPresetVersion = newPreset->version();
        m_oldOpacity = provider->opacity();
    }

    if (opacityToRestore) {
        provider->setOpacity(*opacityToRestore);
    }
}


void KisToolPaint::activate(const QSet<KoShape*> &shapes)
{
    if (currentPaintOpPreset()) {
        const QString formattedBrushName = currentPaintOpPreset() ? currentPaintOpPreset()->name().replace("_", " ") : QString();
        emit statusTextChanged(formattedBrushName);
    }

    KisTool::activate(shapes);
    if (flags() & KisTool::FLAG_USES_CUSTOM_SIZE) {
        connect(action("increase_brush_size"), SIGNAL(triggered()), SLOT(increaseBrushSize()), Qt::UniqueConnection);
        connect(action("decrease_brush_size"), SIGNAL(triggered()), SLOT(decreaseBrushSize()), Qt::UniqueConnection);
        connect(action("increase_brush_size"), SIGNAL(triggered()), this, SLOT(showBrushSize()));
        connect(action("decrease_brush_size"), SIGNAL(triggered()), this, SLOT(showBrushSize()));

    }

    tryRestoreOpacitySnapshot();
}

void KisToolPaint::deactivate()
{
    if (flags() & KisTool::FLAG_USES_CUSTOM_SIZE) {
        disconnect(action("increase_brush_size"), 0, this, 0);
        disconnect(action("decrease_brush_size"), 0, this, 0);
    }

    tryRestoreOpacitySnapshot();
    emit statusTextChanged(QString());

    KisTool::deactivate();
}

void KisToolPaint::slotColorPickerRequestedCursor(const QCursor &cursor)
{
    useCursor(cursor);
}

void KisToolPaint::slotColorPickerRequestedCursorReset()
{
    resetCursorStyle();
}

void KisToolPaint::slotColorPickerRequestedOutlineUpdate()
{
    requestUpdateOutline(m_outlineDocPoint, 0);
}

KisOptimizedBrushOutline KisToolPaint::tryFixBrushOutline(const KisOptimizedBrushOutline &originalOutline)
{
    KisConfig cfg(true);

    bool useSeparateEraserCursor = cfg.separateEraserCursor() &&
            canvas()->resourceManager()->resource(KoCanvasResource::CurrentEffectiveCompositeOp).toString() == COMPOSITE_ERASE;

    const OutlineStyle currentOutlineStyle = !useSeparateEraserCursor ? cfg.newOutlineStyle() : cfg.eraserOutlineStyle();
    if (currentOutlineStyle == OUTLINE_NONE) return originalOutline;

    const qreal minThresholdSize = cfg.outlineSizeMinimum();

    /**
     * If the brush outline is bigger than the canvas itself (which
     * would make it invisible for a user in most of the cases) just
     * add a cross in the center of it
     */

    QSize widgetSize = canvas()->canvasWidget()->size();
    const int maxThresholdSum = widgetSize.width() + widgetSize.height();

    KisOptimizedBrushOutline outline = originalOutline;
    QRectF boundingRect = outline.boundingRect();
    const qreal sum = boundingRect.width() + boundingRect.height();

    QPointF center = boundingRect.center();

    if (sum > maxThresholdSum) {
        const int hairOffset = 7;

        QPainterPath crossIcon;

        crossIcon.moveTo(center.x(), center.y() - hairOffset);
        crossIcon.lineTo(center.x(), center.y() + hairOffset);

        crossIcon.moveTo(center.x() - hairOffset, center.y());
        crossIcon.lineTo(center.x() + hairOffset, center.y());

        outline.addPath(crossIcon);

    } else if (sum < minThresholdSize && !outline.isEmpty()) {
        outline = QPainterPath();
        outline.addEllipse(center, 0.5 * minThresholdSize, 0.5 * minThresholdSize);
    }

    return outline;
}

void KisToolPaint::paint(QPainter &gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    KisOptimizedBrushOutline path = tryFixBrushOutline(pixelToView(m_currentOutline));
    paintToolOutline(&gc, path);

    m_colorSamplerHelper.paint(gc, converter);
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

void KisToolPaint::activateAlternateAction(AlternateAction action)
{
    if (!isSamplingAction(action)) {
        KisTool::activateAlternateAction(action);
        return;
    }

    const bool sampleCurrentLayer = action == SampleFgNode || action == SampleBgNode;
    const bool sampleFgColor = action == SampleFgNode || action == SampleFgImage;
    m_colorSamplerHelper.activate(sampleCurrentLayer, sampleFgColor);
}

void KisToolPaint::deactivateAlternateAction(AlternateAction action)
{
    if (!isSamplingAction(action)) {
        KisTool::deactivateAlternateAction(action);
        return;
    }

    m_colorSamplerHelper.deactivate();
}

bool KisToolPaint::isSamplingAction(AlternateAction action) {
    return action == SampleFgNode ||
        action == SampleBgNode ||
        action == SampleFgImage ||
        action == SampleBgImage;
}

void KisToolPaint::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (isSamplingAction(action)) {
        setMode(SECONDARY_PAINT_MODE);

        KisToolUtils::ColorSamplerConfig config;
        config.load();

        m_colorSamplerHelper.startAction(event->point, config.radius, config.blend);
        requestUpdateOutline(event->point, event);
    } else {
        KisTool::beginAlternateAction(event, action);
    }
}

void KisToolPaint::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (isSamplingAction(action)) {
        m_colorSamplerHelper.continueAction(event->point);
        requestUpdateOutline(event->point, event);
    } else {
        KisTool::continueAlternateAction(event, action);
    }
}

void KisToolPaint::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (isSamplingAction(action)) {
        m_colorSamplerHelper.endAction();
        requestUpdateOutline(event->point, event);
        setMode(HOVER_MODE);
    } else {
        KisTool::endAlternateAction(event, action);
    }
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

KisPopupWidgetInterface *KisToolPaint::popupWidget()
{
    KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());

    if (!kisCanvas) {
        return nullptr;
    }

    KisPopupWidgetInterface* popupWidget = kisCanvas->popupPalette();
    return popupWidget;
}

void KisToolPaint::mouseReleaseEvent(KoPointerEvent *event)
{
    KisTool::mouseReleaseEvent(event);
    if (mode() == KisTool::HOVER_MODE) {
        requestUpdateOutline(event->point, event);
    }
}

QWidget *KisToolPaint::createOptionWidget()
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

void KisToolPaint::slotPopupQuickHelp()
{
    QWhatsThis::showText(QCursor::pos(), quickHelp());
}

void KisToolPaint::activatePrimaryAction()
{
    setOutlineVisible(true);
    KisTool::activatePrimaryAction();
}

void KisToolPaint::deactivatePrimaryAction()
{
    setOutlineVisible(false);
    KisTool::deactivatePrimaryAction();
}

bool KisToolPaint::isOutlineEnabled() const
{
    return m_isOutlineEnabled;
}

void KisToolPaint::setOutlineEnabled(bool enabled)
{
    m_isOutlineEnabled = enabled;
    requestUpdateOutline(m_outlineDocPoint, lastDeliveredPointerEvent());
}

bool KisToolPaint::isOutlineVisible() const
{
    return m_isOutlineVisible;
}

void KisToolPaint::setOutlineVisible(bool visible)
{
    m_isOutlineVisible = visible;
    requestUpdateOutline(m_outlineDocPoint, lastDeliveredPointerEvent());
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

void KisToolPaint::showBrushSize()
{
     KisCanvas2 *kisCanvas =dynamic_cast<KisCanvas2*>(canvas());
     kisCanvas->viewManager()->showFloatingMessage(i18n("Brush Size: %1 px", currentPaintOpPreset()->settings()->paintOpSize())
                                                   , QIcon(), 1000, KisFloatingMessage::High,  Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter);
}

void KisToolPaint::requestUpdateOutline(const QPointF &outlineDocPoint, const KoPointerEvent *event)
{
    QRectF outlinePixelRect;
    QRectF outlineDocRect;

    QRectF colorPreviewDocUpdateRect;

    if (m_supportOutline) {
        KisConfig cfg(true);
        KisPaintOpSettings::OutlineMode outlineMode;

        bool useSeparateEraserCursor = cfg.separateEraserCursor() &&
                canvas()->resourceManager()->resource(KoCanvasResource::CurrentEffectiveCompositeOp).toString() == COMPOSITE_ERASE;

        const OutlineStyle currentOutlineStyle = !useSeparateEraserCursor ? cfg.newOutlineStyle() : cfg.eraserOutlineStyle();
        const auto outlineStyleIsVisible = [&]() {
            return currentOutlineStyle == OUTLINE_FULL ||
                   currentOutlineStyle == OUTLINE_CIRCLE ||
                   currentOutlineStyle == OUTLINE_TILT;
        };
        const auto shouldShowOutlineWhilePainting = [&]() {
            return !useSeparateEraserCursor ? cfg.showOutlineWhilePainting() : cfg.showEraserOutlineWhilePainting();
        };
        if (isOutlineEnabled() && isOutlineVisible() &&
                (mode() == KisTool::GESTURE_MODE ||
                    (outlineStyleIsVisible() &&
                        (mode() == HOVER_MODE ||
                         (mode() == PAINT_MODE && shouldShowOutlineWhilePainting()))))) { // lisp forever!

            outlineMode.isVisible = true;

            switch (!useSeparateEraserCursor ? cfg.newOutlineStyle() : cfg.eraserOutlineStyle()) {
            case OUTLINE_CIRCLE:
                outlineMode.forceCircle = true;
                break;
            case OUTLINE_TILT:
                outlineMode.forceCircle = true;
                outlineMode.showTiltDecoration = true;
                break;
            default:
                break;
            }
        }

        outlineMode.forceFullSize = !useSeparateEraserCursor ? cfg.forceAlwaysFullSizedOutline() : cfg.forceAlwaysFullSizedEraserOutline();

        m_outlineDocPoint = outlineDocPoint;
        m_currentOutline = getOutlinePath(m_outlineDocPoint, event, outlineMode);

        outlinePixelRect = tryFixBrushOutline(m_currentOutline).boundingRect();
        outlineDocRect = currentImage()->pixelToDocument(outlinePixelRect);

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

        colorPreviewDocUpdateRect = m_colorSamplerHelper.colorPreviewDocRect(m_outlineDocPoint);

        if (!colorPreviewDocUpdateRect.isEmpty()) {
            colorPreviewDocUpdateRect = colorPreviewDocUpdateRect.adjusted(-xoffset,-yoffset,xoffset,yoffset);
        }

    }

    // DIRTY HACK ALERT: we should fetch the assistant's dirty rect when requesting
    //                   the update, instead of just dumbly update the entire canvas!

    // WARNING: assistants code is also duplicated in KisDelegatedSelectPathWrapper::mouseMoveEvent

    KisCanvas2 *kiscanvas = qobject_cast<KisCanvas2*>(canvas());
    KisPaintingAssistantsDecorationSP decoration = kiscanvas->paintingAssistantsDecoration();
    if (decoration && decoration->visible() && decoration->hasPaintableAssistants()) {
        kiscanvas->updateCanvasDecorations();
    }

    if (!m_oldColorPreviewUpdateRect.isEmpty()) {
        kiscanvas->updateCanvasToolOutlineDoc(m_oldColorPreviewUpdateRect);
    }

    if (!m_oldOutlineRect.isEmpty()) {
        kiscanvas->updateCanvasToolOutlineDoc(m_oldOutlineRect);
    }

    if (!outlineDocRect.isEmpty()) {
        kiscanvas->updateCanvasToolOutlineDoc(outlineDocRect);
    }

    if (!colorPreviewDocUpdateRect.isEmpty()) {
        kiscanvas->updateCanvasToolOutlineDoc(colorPreviewDocUpdateRect);
    }

    m_oldOutlineRect = outlineDocRect;
    m_oldColorPreviewUpdateRect = colorPreviewDocUpdateRect;
}

KisOptimizedBrushOutline KisToolPaint::getOutlinePath(const QPointF &documentPos,
                                                      const KoPointerEvent *event,
                                                      KisPaintOpSettings::OutlineMode outlineMode)
{
    Q_UNUSED(event);

    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    const KisCoordinatesConverter *converter = canvas2->coordinatesConverter();

    KisPaintInformation info(convertToPixelCoord(documentPos));
    info.setCanvasMirroredH(canvas2->coordinatesConverter()->xAxisMirrored());
    info.setCanvasMirroredV(canvas2->coordinatesConverter()->yAxisMirrored());
    info.setCanvasRotation(canvas2->coordinatesConverter()->rotationAngle());
    info.setRandomSource(new KisRandomSource());
    info.setPerStrokeRandomSource(new KisPerStrokeRandomSource());

    KisOptimizedBrushOutline path = currentPaintOpPreset()->settings()->
        brushOutline(info,
                     outlineMode, converter->effectivePhysicalZoom());

    return path;
}

