/* This file is part of the KDE project
 *
 * Copyright (C) 2006, 2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) Lukáš Tvrdý <lukast.dev@gmail.com>, (C) 2010
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTime>
#include <QLabel>
#include <QMouseEvent>
#include <QDesktopWidget>

#include <kis_debug.h>

#include <KoUnit.h>
#include <KoShapeManager.h>
#include <KoColorProfile.h>
#include <KoCanvasControllerWidget.h>
#include <KisDocument.h>
#include <KoSelection.h>

#include "kis_tool_proxy.h"
#include "kis_coordinates_converter.h"
#include "kis_prescaled_projection.h"
#include "kis_image.h"
#include "kis_image_barrier_locker.h"
#include "KisDocument.h"
#include "flake/kis_shape_layer.h"
#include "kis_canvas_resource_provider.h"
#include "KisViewManager.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_abstract_canvas_widget.h"
#include "kis_qpainter_canvas.h"
#include "kis_group_layer.h"
#include "flake/kis_shape_controller.h"
#include "kis_node_manager.h"
#include "kis_selection.h"
#include "kis_selection_component.h"
#include "flake/kis_shape_selection.h"
#include "kis_image_config.h"
#include "kis_infinity_manager.h"
#include "kis_signal_compressor.h"
#include "kis_display_color_converter.h"
#include "kis_exposure_gamma_correction_interface.h"
#include "KisView.h"
#include "kis_canvas_controller.h"
#include "kis_animation_player.h"
#include "kis_animation_frame_cache.h"

#ifdef HAVE_OPENGL
#include "opengl/kis_opengl_canvas2.h"
#endif

#include "opengl/kis_opengl.h"


#include <kis_favorite_resource_manager.h>
#include <kis_popup_palette.h>

#include "input/kis_input_manager.h"
#include "kis_painting_assistants_decoration.h"

#include "kis_canvas_updates_compressor.h"

class Q_DECL_HIDDEN KisCanvas2::KisCanvas2Private
{

public:

    KisCanvas2Private(KoCanvasBase *parent, KisCoordinatesConverter* coordConverter, QPointer<KisView> view, KoCanvasResourceManager* resourceManager)
        : coordinatesConverter(coordConverter)
        , view(view)
        , shapeManager(parent)
        , toolProxy(parent)
        , displayColorConverter(resourceManager, view)
    {
    }

    KisCoordinatesConverter *coordinatesConverter;
    QPointer<KisView>view;
    KisAbstractCanvasWidget *canvasWidget = 0;
    KoShapeManager shapeManager;
    bool currentCanvasIsOpenGL;
#ifdef HAVE_OPENGL
    int openGLFilterMode;
#endif
    KisToolProxy toolProxy;
    KisPrescaledProjectionSP prescaledProjection;
    bool vastScrolling;

    KisSignalCompressor updateSignalCompressor;
    QRect savedUpdateRect;

    QBitArray channelFlags;

    KisPopupPalette *popupPalette = 0;
    KisDisplayColorConverter displayColorConverter;

    KisCanvasUpdatesCompressor projectionUpdatesCompressor;

    KisAnimationPlayer *animationPlayer;
    KisAnimationFrameCacheSP frameCache;

    bool lodAllowedInCanvas;
    bool bootsrapLodBlocked;

    bool effectiveLodAllowedInCanvas() {
        return lodAllowedInCanvas && !bootsrapLodBlocked;
    }
};

KisCanvas2::KisCanvas2(KisCoordinatesConverter *coordConverter, KoCanvasResourceManager *resourceManager, KisView *view, KoShapeBasedDocumentBase *sc)
    : KoCanvasBase(sc, resourceManager)
    , m_d(new KisCanvas2Private(this, coordConverter, view, resourceManager))
{
    /**
     * While loading LoD should be blocked. Only when GUI has finished
     * loading and zoom level settled down, LoD is given a green
     * light.
     */
    m_d->bootsrapLodBlocked = true;
    connect(view->mainWindow(), SIGNAL(guiLoadingFinished()),
            SLOT(bootstrapFinished()));

}

