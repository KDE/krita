/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#include <klocale.h>
#include <kaction.h>

#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoShapeManager.h>
#include <KoTool.h>
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

#include "kis_node_shape.h"
#include "kis_layer_container_shape.h"
#include "kis_shape_layer.h"

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

#include "kis_canvas_resource_provider.h"
#include "canvas/kis_canvas2.h"
#include "filter/kis_filter_configuration.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_cursor.h"

struct KisTool::Private {
    Private() : currentPattern(0),
            currentGradient(0),
            currentPaintOpPreset(0),
            currentGenerator(0),
            optionWidget(0) { }
    QCursor cursor; // the cursor that should be shown on tool activation.

    // From the canvas resources
    KisPattern * currentPattern;
    KoAbstractGradient * currentGradient;
    KoColor currentFgColor;
    KoColor currentBgColor;
    QString currentPaintOp;
    KisPaintOpPresetSP currentPaintOpPreset;
    KisNodeSP currentNode;
    float currentExposure;
    KisFilterConfiguration * currentGenerator;
    QWidget* optionWidget;

    KAction* toggleFgBg;
    KAction* resetFgBg;

};

KisTool::KisTool(KoCanvasBase * canvas, const QCursor & cursor)
        : KoTool(canvas)
        , d(new Private)
{
    d->cursor = cursor;
    m_outlinePaintMode = XOR_MODE;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(resetCursorStyle()));

    d->toggleFgBg = new KAction(i18n("Swap Foreground and Background Color"), this);
    d->toggleFgBg->setShortcut(QKeySequence(Qt::Key_X));
    connect(d->toggleFgBg, SIGNAL(triggered()), this, SLOT(slotToggleFgBg()));
    addAction("toggle_fg_bg", d->toggleFgBg);

    d->resetFgBg = new KAction(i18n("Reset Foreground and Background Color"), this);
    d->resetFgBg->setShortcut(QKeySequence(Qt::Key_D));
    connect(d->resetFgBg, SIGNAL(triggered()), this, SLOT(slotResetFgBg()));
    addAction("reset_fg_bg", d->resetFgBg);

}

KisTool::~KisTool()
{
    delete d;
}

void KisTool::activate(bool)
{

    resetCursorStyle();

    d->currentFgColor = m_canvas->resourceProvider()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    d->currentBgColor = m_canvas->resourceProvider()->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
    d->currentPattern = static_cast<KisPattern *>(m_canvas->resourceProvider()->
                        resource(KisCanvasResourceProvider::CurrentPattern).value<void *>());
    d->currentGradient = static_cast<KoAbstractGradient *>(m_canvas->resourceProvider()->
                         resource(KisCanvasResourceProvider::CurrentGradient).value<void *>());


    d->currentPaintOpPreset =
        m_canvas->resourceProvider()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();

    if (d->currentPaintOpPreset && d->currentPaintOpPreset->settings()) {
        d->currentPaintOpPreset->settings()->activate();
    }

    d->currentNode = m_canvas->resourceProvider()->
                     resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
    d->currentExposure = static_cast<float>(m_canvas->resourceProvider()->
                                            resource(KisCanvasResourceProvider::HdrExposure).toDouble());
    d->currentGenerator = static_cast<KisFilterConfiguration*>(m_canvas->resourceProvider()->
                          resource(KisCanvasResourceProvider::CurrentGeneratorConfiguration).value<void *>());
}

void KisTool::deactivate()
{
}

