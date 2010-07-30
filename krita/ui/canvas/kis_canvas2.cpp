/* This file is part of the KDE project
 *
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 * Copyright (C) Lukáš Tvrdý <lukast.dev@gmail.com>, (C) 2010
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA..
 */

#include "kis_canvas2.h"

#include <QWidget>
#include <QTime>
#include <QLabel>
#include <QMouseEvent>

#include <kis_debug.h>

#include <KoUnit.h>
#include <KoZoomHandler.h>
#include <KoViewConverter.h>
#include <KoShapeManager.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoCanvasControllerWidget.h>
#include <KoDocument.h>
#include <KoZoomAction.h>
#include <KoToolProxy.h>
#include <KoSelection.h>

#include "kis_prescaled_projection.h"
#include "kis_image.h"
#include "kis_doc2.h"
#include "flake/kis_shape_layer.h"
#include "kis_canvas_resource_provider.h"
#include "kis_view2.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_abstract_canvas_widget.h"
#include "kis_qpainter_canvas.h"
#include "kis_group_layer.h"
#include "flake/kis_shape_controller.h"
#include "kis_layer_manager.h"
#include "kis_selection.h"
#include "kis_selection_component.h"
#include "flake/kis_shape_selection.h"

#include "opengl/kis_opengl_canvas2.h"
#include "opengl/kis_opengl_image_textures.h"
#ifdef HAVE_OPENGL
#include <QGLFormat>
#endif

//Favorite resource Manager
#include <ko_favorite_resource_manager.h>
#include <kis_paintop_box.h>

class KisCanvas2::KisCanvas2Private
{

public:

    KisCanvas2Private(KoCanvasBase * parent, KoViewConverter * viewConverter, KisView2 * view)
        : viewConverter(viewConverter)
        , view(view)
        , canvasWidget(0)
        , shapeManager(new KoShapeManager(parent))
        , monitorProfile(0)
        , currentCanvasIsOpenGL(false)
        , currentCanvasUsesOpenGLShaders(false)
        , toolProxy(new KoToolProxy(parent))
        , favoriteResourceManager(0)
        , mirrorMode(false){
    }

    ~KisCanvas2Private() {
        delete shapeManager;
        delete toolProxy;
    }

    KoViewConverter *viewConverter;
    KisView2 *view;
    KisAbstractCanvasWidget *canvasWidget;
    KoShapeManager *shapeManager;
    KoColorProfile *monitorProfile;
    bool currentCanvasIsOpenGL;
    bool currentCanvasUsesOpenGLShaders;
    KoToolProxy *toolProxy;
    QPoint documentOffset;
    KoShapeControllerBase *sc;
    KoFavoriteResourceManager *favoriteResourceManager;
    bool mirrorMode;
#ifdef HAVE_OPENGL
    KisOpenGLImageTexturesSP openGLImageTextures;
#endif
    KisPrescaledProjectionSP prescaledProjection;
};

KisCanvas2::KisCanvas2(KoViewConverter * viewConverter, KisView2 * view, KoShapeControllerBase * sc)
    : KoCanvasBase(sc)
    , m_d(new KisCanvas2Private(this, viewConverter, view))
{
    KisConfig cfg;
    createCanvas(cfg.useOpenGL());
    connect(view->canvasController()->proxyObject, SIGNAL(moveDocumentOffset(const QPoint&)), SLOT(documentOffsetMoved(const QPoint&)));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    connect(this, SIGNAL(canvasDestroyed(QWidget *)), this, SLOT(slotCanvasDestroyed(QWidget *)));
}

KisCanvas2::~KisCanvas2()
{
    delete m_d;
}