void KisCanvas2::setup(KisShapeController *kritaShapeController, KisView *view)
{
    // a bit of duplication from slotConfigChanged()
    KisConfig cfg;
    m_d->vastScrolling = cfg.vastScrolling();
    m_d->lodAllowedInCanvas = cfg.levelOfDetailEnabled();

    createCanvas(cfg.useOpenGL());

    setLodAllowedInCanvas(m_d->lodAllowedInCanvas);

    m_d->animationPlayer = new KisAnimationPlayer(this);

    connect(view->canvasController()->proxyObject, SIGNAL(moveDocumentOffset(QPoint)), SLOT(documentOffsetMoved(QPoint)));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));

    /**
     * We switch the shape manager every time vector layer or
     * shape selection is activated. Flake does not expect this
     * and connects all the signals of the global shape manager
     * to the clients in the constructor. To workaround this we
     * forward the signals of local shape managers stored in the
     * vector layers to the signals of global shape manager. So the
     * sequence of signal deliveries is the following:
     *
     * shapeLayer.m_d.canvas.m_shapeManager.selection() ->
     * shapeLayer ->
     * shapeController ->
     * globalShapeManager.selection()
     */

    connect(kritaShapeController, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
    connect(kritaShapeController, SIGNAL(selectionContentChanged()),
            globalShapeManager(), SIGNAL(selectionContentChanged()));
    connect(kritaShapeController, SIGNAL(currentLayerChanged(const KoShapeLayer*)),
            globalShapeManager()->selection(), SIGNAL(currentLayerChanged(const KoShapeLayer*)));


    m_d->updateSignalCompressor.setDelay(10);
    m_d->updateSignalCompressor.setMode(KisSignalCompressor::FIRST_ACTIVE);
    connect(&m_d->updateSignalCompressor, SIGNAL(timeout()), SLOT(slotDoCanvasUpdate()));
}

KisCanvas2::~KisCanvas2()
{
    delete m_d;
}

