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

#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>

#include <KoIcon.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoShapeManager.h>
#include <KoToolBase.h>
#include <KoColor.h>
#include <KoID.h>
#include <KoPointerEvent.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <KoAbstractGradient.h>

#include <opengl/kis_opengl.h>

#ifdef HAVE_OPENGL
#include <opengl/kis_opengl_canvas2.h>
#endif

#include <kis_view2.h>
#include <kis_selection.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_mask.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_paintop_preset.h>
#include <kis_paintop_settings.h>
#include <kis_pattern.h>
#include <kis_transaction.h>
#include <kis_selection.h>
#include <kis_floating_message.h>

#include "kis_canvas_resource_provider.h"
#include "canvas/kis_canvas2.h"
#include "kis_coordinates_converter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_cursor.h"
#include <recorder/kis_recorded_paint_action.h>
#include <kis_selection_mask.h>

struct KisTool::Private {
    Private()
        : currentPattern(0),
          currentGradient(0),
          currentPaintOpPreset(0),
          currentGenerator(0),
          optionWidget(0),
          spacePressed(0) { }
    QCursor cursor; // the cursor that should be shown on tool activation.

    // From the canvas resources
    KisPattern * currentPattern;
    KoAbstractGradient * currentGradient;
    KoColor currentFgColor;
    KoColor currentBgColor;
    KisPaintOpPresetSP currentPaintOpPreset;
    KisNodeSP currentNode;
    float currentExposure;
    KisFilterConfiguration * currentGenerator;
    QWidget* optionWidget;

    bool spacePressed;
    QPointF lastDocumentPoint;
    QPointF initialGestureDocPoint;
    QPoint initialGestureGlobalPoint;
};

KisTool::KisTool(KoCanvasBase * canvas, const QCursor & cursor)
        : KoToolBase(canvas)
        , d(new Private)
{
    d->cursor = cursor;
    m_outlinePaintMode = XOR_MODE;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(resetCursorStyle()));

    KActionCollection *collection = this->canvas()->canvasController()->actionCollection();

    if (!collection->action("toggle_fg_bg")) {
        KAction *toggleFgBg = new KAction(i18n("Swap Foreground and Background Color"), collection);
        toggleFgBg->setShortcut(QKeySequence(Qt::Key_X));
        collection->addAction("toggle_fg_bg", toggleFgBg);
    }

    if (!collection->action("reset_fg_bg")) {
        KAction *resetFgBg = new KAction(i18n("Reset Foreground and Background Color"), collection);
        resetFgBg->setShortcut(QKeySequence(Qt::Key_D));
        collection->addAction("reset_fg_bg", resetFgBg);
    }

    addAction("toggle_fg_bg", dynamic_cast<KAction*>(collection->action("toggle_fg_bg")));
    addAction("reset_fg_bg", dynamic_cast<KAction*>(collection->action("reset_fg_bg")));

    setMode(HOVER_MODE);
}

KisTool::~KisTool()
{
    delete d;
}

void KisTool::activate(ToolActivation, const QSet<KoShape*> &)
{
    resetCursorStyle();

    d->currentFgColor = canvas()->resourceManager()->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
    d->currentBgColor = canvas()->resourceManager()->resource(KoCanvasResourceManager::BackgroundColor).value<KoColor>();
    d->currentPattern = static_cast<KisPattern *>(canvas()->resourceManager()->
                        resource(KisCanvasResourceProvider::CurrentPattern).value<void *>());
    d->currentGradient = static_cast<KoAbstractGradient *>(canvas()->resourceManager()->
                         resource(KisCanvasResourceProvider::CurrentGradient).value<void *>());


    d->currentPaintOpPreset =
        canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();

    if (d->currentPaintOpPreset && d->currentPaintOpPreset->settings()) {
        d->currentPaintOpPreset->settings()->activate();
    }

    d->currentNode = canvas()->resourceManager()->
                     resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
    d->currentExposure = static_cast<float>(canvas()->resourceManager()->
                                            resource(KisCanvasResourceProvider::HdrExposure).toDouble());
    d->currentGenerator = static_cast<KisFilterConfiguration*>(canvas()->resourceManager()->
                          resource(KisCanvasResourceProvider::CurrentGeneratorConfiguration).value<void *>());

    connect(actions().value("toggle_fg_bg"), SIGNAL(triggered()), SLOT(slotToggleFgBg()), Qt::UniqueConnection);
    connect(actions().value("reset_fg_bg"), SIGNAL(triggered()), SLOT(slotResetFgBg()), Qt::UniqueConnection);
}