void KisTool::resourceChanged(int key, const QVariant & v)
{

    switch (key) {
    case(KoCanvasResource::ForegroundColor):
        d->currentFgColor = v.value<KoColor>();
        break;
    case(KoCanvasResource::BackgroundColor):
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
            m_canvas->resourceProvider()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
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

QPointF KisTool::viewToPixel(const QPointF &viewCoord)
{
    if (!image())
        return viewCoord;

    return image()->documentToPixel(m_canvas->viewConverter()->viewToDocument(viewCoord));
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

QPointF KisTool::pixelToView(const QPoint &pixelCoord)
{
    if (!image())
        return pixelCoord;
    QPointF documentCoord = image()->pixelToDocument(pixelCoord);
    return m_canvas->viewConverter()->documentToView(documentCoord);
}

QPointF KisTool::pixelToView(const QPointF &pixelCoord)
{
    if (!image())
        return pixelCoord;
    QPointF documentCoord = image()->pixelToDocument(pixelCoord);
    return m_canvas->viewConverter()->documentToView(documentCoord);
}

QRectF KisTool::pixelToView(const QRectF &pixelRect)
{
    if (!image())
        return pixelRect;
    QPointF topLeft = pixelToView(pixelRect.topLeft());
    QPointF bottomRight = pixelToView(pixelRect.bottomRight());
    return QRectF(topLeft, bottomRight);
}

void KisTool::updateCanvasPixelRect(const QRectF &pixelRect)
{
    m_canvas->updateCanvas(convertToPt(pixelRect));
}

void KisTool::updateCanvasViewRect(const QRectF &viewRect)
{
    m_canvas->updateCanvas(m_canvas->viewConverter()->viewToDocument(viewRect));
}

KisImageWSP KisTool::image() const
{
    // For now, krita tools only work in krita, not for a krita shape. Krita shapes are for 2.1
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(m_canvas);
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
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(m_canvas);
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

void KisTool::mousePressEvent(KoPointerEvent *event)
{
    event->ignore();
}

void KisTool::mouseMoveEvent(KoPointerEvent *event)
{
    event->ignore();
}

void KisTool::mouseReleaseEvent(KoPointerEvent *event)
{
    event->ignore();
}

void KisTool::setupPainter(KisPainter* painter)
{
    Q_ASSERT(currentImage());
    if (!currentImage()) return;

    painter->setBounds(currentImage()->bounds());
    painter->setPaintColor(currentFgColor());
    painter->setBackgroundColor(currentBgColor());
    painter->setGenerator(currentGenerator());
    painter->setPattern(currentPattern());
    painter->setGradient(currentGradient());
    painter->setPaintOpPreset(currentPaintOpPreset(), currentImage());

    if (KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(currentNode().data())) {
        painter->setChannelFlags(l->channelFlags());
        if (l->alphaLocked()) {
            painter->setLockAlpha(l->alphaLocked());
        }
    }

}


QWidget* KisTool::createOptionWidget()
{
    d->optionWidget = new QLabel(i18n("No options"));
    d->optionWidget->setObjectName(toolId() + " Option Widget");
    return d->optionWidget;
}

QWidget* KisTool::optionWidget()
{
    return d->optionWidget;
}


void KisTool::paintToolOutline(QPainter* painter, QPainterPath &path)
{
    switch (m_outlinePaintMode) {
    case XOR_MODE: {
        painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painter->setPen(QColor(128, 255, 128));
        painter->drawPath(path);
        break;
    }
    case BW_MODE: {
        QPen pen = painter->pen();
        pen.setWidth(3);
        pen.setColor(QColor(0, 0, 0, 100));
        painter->setPen(pen);
        painter->drawPath(path);
        pen.setWidth(1);
        pen.setColor(Qt::white);
        painter->setPen(pen);
        painter->drawPath(path);
        break;
    }
    default:
        break;
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
    case CURSOR_STYLE_POINTER:
        useCursor(KisCursor::upArrowCursor());
        break;
    case CURSOR_STYLE_NO_CURSOR:
        useCursor(KisCursor::blankCursor());
        break;
#if defined(HAVE_OPENGL)
    case CURSOR_STYLE_3D_MODEL:
        useCursor(d->cursor);
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
    KoCanvasResourceProvider* resourceProvider = canvas()->resourceProvider();
    KoColor c = resourceProvider->foregroundColor();
    resourceProvider->setForegroundColor(resourceProvider->backgroundColor());
    resourceProvider->setBackgroundColor(c);
}

void KisTool::slotResetFgBg()
{
    KoCanvasResourceProvider* resourceProvider = canvas()->resourceProvider();
    resourceProvider->setForegroundColor(KoColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8()));
    resourceProvider->setBackgroundColor(KoColor(Qt::white, KoColorSpaceRegistry::instance()->rgb8()));
}

bool KisTool::isCanvasOpenGL() const
{
    return m_canvas->canvasController()->isCanvasOpenGL();
}

void KisTool::beginOpenGL()
{
#if defined(HAVE_OPENGL)
    KisOpenGLCanvas2 *canvasWidget = dynamic_cast<KisOpenGLCanvas2 *>(m_canvas->canvasWidget());
    Q_ASSERT(canvasWidget);

    if (canvasWidget) {
        canvasWidget->beginOpenGL();
    }
#endif
}

void KisTool::endOpenGL()
{
#if defined(HAVE_OPENGL)
    KisOpenGLCanvas2 *canvasWidget = dynamic_cast<KisOpenGLCanvas2 *>(m_canvas->canvasWidget());
    Q_ASSERT(canvasWidget);

    if (canvasWidget) {
        canvasWidget->endOpenGL();
    }
#endif
}

#include "kis_tool.moc"