void KisCanvas2::setCanvasWidget(QWidget * widget)
{
    connect(widget, SIGNAL(documentOriginChanged(const QPoint&)), this, SLOT(updateRulers()));

    KisAbstractCanvasWidget * tmp = dynamic_cast<KisAbstractCanvasWidget*>(widget);
    Q_ASSERT_X(tmp, "setCanvasWidget", "Cannot cast the widget to a KisAbstractCanvasWidget");
    emit canvasDestroyed(widget);

    if(m_d->canvasWidget!=0)
        tmp->setDecorations(m_d->canvasWidget->decorations());
    m_d->canvasWidget = tmp;

    widget->setAutoFillBackground(false);
    widget->setAttribute(Qt::WA_OpaquePaintEvent);
    widget->setMouseTracking(true);
    widget->setAcceptDrops(true);
    KoCanvasControllerWidget *controller = dynamic_cast<KoCanvasControllerWidget*>(canvasController());
    if (controller) {
        Q_ASSERT(controller->canvas() == this);
        // Avoids jumping and redrawing when changing zoom means the image fits in the area completely
        controller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        controller->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        controller->changeCanvasWidget(widget);
    }
}

bool KisCanvas2::canvasIsOpenGL()
{
    return m_d->currentCanvasIsOpenGL;
}

void KisCanvas2::gridSize(qreal *horizontal, qreal *vertical) const
{
    Q_ASSERT(horizontal);
    Q_ASSERT(vertical);
    *horizontal = m_d->view->document()->gridData().gridX();
    *vertical = m_d->view->document()->gridData().gridY();
}

bool KisCanvas2::snapToGrid() const
{
    return m_d->view->document()->gridData().snapToGrid();
}

bool KisCanvas2::isCanvasMirrored()
{
    return m_d->mirrorMode;
}

void KisCanvas2::mirrorCanvas(bool enable)
{
    m_d->mirrorMode = enable;
    updateCanvas();
}

void KisCanvas2::addCommand(QUndoCommand *command)
{
    m_d->view->koDocument()->addCommand(command);
}

void KisCanvas2::startMacro(const QString &title)
{
    m_d->view->koDocument()->beginMacro(title);
}

void KisCanvas2::stopMacro()
{
    m_d->view->koDocument()->endMacro();
}

KoShapeManager* KisCanvas2::shapeManager() const
{
    if (!m_d->view) return m_d->shapeManager;
    if (!m_d->view->layerManager()) return m_d->shapeManager;

    KisLayerSP activeLayer = m_d->view->layerManager()->activeLayer();
    if (activeLayer) {
        KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(activeLayer.data());
        if (shapeLayer) {
            return shapeLayer->shapeManager();
        }
        if (activeLayer->selection() && activeLayer->selection()->hasShapeSelection()) {
            KoShapeManager* m = static_cast<KisShapeSelection*>(activeLayer->selection()->shapeSelection())->shapeManager();
            return m;

        }
    }
    return m_d->shapeManager;
}

KoShapeManager * KisCanvas2::globalShapeManager() const
{
    return m_d->shapeManager;
}

void KisCanvas2::updateInputMethodInfo()
{
    // TODO call (the protected) QWidget::updateMicroFocus() on the proper canvas widget...
}

const KoViewConverter* KisCanvas2::viewConverter() const
{
    return m_d->viewConverter;
}

QWidget* KisCanvas2::canvasWidget()
{
    return m_d->canvasWidget->widget();
}

const QWidget* KisCanvas2::canvasWidget() const
{
    return m_d->canvasWidget->widget();
}


KoUnit KisCanvas2::unit() const
{
    return KoUnit(KoUnit::Pixel);
}

KoToolProxy * KisCanvas2::toolProxy() const
{
    return m_d->toolProxy;
}

void KisCanvas2::createQPainterCanvas()
{
    m_d->currentCanvasIsOpenGL = false;

    KisQPainterCanvas * canvasWidget = new KisQPainterCanvas(this, m_d->view);
    m_d->prescaledProjection = new KisPrescaledProjection();
    m_d->prescaledProjection->setViewConverter(m_d->viewConverter);
    m_d->prescaledProjection->setMonitorProfile(monitorProfile());
    canvasWidget->setPrescaledProjection(m_d->prescaledProjection);
    setCanvasWidget(canvasWidget);
}