void KisTool::deactivate()
{
    disconnect(actions().value("toggle_fg_bg"), 0, this, 0);
    disconnect(actions().value("reset_fg_bg"), 0, this, 0);
}

void KisTool::resourceChanged(int key, const QVariant & v)
{

    switch (key) {
    case(KoCanvasResourceManager::ForegroundColor):
        d->currentFgColor = v.value<KoColor>();
        break;
    case(KoCanvasResourceManager::BackgroundColor):
        d->currentBgColor = v.value<KoColor>();
        break;
    case(KisCanvasResourceProvider::CurrentPattern):
        d->currentPattern = static_cast<KisPattern *>(v.value<void *>());
        break;
    case(KisCanvasResourceProvider::CurrentGradient):
        d->currentGradient = static_cast<KoAbstractGradient *>(v.value<void *>());
        break;
    case(KisCanvasResourceProvider::CurrentPaintOpPreset):
        d->currentPaintOpPreset =
            canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
        break;
    case(KisCanvasResourceProvider::HdrExposure):
        d->currentExposure = static_cast<float>(v.toDouble());
    case(KisCanvasResourceProvider::CurrentGeneratorConfiguration):
        d->currentGenerator = static_cast<KisFilterConfiguration*>(v.value<void *>());
    case(KisCanvasResourceProvider::CurrentKritaNode):
        d->currentNode = (v.value<KisNodeSP>());
    default:
        ;
        // Do nothing
    };
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

QPoint KisTool::convertToIntPixelCoord(KoPointerEvent *e)
{
    if (!image())
        return e->point.toPoint();

    return image()->documentToIntPixel(e->point);
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
                int(1 + rect.right()) / image()->xRes(), int(1 + rect.bottom()) / image()->yRes());
    return r;
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

KisSelectionSP KisTool::currentSelection() const
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (kisCanvas) {
        KisView2 * view = kisCanvas->view();
        if (view) return view->selection();
    }

    return 0;

}

void KisTool::notifyModified() const
{
    if (image()) {
        image()->setModified();
    }
}

KisPattern * KisTool::currentPattern()
{
    return d->currentPattern;
}

KoAbstractGradient * KisTool::currentGradient()
{
    return d->currentGradient;
}

KisPaintOpPresetSP KisTool::currentPaintOpPreset()
{
    return d->currentPaintOpPreset;
}

KisNodeSP KisTool::currentNode()
{
    return d->currentNode;
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

KisFilterConfiguration * KisTool::currentGenerator()
{
    return d->currentGenerator;
}

bool KisTool::specialModifierActive()
{
    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    bool popupPalletActive = canvas2->handlePopupPaletteIsVisible();

    KisConfig cfg;
    return popupPalletActive || (d->spacePressed && !cfg.clicklessSpacePan());
}

void KisTool::setMode(ToolMode mode) {
    m_mode = mode;
}

KisTool::ToolMode KisTool::mode() const {
    return m_mode;
}

void KisTool::mousePressEvent(KoPointerEvent *event)
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (kisCanvas) {
        kisCanvas->setSmoothingEnabled(false);
    }

    KisConfig cfg;

    if (mode() == KisTool::HOVER_MODE &&
             (event->button() == Qt::LeftButton &&
              event->modifiers() == Qt::ShiftModifier)) {

        initGesture(event->point);
        event->accept();
    }


    d->lastDocumentPoint = event->point;
}

void KisTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (mode() == GESTURE_MODE) {
        processGesture(event->point);
        event->accept();
    }

    d->lastDocumentPoint = event->point;
}

void KisTool::mouseReleaseEvent(KoPointerEvent *event)
{

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (kisCanvas) {
        kisCanvas->setSmoothingEnabled(true);
    }
    KisConfig cfg;

    if (mode() == GESTURE_MODE) {
        if (event->button() == Qt::LeftButton) {
            endGesture();
            event->accept();
        }
    }

    d->lastDocumentPoint = event->point;
}

