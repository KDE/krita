/*
 *  Copyright (c) 2006, 2010 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_tool.h"
#include <QCursor>
#include <QLabel>
#include <QWidget>
#include <QPolygonF>
#include <QTransform>

#include <klocalizedstring.h>
#include <QAction>
#include <kactioncollection.h>

#include <kis_icon.h>
#include <KoConfig.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoToolBase.h>
#include <KoID.h>
#include <KoPointerEvent.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <resources/KoAbstractGradient.h>
#include <KoSnapGuide.h>

#include <KisViewManager.h>
#include "kis_node_manager.h"
#include <kis_selection.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_mask.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>
#include <resources/KoPattern.h>
#include <kis_floating_message.h>

#include "opengl/kis_opengl_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "canvas/kis_canvas2.h"
#include "kis_coordinates_converter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_cursor.h"
#include <kis_selection_mask.h>
#include "kis_resources_snapshot.h"
#include <KisView.h>
#include "kis_action_registry.h"
#include "kis_tool_utils.h"


struct Q_DECL_HIDDEN KisTool::Private {
    QCursor cursor; // the cursor that should be shown on tool activation.

    // From the canvas resources
    KoPattern* currentPattern{0};
    KoAbstractGradient* currentGradient{0};
    KoColor currentFgColor;
    KoColor currentBgColor;
    float currentExposure{1.0};
    KisFilterConfigurationSP currentGenerator;
    QWidget* optionWidget{0};
    ToolMode m_mode{HOVER_MODE};
    bool m_isActive{false};
};

KisTool::KisTool(KoCanvasBase * canvas, const QCursor & cursor)
    : KoToolBase(canvas)
    , d(new Private)
{
    d->cursor = cursor;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(resetCursorStyle()));
    connect(this, SIGNAL(isActiveChanged(bool)), SLOT(resetCursorStyle()));
}

KisTool::~KisTool()
{
    delete d;
}

void KisTool::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KoToolBase::activate(activation, shapes);

    resetCursorStyle();

    if (!canvas()) return;
    if (!canvas()->resourceManager()) return;


    d->currentFgColor = canvas()->resourceManager()->resource(KoCanvasResourceProvider::ForegroundColor).value<KoColor>();
    d->currentBgColor = canvas()->resourceManager()->resource(KoCanvasResourceProvider::BackgroundColor).value<KoColor>();

    if (canvas()->resourceManager()->hasResource(KisCanvasResourceProvider::CurrentPattern)) {
        d->currentPattern = canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentPattern).value<KoPattern*>();
    }

    if (canvas()->resourceManager()->hasResource(KisCanvasResourceProvider::CurrentGradient)) {
        d->currentGradient = canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentGradient).value<KoAbstractGradient*>();
    }

    KisPaintOpPresetSP preset = canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
    if (preset && preset->settings()) {
        preset->settings()->activate();
    }

    if (canvas()->resourceManager()->hasResource(KisCanvasResourceProvider::HdrExposure)) {
        d->currentExposure = static_cast<float>(canvas()->resourceManager()->resource(KisCanvasResourceProvider::HdrExposure).toDouble());
    }

    if (canvas()->resourceManager()->hasResource(KisCanvasResourceProvider::CurrentGeneratorConfiguration)) {
        d->currentGenerator = canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentGeneratorConfiguration).value<KisFilterConfiguration*>();
    }

    d->m_isActive = true;
    emit isActiveChanged(true);
}

void KisTool::deactivate()
{
    d->m_isActive = false;
    emit isActiveChanged(false);

    KoToolBase::deactivate();
}

void KisTool::canvasResourceChanged(int key, const QVariant & v)
{
    QString formattedBrushName;
    if (key == KisCanvasResourceProvider::CurrentPaintOpPreset) {
         formattedBrushName = v.value<KisPaintOpPresetSP>()->name().replace("_", " ");
    }

    switch (key) {
    case(KoCanvasResourceProvider::ForegroundColor):
        d->currentFgColor = v.value<KoColor>();
        break;
    case(KoCanvasResourceProvider::BackgroundColor):
        d->currentBgColor = v.value<KoColor>();
        break;
    case(KisCanvasResourceProvider::CurrentPattern):
        d->currentPattern = v.value<KoPattern*>();
        break;
    case(KisCanvasResourceProvider::CurrentGradient):
        d->currentGradient = static_cast<KoAbstractGradient *>(v.value<void *>());
        break;
    case(KisCanvasResourceProvider::HdrExposure):
        d->currentExposure = static_cast<float>(v.toDouble());
        break;
    case(KisCanvasResourceProvider::CurrentGeneratorConfiguration):
        d->currentGenerator = static_cast<KisFilterConfiguration*>(v.value<void *>());
        break;
    case(KisCanvasResourceProvider::CurrentPaintOpPreset):
        emit statusTextChanged(formattedBrushName);
        break;
    case(KisCanvasResourceProvider::CurrentKritaNode):
        resetCursorStyle();
        break;
    default:
        break; // Do nothing
    };
}

void KisTool::updateSettingsViews()
{
}

QPointF KisTool::widgetCenterInWidgetPixels()
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);

    const KisCoordinatesConverter *converter = kritaCanvas->coordinatesConverter();
    return converter->flakeToWidget(converter->flakeCenterPoint());
}

QPointF KisTool::convertDocumentToWidget(const QPointF& pt)
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);

    return kritaCanvas->coordinatesConverter()->documentToWidget(pt);
}

QPointF KisTool::convertToPixelCoord(KoPointerEvent *e)
{
    if (!image())
        return e->point;

    return image()->documentToPixel(e->point);
}

QPointF KisTool::convertToPixelCoord(const QPointF& pt)
{
    if (!image())
        return pt;

    return image()->documentToPixel(pt);
}

QPointF KisTool::convertToPixelCoordAndSnap(KoPointerEvent *e, const QPointF &offset, bool useModifiers)
{
    if (!image())
        return e->point;

    KoSnapGuide *snapGuide = canvas()->snapGuide();
    QPointF pos = snapGuide->snap(e->point, offset, useModifiers ? e->modifiers() : Qt::NoModifier);

    return image()->documentToPixel(pos);
}

QPointF KisTool::convertToPixelCoordAndSnap(const QPointF& pt, const QPointF &offset)
{
    if (!image())
        return pt;

    KoSnapGuide *snapGuide = canvas()->snapGuide();
    QPointF pos = snapGuide->snap(pt, offset, Qt::NoModifier);

    return image()->documentToPixel(pos);
}

QPoint KisTool::convertToImagePixelCoordFloored(KoPointerEvent *e)
{
    if (!image())
        return e->point.toPoint();

    return image()->documentToImagePixelFloored(e->point);
}

QPointF KisTool::viewToPixel(const QPointF &viewCoord) const
{
    if (!image())
        return viewCoord;

    return image()->documentToPixel(canvas()->viewConverter()->viewToDocument(viewCoord));
}

QRectF KisTool::convertToPt(const QRectF &rect)
{
    if (!image())
        return rect;
    QRectF r;
    //We add 1 in the following to the extreme coords because a pixel always has size
    r.setCoords(int(rect.left()) / image()->xRes(), int(rect.top()) / image()->yRes(),
                int(rect.right()) / image()->xRes(), int( rect.bottom()) / image()->yRes());
    return r;
}

qreal KisTool::convertToPt(qreal value)
{
    const qreal avgResolution = 0.5 * (image()->xRes() + image()->yRes());
    return value / avgResolution;
}

QPointF KisTool::pixelToView(const QPoint &pixelCoord) const
{
    if (!image())
        return pixelCoord;
    QPointF documentCoord = image()->pixelToDocument(pixelCoord);
    return canvas()->viewConverter()->documentToView(documentCoord);
}

QPointF KisTool::pixelToView(const QPointF &pixelCoord) const
{
    if (!image())
        return pixelCoord;
    QPointF documentCoord = image()->pixelToDocument(pixelCoord);
    return canvas()->viewConverter()->documentToView(documentCoord);
}

QRectF KisTool::pixelToView(const QRectF &pixelRect) const
{
    if (!image())
        return pixelRect;
    QPointF topLeft = pixelToView(pixelRect.topLeft());
    QPointF bottomRight = pixelToView(pixelRect.bottomRight());
    return QRectF(topLeft, bottomRight);
}

QPainterPath KisTool::pixelToView(const QPainterPath &pixelPolygon) const
{
    QTransform matrix;
    qreal zoomX, zoomY;
    canvas()->viewConverter()->zoom(&zoomX, &zoomY);
    matrix.scale(zoomX/image()->xRes(), zoomY/ image()->yRes());
    return matrix.map(pixelPolygon);
}

QPolygonF KisTool::pixelToView(const QPolygonF &pixelPath) const
{
    QTransform matrix;
    qreal zoomX, zoomY;
    canvas()->viewConverter()->zoom(&zoomX, &zoomY);
    matrix.scale(zoomX/image()->xRes(), zoomY/ image()->yRes());
    return matrix.map(pixelPath);
}

void KisTool::updateCanvasPixelRect(const QRectF &pixelRect)
{
    canvas()->updateCanvas(convertToPt(pixelRect));
}

void KisTool::updateCanvasViewRect(const QRectF &viewRect)
{
    canvas()->updateCanvas(canvas()->viewConverter()->viewToDocument(viewRect));
}

KisImageWSP KisTool::image() const
{
    // For now, krita tools only work in krita, not for a krita shape. Krita shapes are for 2.1
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (kisCanvas) {
        return kisCanvas->currentImage();
    }

    return 0;

}

QCursor KisTool::cursor() const
{
    return d->cursor;
}

void KisTool::notifyModified() const
{
    if (image()) {
        image()->setModified();
    }
}

KoPattern * KisTool::currentPattern()
{
    return d->currentPattern;
}

KoAbstractGradient * KisTool::currentGradient()
{
    return d->currentGradient;
}

KisPaintOpPresetSP KisTool::currentPaintOpPreset()
{
    return canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
}

KisNodeSP KisTool::currentNode() const
{
    KisNodeSP node = canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeWSP>();
    return node;
}

KisNodeList KisTool::selectedNodes() const
{
    KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
    KisViewManager* viewManager = kiscanvas->viewManager();
    return viewManager->nodeManager()->selectedNodes();
}

KoColor KisTool::currentFgColor()
{
    return d->currentFgColor;
}

KoColor KisTool::currentBgColor()
{
    return d->currentBgColor;
}

KisImageWSP KisTool::currentImage()
{
    return image();
}

KisFilterConfigurationSP  KisTool::currentGenerator()
{
    return d->currentGenerator;
}

void KisTool::setMode(ToolMode mode) {
    d->m_mode = mode;
}

KisTool::ToolMode KisTool::mode() const {
    return d->m_mode;
}

void KisTool::setCursor(const QCursor &cursor)
{
    d->cursor = cursor;
}

KisTool::AlternateAction KisTool::actionToAlternateAction(ToolAction action) {
    KIS_ASSERT_RECOVER_RETURN_VALUE(action != Primary, Secondary);
    return (AlternateAction)action;
}

void KisTool::activatePrimaryAction()
{
    resetCursorStyle();
}

void KisTool::deactivatePrimaryAction()
{
    resetCursorStyle();
}

void KisTool::beginPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
}

void KisTool::beginPrimaryDoubleClickAction(KoPointerEvent *event)
{
    beginPrimaryAction(event);
}

void KisTool::continuePrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
}

void KisTool::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
}

bool KisTool::primaryActionSupportsHiResEvents() const
{
    return false;
}

void KisTool::activateAlternateAction(AlternateAction action)
{
    Q_UNUSED(action);
}

void KisTool::deactivateAlternateAction(AlternateAction action)
{
    Q_UNUSED(action);
}

void KisTool::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    Q_UNUSED(event);
    Q_UNUSED(action);
}

void KisTool::beginAlternateDoubleClickAction(KoPointerEvent *event, AlternateAction action)
{
    beginAlternateAction(event, action);
}

void KisTool::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    Q_UNUSED(event);
    Q_UNUSED(action);
}

void KisTool::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    Q_UNUSED(event);
    Q_UNUSED(action);
}

void KisTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
}

void KisTool::mouseTripleClickEvent(KoPointerEvent *event)
{
    mouseDoubleClickEvent(event);
}

void KisTool::mousePressEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
}

void KisTool::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
}

void KisTool::mouseMoveEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
}

void KisTool::deleteSelection()
{
    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image(), currentNode(), this->canvas()->resourceManager());

    if (!blockUntilOperationsFinished()) {
        return;
    }

    if (!KisToolUtils::clearImage(image(), resources->currentNode(), resources->activeSelection())) {
        KoToolBase::deleteSelection();
    }
}

QWidget* KisTool::createOptionWidget()
{
    d->optionWidget = new QLabel(i18n("No options"));
    d->optionWidget->setObjectName("SpecialSpacer");
    return d->optionWidget;
}

#define NEAR_VAL -1000.0
#define FAR_VAL 1000.0
#define PROGRAM_VERTEX_ATTRIBUTE 0

void KisTool::paintToolOutline(QPainter* painter, const QPainterPath &path)
{
    KisOpenGLCanvas2 *canvasWidget = dynamic_cast<KisOpenGLCanvas2 *>(canvas()->canvasWidget());
    if (canvasWidget)  {
        painter->beginNativePainting();
        canvasWidget->paintToolOutline(path);
        painter->endNativePainting();
    }
    else {
        painter->save();
        painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painter->setPen(QColor(128, 255, 128));
        painter->drawPath(path);
        painter->restore();
    }
}

void KisTool::resetCursorStyle()
{
    useCursor(d->cursor);
}

bool KisTool::overrideCursorIfNotEditable()
{
    // override cursor for canvas iff this tool is active
    // and we can't paint on the active layer
    if (isActive()) {
        KisNodeSP node = currentNode();
        if (node && !node->isEditable()) {
            canvas()->setCursor(Qt::ForbiddenCursor);
            return true;
        }
    }
    return false;
}

bool KisTool::blockUntilOperationsFinished()
{
    KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
    KisViewManager* viewManager = kiscanvas->viewManager();
    return viewManager->blockUntilOperationsFinished(image());
}

void KisTool::blockUntilOperationsFinishedForced()
{
    KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
    KisViewManager* viewManager = kiscanvas->viewManager();
    viewManager->blockUntilOperationsFinishedForced(image());
}

bool KisTool::isActive() const
{
    return d->m_isActive;
}

bool KisTool::nodeEditable()
{
    KisNodeSP node = currentNode();
    if (!node) {
        return false;
    }

    bool blockedNoIndirectPainting = false;

    const bool presetUsesIndirectPainting =
        !currentPaintOpPreset()->settings()->paintIncremental();

    if (!presetUsesIndirectPainting) {
        const KisIndirectPaintingSupport *indirectPaintingLayer =
                dynamic_cast<const KisIndirectPaintingSupport*>(node.data());
        if (indirectPaintingLayer) {
            blockedNoIndirectPainting = !indirectPaintingLayer->supportsNonIndirectPainting();
        }
    }

    bool nodeEditable = node->isEditable() && !blockedNoIndirectPainting;

    if (!nodeEditable) {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        QString message;
        if (!node->visible() && node->userLocked()) {
            message = i18n("Layer is locked and invisible.");
        } else if (node->userLocked()) {
            message = i18n("Layer is locked.");
        } else if(!node->visible()) {
            message = i18n("Layer is invisible.");
        } else if (blockedNoIndirectPainting) {
            message = i18n("Layer can be painted in Wash Mode only.");
        } else {
            message = i18n("Group not editable.");
        }
        kiscanvas->viewManager()->showFloatingMessage(message, KisIconUtils::loadIcon("object-locked"));
    }
    return nodeEditable;
}

bool KisTool::selectionEditable()
{
    KisCanvas2 * kisCanvas = static_cast<KisCanvas2*>(canvas());
    KisViewManager * view = kisCanvas->viewManager();

    bool editable = view->selectionEditable();
    if (!editable) {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        kiscanvas->viewManager()->showFloatingMessage(i18n("Local selection is locked."), KisIconUtils::loadIcon("object-locked"));
    }
    return editable;
}

void KisTool::listenToModifiers(bool listen)
{
    Q_UNUSED(listen);
}

bool KisTool::listeningToModifiers()
{
    return false;
}