void KisCanvas2::createOpenGLCanvas()
{
#ifdef HAVE_OPENGL
    m_d->currentCanvasIsOpenGL = true;

    // XXX: The image isn't done loading here!
    m_d->openGLImageTextures = KisOpenGLImageTextures::getImageTextures(m_d->view->image(), m_d->monitorProfile);
    KisOpenGLCanvas2 * canvasWidget = new KisOpenGLCanvas2(this, m_d->view, m_d->openGLImageTextures);
    m_d->currentCanvasUsesOpenGLShaders = m_d->openGLImageTextures->usingHDRExposureProgram();
    setCanvasWidget(canvasWidget);
#else
    qFatal("Bad use of createOpenGLCanvas(). It shouldn't have happened =(");
#endif
}

void KisCanvas2::createCanvas(bool useOpenGL)
{
    KisConfig cfg;
    slotSetDisplayProfile(KoColorSpaceRegistry::instance()->profileByName(cfg.monitorProfile()));

    if (useOpenGL) {
#ifdef HAVE_OPENGL
        if (QGLFormat::hasOpenGL()) {
            createOpenGLCanvas();
        } else {
            warnKrita << "Tried to create OpenGL widget when system doesn't have OpenGL\n";
            createQPainterCanvas();
        }
#else
        warnKrita << "OpenGL requested while its not available, starting qpainter canvas";
        createQPainterCanvas();
#endif
    } else {
#ifdef HAVE_OPENGL
        // Free shared pointer
        m_d->openGLImageTextures = 0;
#endif
        createQPainterCanvas();
    }
}

void KisCanvas2::connectCurrentImage()
{

    if (m_d->currentCanvasIsOpenGL) {
#ifdef HAVE_OPENGL
        Q_ASSERT(m_d->openGLImageTextures);

        connect(m_d->view->image(), SIGNAL(sigSizeChanged(qint32, qint32)),
                m_d->openGLImageTextures, SLOT(slotImageSizeChanged(qint32, qint32)));
#else
        qFatal("Bad use of connectCurrentImage(). It shouldn't have happened =(");
#endif
    } else {
        connect(m_d->view->image(), SIGNAL(sigSizeChanged(qint32, qint32)),
                SLOT(setImageSize(qint32, qint32)));

        Q_ASSERT(m_d->prescaledProjection);
        m_d->prescaledProjection->setImage(m_d->view->image());

    }

    connect(m_d->view->image(), SIGNAL(sigImageUpdated(const QRect &)),
            SLOT(startUpdateCanvasProjection(const QRect &)),
            Qt::DirectConnection);
    connect(this, SIGNAL(sigCanvasCacheUpdated(KisUpdateInfoSP)),
            this, SLOT(updateCanvasProjection(KisUpdateInfoSP)));

    emit imageChanged(m_d->view->image());
}

void KisCanvas2::disconnectCurrentImage()
{
    if (m_d->currentCanvasIsOpenGL) {
#ifdef HAVE_OPENGL
        Q_ASSERT(m_d->openGLImageTextures);
        m_d->openGLImageTextures->disconnect(this);
        m_d->openGLImageTextures->disconnect(m_d->view->image());
#else
        qFatal("Bad use of disconnectCurrentImage(). It shouldn't have happened =(");
#endif
    }

    disconnect(SIGNAL(sigCanvasCacheUpdated(KisUpdateInfoSP)));

    // for sigSizeChanged()
    m_d->view->image()->disconnect(this);
}