void KisCanvas2::setCanvasWidget(QWidget * widget)
{
    KisAbstractCanvasWidget *tmp = dynamic_cast<KisAbstractCanvasWidget*>(widget);
    Q_ASSERT_X(tmp, "setCanvasWidget", "Cannot cast the widget to a KisAbstractCanvasWidget");
    if (m_d->popupPalette) {
        m_d->popupPalette->setParent(widget);
    }

    if(m_d->canvasWidget != 0)
    {
        tmp->setDecorations(m_d->canvasWidget->decorations());

        // Redundant check for the constructor case, see below
        if(viewManager() != 0)
            viewManager()->inputManager()->removeTrackedCanvas(this);
    }

    m_d->canvasWidget = tmp;

    // Either tmp was null or we are being called by KisCanvas2 constructor that is called by KisView
    // constructor, so the view manager still doesn't exists.
    if(m_d->canvasWidget != 0 && viewManager() != 0)
        viewManager()->inputManager()->addTrackedCanvas(this);

    if (!m_d->canvasWidget->decoration(INFINITY_DECORATION_ID)) {
        KisInfinityManager *manager = new KisInfinityManager(m_d->view, this);
        manager->setVisible(true);
        m_d->canvasWidget->addDecoration(manager);
    }

    widget->setAutoFillBackground(false);
    widget->setAttribute(Qt::WA_OpaquePaintEvent);
    widget->setMouseTracking(true);
    widget->setAcceptDrops(true);

    KoCanvasControllerWidget *controller = dynamic_cast<KoCanvasControllerWidget*>(canvasController());
    if (controller) {
        Q_ASSERT(controller->canvas() == this);
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
    QTransform transform = coordinatesConverter()->imageToDocumentTransform();

    QPointF size = transform.map(QPointF(m_d->view->document()->gridData().gridX(), m_d->view->document()->gridData().gridY()));
    *horizontal = size.x();
    *vertical = size.y();
}

bool KisCanvas2::snapToGrid() const
{
    return m_d->view->document()->gridData().snapToGrid();
}

qreal KisCanvas2::rotationAngle() const
{
    return m_d->coordinatesConverter->rotationAngle();
}

bool KisCanvas2::xAxisMirrored() const
{
    return m_d->coordinatesConverter->xAxisMirrored();
}

bool KisCanvas2::yAxisMirrored() const
{
    return m_d->coordinatesConverter->yAxisMirrored();
}

void KisCanvas2::channelSelectionChanged()
{
    KisImageWSP image = this->image();
    m_d->channelFlags = image->rootLayer()->channelFlags();

    image->barrierLock();
    m_d->canvasWidget->channelSelectionChanged(m_d->channelFlags);
    startUpdateInPatches(image->bounds());

    image->unlock();

}

void KisCanvas2::addCommand(KUndo2Command *command)
{
    // This method exists to support flake-related operations
    m_d->view->document()->addCommand(command);
}

KoShapeManager* KisCanvas2::shapeManager() const
{
    if (!viewManager()) return globalShapeManager();
    if (!viewManager()->nodeManager()) return globalShapeManager();

    KisLayerSP activeLayer = viewManager()->nodeManager()->activeLayer();
    if (activeLayer && activeLayer->isEditable()) {
        KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(activeLayer.data());
        if (shapeLayer) {
            return shapeLayer->shapeManager();
        }
        KisSelectionSP selection = activeLayer->selection();
        if (selection && !selection.isNull()) {
            if (selection->hasShapeSelection()) {
                KoShapeManager* m = dynamic_cast<KisShapeSelection*>(selection->shapeSelection())->shapeManager();
                return m;
            }

        }
    }
    return globalShapeManager();
}

KoShapeManager * KisCanvas2::globalShapeManager() const
{
    return &m_d->shapeManager;
}

void KisCanvas2::updateInputMethodInfo()
{
    // TODO call (the protected) QWidget::updateMicroFocus() on the proper canvas widget...
}

const KisCoordinatesConverter* KisCanvas2::coordinatesConverter() const
{
    return m_d->coordinatesConverter;
}

KoViewConverter* KisCanvas2::viewConverter() const
{
    return m_d->coordinatesConverter;
}

KisInputManager* KisCanvas2::globalInputManager() const
{
    return m_d->view->globalInputManager();
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
    KoUnit unit(KoUnit::Pixel);

    KisImageWSP image = m_d->view->image();
    if (image) {
        if (!qFuzzyCompare(image->xRes(), image->yRes())) {
            warnKrita << "WARNING: resolution of the image is anisotropic"
                       << ppVar(image->xRes())
                       << ppVar(image->yRes());
        }

        const qreal resolution = image->xRes();
        unit.setFactor(resolution);
    }

    return unit;
}

KoToolProxy * KisCanvas2::toolProxy() const
{
    return &m_d->toolProxy;
}

void KisCanvas2::createQPainterCanvas()
{
    m_d->currentCanvasIsOpenGL = false;

    KisQPainterCanvas * canvasWidget = new KisQPainterCanvas(this, m_d->coordinatesConverter, m_d->view);
    m_d->prescaledProjection = new KisPrescaledProjection();
    m_d->prescaledProjection->setCoordinatesConverter(m_d->coordinatesConverter);
    m_d->prescaledProjection->setMonitorProfile(m_d->displayColorConverter.monitorProfile(),
                                                m_d->displayColorConverter.renderingIntent(),
                                                m_d->displayColorConverter.conversionFlags());
    m_d->prescaledProjection->setDisplayFilter(m_d->displayColorConverter.displayFilter());
    canvasWidget->setPrescaledProjection(m_d->prescaledProjection);
    setCanvasWidget(canvasWidget);
}

void KisCanvas2::createOpenGLCanvas()
{
#ifdef HAVE_OPENGL
    KisConfig cfg;
    m_d->openGLFilterMode = cfg.openGLFilteringMode();
    m_d->currentCanvasIsOpenGL = true;

    KisOpenGLCanvas2 *canvasWidget = new KisOpenGLCanvas2(this, m_d->coordinatesConverter, 0, m_d->view->image(), &m_d->displayColorConverter);
    m_d->frameCache = KisAnimationFrameCache::getFrameCache(canvasWidget->openGLImageTextures());

    setCanvasWidget(canvasWidget);
#else
    qFatal("Bad use of createOpenGLCanvas(). It shouldn't have happened =(");
#endif
}

void KisCanvas2::createCanvas(bool useOpenGL)
{
    KisConfig cfg;
    QDesktopWidget dw;
    const KoColorProfile *profile = cfg.displayProfile(dw.screenNumber(imageView()));
    m_d->displayColorConverter.setMonitorProfile(profile);

    if (useOpenGL) {
#ifdef HAVE_OPENGL
        if (KisOpenGL::hasOpenGL()) {
            createOpenGLCanvas();
            if (cfg.canvasState() == "OPENGL_FAILED") {
                // Creating the opengl canvas failed, fall back
                warnKrita << "OpenGL Canvas initialization returned OPENGL_FAILED. Falling back to QPainter.";
                createQPainterCanvas();
            }
        } else {
            warnKrita << "Tried to create OpenGL widget when system doesn't have OpenGL\n";
            createQPainterCanvas();
        }
#else
        warnKrita << "User requested an OpenGL canvas, but Krita was compiled without OpenGL support. Falling back to QPainter.";
        createQPainterCanvas();
#endif
    } else {
        createQPainterCanvas();
    }
    if (m_d->popupPalette) {
        m_d->popupPalette->setParent(m_d->canvasWidget->widget());
    }

}

void KisCanvas2::initializeImage()
{
    KisImageWSP image = m_d->view->image();

    m_d->coordinatesConverter->setImage(image);

    connect(image, SIGNAL(sigImageUpdated(QRect)), SLOT(startUpdateCanvasProjection(QRect)), Qt::DirectConnection);
    connect(this, SIGNAL(sigCanvasCacheUpdated()), SLOT(updateCanvasProjection()));
    connect(image, SIGNAL(sigSizeChanged(const QPointF&, const QPointF&)), SLOT(startResizingImage()), Qt::DirectConnection);
    connect(this, SIGNAL(sigContinueResizeImage(qint32,qint32)), SLOT(finishResizingImage(qint32,qint32)));

    connectCurrentCanvas();
}

void KisCanvas2::connectCurrentCanvas()
{
    KisImageWSP image = m_d->view->image();

    if (!m_d->currentCanvasIsOpenGL) {
        Q_ASSERT(m_d->prescaledProjection);
        m_d->prescaledProjection->setImage(image);
    }

    startResizingImage();

    emit imageChanged(image);
    setLodAllowedInCanvas(m_d->lodAllowedInCanvas);
}

void KisCanvas2::disconnectCurrentCanvas()
{
    m_d->canvasWidget->disconnectCurrentCanvas();
}

void KisCanvas2::resetCanvas(bool useOpenGL)
{
    // we cannot reset the canvas before it's created, but this method might be called,
    // for instance when setting the monitor profile.
    if (!m_d->canvasWidget) {
        return;
    }
#ifdef HAVE_OPENGL
    KisConfig cfg;
    bool needReset = (m_d->currentCanvasIsOpenGL != useOpenGL) ||
        (m_d->currentCanvasIsOpenGL &&
         m_d->openGLFilterMode != cfg.openGLFilteringMode());

    if (needReset) {
        disconnectCurrentCanvas();
        createCanvas(useOpenGL);
        connectCurrentCanvas();
        notifyZoomChanged();
    }
#else
    Q_UNUSED(useOpenGL)
#endif

    updateCanvasWidgetImpl();
}

void KisCanvas2::startUpdateInPatches(const QRect &imageRect)
{
    if (m_d->currentCanvasIsOpenGL) {
        startUpdateCanvasProjection(imageRect);
    } else {
        KisImageConfig imageConfig;
        int patchWidth = imageConfig.updatePatchWidth();
        int patchHeight = imageConfig.updatePatchHeight();

        for (int y = 0; y < imageRect.height(); y += patchHeight) {
            for (int x = 0; x < imageRect.width(); x += patchWidth) {
                QRect patchRect(x, y, patchWidth, patchHeight);
                startUpdateCanvasProjection(patchRect);
            }
        }
    }
}

void KisCanvas2::setDisplayFilter(KisDisplayFilter *displayFilter)
{
    m_d->displayColorConverter.setDisplayFilter(displayFilter);
    KisImageWSP image = this->image();

    image->barrierLock();

    m_d->canvasWidget->setDisplayFilter(displayFilter);

    image->unlock();
}

KisDisplayFilter *KisCanvas2::displayFilter() const
{
    return m_d->displayColorConverter.displayFilter();
}

KisDisplayColorConverter* KisCanvas2::displayColorConverter() const
{
    return &m_d->displayColorConverter;
}

KisExposureGammaCorrectionInterface* KisCanvas2::exposureGammaCorrectionInterface() const
{
    KisDisplayFilter *displayFilter =
        m_d->displayColorConverter.displayFilter();

    return displayFilter ?
        displayFilter->correctionInterface() :
        KisDumbExposureGammaCorrectionInterface::instance();
}

void KisCanvas2::startResizingImage()
{
    KisImageWSP image = this->image();
    qint32 w = image->width();
    qint32 h = image->height();

    emit sigContinueResizeImage(w, h);

    QRect imageBounds(0, 0, w, h);
    startUpdateInPatches(imageBounds);
}

void KisCanvas2::finishResizingImage(qint32 w, qint32 h)
{
    m_d->canvasWidget->finishResizingImage(w, h);
}

void KisCanvas2::startUpdateCanvasProjection(const QRect & rc)
{
    KisUpdateInfoSP info = m_d->canvasWidget->startUpdateCanvasProjection(rc, m_d->channelFlags);
    if (m_d->projectionUpdatesCompressor.putUpdateInfo(info)) {
        emit sigCanvasCacheUpdated();
    }
}

void KisCanvas2::updateCanvasProjection()
{
    while (KisUpdateInfoSP info = m_d->projectionUpdatesCompressor.takeUpdateInfo()) {
        QRect vRect = m_d->canvasWidget->updateCanvasProjection(info);
        if (!vRect.isEmpty()) {
            updateCanvasWidgetImpl(m_d->coordinatesConverter->viewportToWidget(vRect).toAlignedRect());
        }
    }

    // TODO: Implement info->dirtyViewportRect() for KisOpenGLCanvas2 to avoid updating whole canvas
    if (m_d->currentCanvasIsOpenGL) {
        updateCanvasWidgetImpl();
    }
}

void KisCanvas2::slotDoCanvasUpdate()
{
    if (m_d->canvasWidget->isBusy()) {
        // just restarting the timer
        updateCanvasWidgetImpl(m_d->savedUpdateRect);
        return;
    }

    if (m_d->savedUpdateRect.isEmpty()) {
        m_d->canvasWidget->widget()->update();
        emit updateCanvasRequested(m_d->canvasWidget->widget()->rect());
    } else {
        emit updateCanvasRequested(m_d->savedUpdateRect);
        m_d->canvasWidget->widget()->update(m_d->savedUpdateRect);
    }

    m_d->savedUpdateRect = QRect();
}

void KisCanvas2::updateCanvasWidgetImpl(const QRect &rc)
{
    if (!m_d->updateSignalCompressor.isActive() ||
        !m_d->savedUpdateRect.isEmpty()) {
        m_d->savedUpdateRect |= rc;
    }
    m_d->updateSignalCompressor.start();
}

void KisCanvas2::updateCanvas()
{
    updateCanvasWidgetImpl();
}

void KisCanvas2::updateCanvas(const QRectF& documentRect)
{
    if (m_d->currentCanvasIsOpenGL && m_d->canvasWidget->decorations().size() > 0) {
        updateCanvasWidgetImpl();
    }
    else {
        // updateCanvas is called from tools, never from the projection
        // updates, so no need to prescale!
        QRect widgetRect = m_d->coordinatesConverter->documentToWidget(documentRect).toAlignedRect();
        widgetRect.adjust(-2, -2, 2, 2);
        if (!widgetRect.isEmpty()) {
            updateCanvasWidgetImpl(widgetRect);
        }
    }
}

void KisCanvas2::disconnectCanvasObserver(QObject *object)
{
    KoCanvasBase::disconnectCanvasObserver(object);
    m_d->view->disconnect(object);
}

void KisCanvas2::notifyZoomChanged()
{
    if (!m_d->currentCanvasIsOpenGL) {
        Q_ASSERT(m_d->prescaledProjection);
        m_d->prescaledProjection->notifyZoomChanged();
    }

    notifyLevelOfDetailChange();
    updateCanvas(); // update the canvas, because that isn't done when zooming using KoZoomAction
}

void KisCanvas2::notifyLevelOfDetailChange()
{
    if (!m_d->effectiveLodAllowedInCanvas()) return;

    KisImageSP image = this->image();

    const qreal effectiveZoom = m_d->coordinatesConverter->effectiveZoom();

    KisConfig cfg;
    const int maxLod = cfg.numMipmapLevels();

    int lod = KisLodTransform::scaleToLod(effectiveZoom, maxLod);
    image->setDesiredLevelOfDetail(lod);
}

void KisCanvas2::preScale()
{
    if (!m_d->currentCanvasIsOpenGL) {
        Q_ASSERT(m_d->prescaledProjection);
        m_d->prescaledProjection->preScale();
    }
}

const KoColorProfile *  KisCanvas2::monitorProfile()
{
    return m_d->displayColorConverter.monitorProfile();
}

KisViewManager* KisCanvas2::viewManager() const
{
    if (m_d->view) {
        return m_d->view->viewManager();
    }
    return 0;
}

QPointer<KisView>KisCanvas2::imageView() const
{
    return m_d->view;
}

KisImageWSP KisCanvas2::image() const
{
    return m_d->view->image();

}

KisImageWSP KisCanvas2::currentImage() const
{
    return m_d->view->image();
}

void KisCanvas2::documentOffsetMoved(const QPoint &documentOffset)
{
    QPointF offsetBefore = m_d->coordinatesConverter->imageRectInViewportPixels().topLeft();
    m_d->coordinatesConverter->setDocumentOffset(documentOffset);
    QPointF offsetAfter = m_d->coordinatesConverter->imageRectInViewportPixels().topLeft();

    QPointF moveOffset = offsetAfter - offsetBefore;

    if (!m_d->currentCanvasIsOpenGL)
        m_d->prescaledProjection->viewportMoved(moveOffset);

    emit documentOffsetUpdateFinished();

    updateCanvas();
}

void KisCanvas2::slotConfigChanged()
{
    KisConfig cfg;
    m_d->vastScrolling = cfg.vastScrolling();

    resetCanvas(cfg.useOpenGL());
    slotSetDisplayProfile(cfg.displayProfile(QApplication::desktop()->screenNumber(this->canvasWidget())));

}

void KisCanvas2::refetchDataFromImage()
{
    KisImageSP image = this->image();
    KisImageBarrierLocker l(image);
    startUpdateInPatches(image->bounds());
}

void KisCanvas2::slotSetDisplayProfile(const KoColorProfile *monitorProfile)
{
    if (m_d->displayColorConverter.monitorProfile() == monitorProfile) return;

    m_d->displayColorConverter.setMonitorProfile(monitorProfile);

    KisImageWSP image = this->image();

    image->barrierLock();

    m_d->canvasWidget->setDisplayProfile(&m_d->displayColorConverter);

    refetchDataFromImage();
}

void KisCanvas2::addDecoration(KisCanvasDecoration* deco)
{
    m_d->canvasWidget->addDecoration(deco);
}

KisCanvasDecoration* KisCanvas2::decoration(const QString& id) const
{
    return m_d->canvasWidget->decoration(id);
}


QPoint KisCanvas2::documentOrigin() const
{
    /**
     * In Krita we don't use document origin anymore.
     * All the centering when needed (vastScrolling < 0.5) is done
     * automatically by the KisCoordinatesConverter.
     */

    return QPoint();
}

QPoint KisCanvas2::documentOffset() const
{
    return m_d->coordinatesConverter->documentOffset();
}

void KisCanvas2::setFavoriteResourceManager(KisFavoriteResourceManager* favoriteResourceManager)
{
    m_d->popupPalette = new KisPopupPalette(favoriteResourceManager, displayColorConverter()->displayRendererInterface(), m_d->canvasWidget->widget());
    m_d->popupPalette->showPopupPalette(false);
}

void KisCanvas2::setCursor(const QCursor &cursor)
{
    canvasWidget()->setCursor(cursor);
}

KisAnimationFrameCacheSP KisCanvas2::frameCache() const
{
    return m_d->frameCache;
}

KisAnimationPlayer *KisCanvas2::animationPlayer() const
{
    return m_d->animationPlayer;
}

void KisCanvas2::slotSelectionChanged()
{
    KisShapeLayer* shapeLayer = dynamic_cast<KisShapeLayer*>(viewManager()->activeLayer().data());
    if (!shapeLayer) {
        return;
    }
    m_d->shapeManager.selection()->deselectAll();
    foreach(KoShape* shape, shapeLayer->shapeManager()->selection()->selectedShapes()) {
        m_d->shapeManager.selection()->select(shape);
    }
}

bool KisCanvas2::isPopupPaletteVisible() const
{
    if (!m_d->popupPalette) {
        return false;
    }
    return m_d->popupPalette->isVisible();
}


void KisCanvas2::setWrapAroundViewingMode(bool value)
{
    KisCanvasDecoration *infinityDecoration =
        m_d->canvasWidget->decoration(INFINITY_DECORATION_ID);

    if (infinityDecoration) {
        infinityDecoration->setVisible(!value);
    }

    m_d->canvasWidget->setWrapAroundViewingMode(value);
}

bool KisCanvas2::wrapAroundViewingMode() const
{
    KisCanvasDecoration *infinityDecoration =
        m_d->canvasWidget->decoration(INFINITY_DECORATION_ID);

    if (infinityDecoration) {
        return !(infinityDecoration->visible());
    }
    return false;
}

void KisCanvas2::bootstrapFinished()
{
    if (!m_d->bootsrapLodBlocked) return;

    m_d->bootsrapLodBlocked = false;
    setLodAllowedInCanvas(m_d->lodAllowedInCanvas);
}

void KisCanvas2::setLodAllowedInCanvas(bool value)
{
#ifdef HAVE_OPENGL
    m_d->lodAllowedInCanvas =
        value &&
        m_d->currentCanvasIsOpenGL &&
        KisOpenGL::supportsGLSL13();
#else
    Q_UNUSED(value);
    m_d->lodAllowedInCanvas = false;
#endif

    KisImageSP image = this->image();

    if (m_d->effectiveLodAllowedInCanvas() != !image->levelOfDetailBlocked()) {

        image->setLevelOfDetailBlocked(!m_d->effectiveLodAllowedInCanvas());
        notifyLevelOfDetailChange();
    }

    KisConfig cfg;
    cfg.setLevelOfDetailEnabled(m_d->lodAllowedInCanvas);
}

bool KisCanvas2::lodAllowedInCanvas() const
{
    return m_d->lodAllowedInCanvas;
}

KoGuidesData *KisCanvas2::guidesData()
{
    return &m_d->view->document()->guidesData();
}

void KisCanvas2::slotShowPopupPalette(const QPoint &p)
{
    if (!m_d->popupPalette) {
        return;
    }

    m_d->popupPalette->showPopupPalette(p);
}

KisPaintingAssistantsDecoration* KisCanvas2::paintingAssistantsDecoration() const
{
    KisCanvasDecoration* deco = decoration("paintingAssistantsDecoration");
    return qobject_cast<KisPaintingAssistantsDecoration*>(deco);
}