void KisTool::keyPressEvent(QKeyEvent *event)
{
    if (mode() == GESTURE_MODE) {
        event->accept();
    } else {
        event->ignore();
    }
}

void KisTool::keyReleaseEvent(QKeyEvent* event)
{
    if (mode() == GESTURE_MODE) {
        event->accept();
    } else {
        event->ignore();
    }
}

void KisTool::initGesture(const QPointF &docPoint)
{
    setMode(GESTURE_MODE);
    d->initialGestureDocPoint = docPoint;
    d->initialGestureGlobalPoint = QCursor::pos();
    useCursor(KisCursor::blankCursor());
}

void KisTool::processGesture(const QPointF &docPoint)
{
    QPointF lastWidgetPosition = convertDocumentToWidget(d->lastDocumentPoint);
    QPointF actualWidgetPosition = convertDocumentToWidget(docPoint);

    QPointF offset = actualWidgetPosition - lastWidgetPosition;

    /**
     * view pixels != widget pixels, but we do this anyway, we only
     * need to scale the gesture down, not rotate or anything
     */
    QPointF scaledOffset = canvas()->viewConverter()->viewToDocument(offset);
    gesture(scaledOffset, d->initialGestureDocPoint);
}

void KisTool::endGesture()
{
    gesture(QPointF(), d->initialGestureDocPoint);
    setMode(HOVER_MODE);
    resetCursorStyle();
    QCursor::setPos(d->initialGestureGlobalPoint);
}

void KisTool::gesture(const QPointF &offsetInDocPixels, const QPointF &initialDocPoint)
{
    Q_UNUSED(offsetInDocPixels);
    Q_UNUSED(initialDocPoint);
}

void KisTool::deleteSelection()
{
    KisSelectionSP selection = currentSelection();
    KisNodeSP node = currentNode();
    KisPaintDeviceSP device;

    if(node && (device = node->paintDevice())) {
        KisTransaction transaction(i18n("Clear"), device);

        QRect dirtyRect;
        if (selection) {
            dirtyRect = selection->selectedRect();
            device->clearSelection(selection);
        }
        else {
            dirtyRect = device->extent();
            device->clear();
        }

        transaction.commit(image()->undoAdapter());
        device->setDirty(dirtyRect);
    }
    else {
        KoToolBase::deleteSelection();
    }
}

void KisTool::setupPaintAction(KisRecordedPaintAction* action)
{
    action->setPaintColor(currentFgColor());
    action->setBackgroundColor(currentBgColor());
}

QWidget* KisTool::createOptionWidget()
{
    d->optionWidget = new QLabel(i18n("No options"));
    d->optionWidget->setObjectName(toolId() + " Option Widget");
    return d->optionWidget;
}

void KisTool::paintToolOutline(QPainter* painter, const QPainterPath &path)
{
    //KisToolSelectMagnetic uses custom painting, so don't forget to update that as well
    KisConfig cfg;
    bool useWorkaround = cfg.useOpenGLToolOutlineWorkaround();
#if defined(HAVE_OPENGL)

    if (m_outlinePaintMode==XOR_MODE && isCanvasOpenGL() && !useWorkaround) {
        beginOpenGL();

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_XOR);
        glColor3f(0.501961, 1.0, 0.501961);

        QList<QPolygonF> subPathPolygons = path.toSubpathPolygons();
        for(int i=0; i<subPathPolygons.size(); i++) {
            const QPolygonF& polygon = subPathPolygons.at(i);

            glBegin(GL_LINE_STRIP);
            for(int j=0; j<polygon.count(); j++) {
                QPointF p = polygon.at(j);
                glVertex2f(p.x(), p.y());
            }
            glEnd();
        }

        glDisable(GL_COLOR_LOGIC_OP);
        glDisable(GL_LINE_SMOOTH);

        endOpenGL();
    }
    else