void KisCanvas2::resetCanvas(bool useOpenGL)
{
#ifdef HAVE_OPENGL
    KisConfig cfg;

    if (   (useOpenGL != m_d->currentCanvasIsOpenGL)
        || (   m_d->currentCanvasIsOpenGL
               && (cfg.useOpenGLShaders() != m_d->currentCanvasUsesOpenGLShaders))) {

        disconnectCurrentImage();
        createCanvas(useOpenGL);
        connectCurrentImage();
    }

    if (useOpenGL) {
        Q_ASSERT(m_d->openGLImageTextures);
        m_d->openGLImageTextures->setMonitorProfile(monitorProfile());
    } else {
        if (image()) {
            startUpdateCanvasProjection(image()->bounds());
        }
    }

#endif
    m_d->canvasWidget->widget()->update();
}

void KisCanvas2::startUpdateCanvasProjection(const QRect & rc)
{
    if (m_d->currentCanvasIsOpenGL) {
#ifdef HAVE_OPENGL
        Q_ASSERT(m_d->openGLImageTextures);
        KisUpdateInfoSP info = m_d->openGLImageTextures->updateCache(rc);

        emit sigCanvasCacheUpdated(info);
#else
        Q_ASSERT_X(0, "startUpdateCanvasProjection()", "Bad use of startUpdateCanvasProjection(). It shouldn't have happened =(");
#endif
    } else {
        Q_ASSERT(m_d->prescaledProjection);
        KisUpdateInfoSP info = m_d->prescaledProjection->updateCache(rc);

        emit sigCanvasCacheUpdated(info);
    }


}

void KisCanvas2::updateCanvasProjection(KisUpdateInfoSP info)
{
    if (m_d->currentCanvasIsOpenGL) {
#ifdef HAVE_OPENGL
        Q_ASSERT(m_d->openGLImageTextures);
        m_d->openGLImageTextures->recalculateCache(info);

        /**
         * FIXME: Please not update entire canvas
         * Implement info->dirtyViewportRect()
         */
        m_d->canvasWidget->widget()->update();
#else
        Q_ASSERT_X(0, "updateCanvasProjection()", "Bad use of updateCanvasProjection(). It shouldn't have happened =(");
#endif
    }
    else {

        // See comment in startUpdateCanvasProjection()
        Q_ASSERT(m_d->prescaledProjection);

        m_d->prescaledProjection->recalculateCache(info);

        QRect vRect = info->dirtyViewportRect();
        if (m_d->mirrorMode){
            m_d->canvasWidget->widget()->update();
        }else if (!vRect.isEmpty()) {
            vRect.translate(m_d->canvasWidget->documentOrigin());
            m_d->canvasWidget->widget()->update(vRect);
        }
    }
}

void KisCanvas2::updateCanvas()
{
    m_d->canvasWidget->widget()->update();
}

void KisCanvas2::updateCanvas(const QRectF& rc)
{
    // updateCanvas is called from tools, never from the projection
    // updates, so no need to prescale!
    QRect vRect = viewRectFromDoc(rc);
    if (!vRect.isEmpty()) {
        m_d->canvasWidget->widget()->update(vRect);
    }
}

void KisCanvas2::preScale()
{
    if (!m_d->currentCanvasIsOpenGL) {
        Q_ASSERT(m_d->prescaledProjection);
        m_d->prescaledProjection->preScale();
    }
}

QRect KisCanvas2::viewRectFromDoc(const QRectF & rc)
{
    QRect viewRect = m_d->viewConverter->documentToView(rc).toAlignedRect();
    viewRect = viewRect.translated(-m_d->documentOffset);
    // comment out this line if you want to see the preview outside of the canvas
    // viewRect = viewRect.intersected(QRect(0, 0, m_d->canvasWidget->widget()->width(), m_d->canvasWidget->widget()->height()));
    viewRect.translate(documentOrigin());
    return viewRect;
}

KoColorProfile *  KisCanvas2::monitorProfile()
{
    return m_d->monitorProfile;
}

KisView2* KisCanvas2::view()
{
    return m_d->view;
}

KisImageWSP KisCanvas2::image()
{
    return m_d->view->image();

}

KisImageWSP KisCanvas2::currentImage()
{
    return m_d->view->image();
}

void KisCanvas2::setImageSize(qint32 w, qint32 h)
{
    if (m_d->prescaledProjection)
        m_d->prescaledProjection->setImageSize(w, h);
}

void KisCanvas2::documentOffsetMoved(const QPoint &documentOffset)
{
    m_d->documentOffset = documentOffset;
    m_d->canvasWidget->documentOffsetMoved(documentOffset);
}

bool KisCanvas2::usingHDRExposureProgram()
{
#ifdef HAVE_OPENGL
    if (m_d->currentCanvasIsOpenGL) {
        if (m_d->openGLImageTextures->usingHDRExposureProgram()) {
            return true;
        }
    }
#endif
    return false;
}

void KisCanvas2::slotConfigChanged()
{
    // first, assume we're going to crash when switching to opengl
    KisConfig cfg;
    bool useOpenGL = cfg.useOpenGL();
    if (cfg.canvasState() == "TRY_OPENGL" && useOpenGL) {
        cfg.setCanvasState("OPENGL_FAILED");
    }
    resetCanvas(useOpenGL);
    if (useOpenGL) {
        cfg.setCanvasState("OPENGL_SUCCESS");
    }
}

void KisCanvas2::slotSetDisplayProfile(const KoColorProfile * profile)
{
    m_d->monitorProfile = const_cast<KoColorProfile*>(profile);
}

void KisCanvas2::addDecoration(KisCanvasDecoration* deco)
{
    m_d->canvasWidget->addDecoration(deco);
}

KisCanvasDecoration* KisCanvas2::decoration(const QString& id)
{
    return m_d->canvasWidget->decoration(id);
}


QPoint KisCanvas2::documentOrigin() const
{
    return m_d->canvasWidget->documentOrigin();
}


void KisCanvas2::adjustOrigin()
{
    m_d->canvasWidget->adjustOrigin();
}


void KisCanvas2::updateRulers()
{
    emit documentOriginChanged();
}


QPoint KisCanvas2::documentOffset() const
{
    return m_d->documentOffset;
}

void KisCanvas2::createFavoriteResourceManager(KisPaintopBox* paintopbox)
{
    m_d->favoriteResourceManager = new KoFavoriteResourceManager(paintopbox, canvasWidget());
    connect(this, SIGNAL(favoritePaletteCalled(const QPoint&)), favoriteResourceManager(), SLOT(slotShowPopupPalette(const QPoint&)));
    connect(view()->resourceProvider(), SIGNAL(sigFGColorUsed(KoColor)), favoriteResourceManager(), SLOT(slotAddRecentColor(KoColor)));
    connect(view()->resourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), favoriteResourceManager(), SLOT(slotChangeFGColorSelector(KoColor)));
    connect(favoriteResourceManager(), SIGNAL(sigSetFGColor(KoColor)), view()->resourceProvider(), SLOT(slotSetFGColor(KoColor)));
    connect(favoriteResourceManager(), SIGNAL(sigEnableChangeColor(bool)), view()->resourceProvider(), SLOT(slotResetEnableFGChange(bool)));
}

void KisCanvas2::slotCanvasDestroyed(QWidget* w)
{
    if (m_d->favoriteResourceManager != 0)
    {
        m_d->favoriteResourceManager->resetPopupPaletteParent(w);
    }
}

KoFavoriteResourceManager* KisCanvas2::favoriteResourceManager()
{
    return m_d->favoriteResourceManager;
}


bool KisCanvas2::handlePopupPaletteIsVisible(KoPointerEvent* e)
{
    e->accept();

    //TODO: port favoriteResourceManager to KisCanvas2, then enable this function.
    if (e->button() && (Qt::LeftButton || Qt::MidButton || Qt::RightButton)
        && favoriteResourceManager()->isPopupPaletteVisible()) {
        favoriteResourceManager()->slotShowPopupPalette();
        return true;
    }
    return false;
}

#include "kis_canvas2.moc"