#endif
    if (m_outlinePaintMode==XOR_MODE && !(isCanvasOpenGL() && useWorkaround)) {
        painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painter->setPen(QColor(128, 255, 128));
        painter->drawPath(path);
    }
    else/* if (m_outlinePaintMode==BW_MODE)*/
    {
        QPen pen = painter->pen();
        pen.setWidth(3);
        pen.setColor(QColor(0, 0, 0, 100));
        painter->setPen(pen);
        painter->drawPath(path);
        pen.setWidth(1);
        pen.setColor(Qt::white);
        painter->setPen(pen);
        painter->drawPath(path);
    }
}

void KisTool::resetCursorStyle()
{
    KisConfig cfg;
    switch (cfg.cursorStyle()) {
    case CURSOR_STYLE_TOOLICON:
        useCursor(d->cursor);
        break;
    case CURSOR_STYLE_CROSSHAIR:
        useCursor(KisCursor::crossCursor());
        break;
    case CURSOR_STYLE_SMALL_ROUND:
        useCursor(KisCursor::roundCursor());
        break;
    case CURSOR_STYLE_POINTER:
        useCursor(KisCursor::upArrowCursor());
        break;
    case CURSOR_STYLE_NO_CURSOR:
        useCursor(KisCursor::blankCursor());
        break;
#if defined(HAVE_OPENGL)
    case CURSOR_STYLE_3D_MODEL:
        if(isCanvasOpenGL()) {
            useCursor(d->cursor);
        } else {
            useCursor(KisCursor::upArrowCursor());
        }
        break;
#endif
    case CURSOR_STYLE_OUTLINE:
    default:
        // use tool cursor as default, if the tool supports outline, it will set the cursor to blank and show outline
        useCursor(d->cursor);
    }
}


void KisTool::slotToggleFgBg()
{
    KoCanvasResourceManager* resourceManager = canvas()->resourceManager();
    KoColor c = resourceManager->foregroundColor();
    resourceManager->setForegroundColor(resourceManager->backgroundColor());
    resourceManager->setBackgroundColor(c);
}

void KisTool::slotResetFgBg()
{
    KoCanvasResourceManager* resourceManager = canvas()->resourceManager();
    resourceManager->setForegroundColor(KoColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8()));
    resourceManager->setBackgroundColor(KoColor(Qt::white, KoColorSpaceRegistry::instance()->rgb8()));
}

bool KisTool::isCanvasOpenGL() const
{
    return canvas()->canvasIsOpenGL();
}

void KisTool::beginOpenGL()
{
#if defined(HAVE_OPENGL)
    KisOpenGLCanvas2 *canvasWidget = dynamic_cast<KisOpenGLCanvas2 *>(canvas()->canvasWidget());
    Q_ASSERT(canvasWidget);

    if (canvasWidget) {
        canvasWidget->beginOpenGL();
        canvasWidget->setupFlakeToWidgetTransformation();
    }
#endif
}

void KisTool::endOpenGL()
{
#if defined(HAVE_OPENGL)
    KisOpenGLCanvas2 *canvasWidget = dynamic_cast<KisOpenGLCanvas2 *>(canvas()->canvasWidget());
    Q_ASSERT(canvasWidget);

    if (canvasWidget) {
        canvasWidget->endOpenGL();
    }
#endif
}

void KisTool::setCurrentNodeLocked(bool locked)
{
    if (currentNode()) {
        currentNode()->setSystemLocked(locked, false);
    }
}

bool KisTool::nodeEditable()
{
    KisNodeSP node = currentNode();
    if (!node) {
        return false;
    }
    if (!node->isEditable()) {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        QString message;
        if (!node->visible() && node->userLocked()) {
            message = i18n("Layer is locked and invisible.");
        } else if (node->userLocked()) {
            message = i18n("Layer is locked.");
        } else if(!node->visible()) {
            message = i18n("Layer is invisible.");
        } else {
            message = i18n("Group not editable.");
        }
        kiscanvas->view()->showFloatingMessage(message, koIcon("object-locked"));
    }
    return node->isEditable();
}

bool KisTool::selectionEditable()
{
    KisCanvas2 * kisCanvas = static_cast<KisCanvas2*>(canvas());
    KisView2 * view = kisCanvas->view();

    bool editable = view->selectionEditable();
    if (!editable) {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        kiscanvas->view()->showFloatingMessage(i18n("Local selection is locked."), koIcon("object-locked"));
    }
    return editable;
}


#include "kis_tool.moc"

