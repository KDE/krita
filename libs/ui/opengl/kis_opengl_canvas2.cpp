/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2013 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#define GL_GLEXT_PROTOTYPES

#include "opengl/kis_opengl_canvas2.h"
#include "opengl/KisOpenGLCanvasRenderer.h"
#include "opengl/KisOpenGLSync.h"
#include "opengl/kis_opengl_canvas_debugger.h"

#include "canvas/kis_canvas2.h"
#include <kis_canvas_resource_provider.h>
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_debug.h"
#include <KisViewManager.h>
#include <KisMainWindow.h>
#include "KisRepaintDebugger.h"

#include "KisOpenGLModeProber.h"
#include "KisOpenGLContextSwitchLock.h"

#include "config-qt-patches-present.h"

static bool OPENGL_SUCCESS = false;

class KisOpenGLCanvas2::CanvasBridge
    : public KisOpenGLCanvasRenderer::CanvasBridge
{
    friend class KisOpenGLCanvas2;
    explicit CanvasBridge(KisOpenGLCanvas2 *canvas)
        : m_canvas(canvas)
    {}
    ~CanvasBridge() override = default;
    Q_DISABLE_COPY(CanvasBridge)
    KisOpenGLCanvas2 *m_canvas;
protected:
    KisCanvas2 *canvas() const override {
        return m_canvas->canvas();
    }
    QOpenGLContext *openglContext() const override {
        return m_canvas->context();
    }
    qreal devicePixelRatioF() const override {
        return m_canvas->devicePixelRatioF();
    }
    KisCoordinatesConverter *coordinatesConverter() const override {
        return m_canvas->coordinatesConverter();
    }
    QColor borderColor() const override {
        return m_canvas->borderColor();
    }
};

struct KisOpenGLCanvas2::Private
{
public:
    ~Private() {
        delete renderer;
    }

    boost::optional<QRect> updateRect;
#if KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    bool shouldSkipRenderingPass = false;
#endif
    QRect canvasImageDirtyRect;
    KisOpenGLCanvasRenderer *renderer;
    QScopedPointer<KisOpenGLSync> glSyncObject;
    KisRepaintDebugger repaintDbg;
};

KisOpenGLCanvas2::KisOpenGLCanvas2(KisCanvas2 *canvas,
                                   KisCoordinatesConverter *coordinatesConverter,
                                   QWidget *parent,
                                   KisImageWSP image,
                                   KisDisplayColorConverter *colorConverter)
    : QOpenGLWidget(parent)
    , KisCanvasWidgetBase(canvas, coordinatesConverter)
    , d(new Private())
{
    KisConfig cfg(false);
    cfg.setCanvasState("OPENGL_STARTED");

    d->renderer = new KisOpenGLCanvasRenderer(new CanvasBridge(this), image, colorConverter);

    connect(d->renderer->openGLImageTextures().data(),
            SIGNAL(sigShowFloatingMessage(QString, int, bool)),
            SLOT(slotShowFloatingMessage(QString, int, bool)));

    setAcceptDrops(true);
    setAutoFillBackground(false);

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_NoSystemBackground, true);
#ifdef Q_OS_MACOS
    setAttribute(Qt::WA_AcceptTouchEvents, false);
#else
    setAttribute(Qt::WA_AcceptTouchEvents, true);
#endif
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setAttribute(Qt::WA_DontCreateNativeAncestors, true);

    const bool osManagedSurfacePresent = canvas->viewManager()->mainWindow()->managedSurfaceProfile();
    bool useNativeSurfaceForCanvas = osManagedSurfacePresent && cfg.enableCanvasSurfaceColorSpaceManagement();
    if (qEnvironmentVariableIsSet("KRITA_USE_NATIVE_CANVAS_SURFACE")) {
        useNativeSurfaceForCanvas = qEnvironmentVariableIntValue("KRITA_USE_NATIVE_CANVAS_SURFACE");
        qDebug() << "FPS-DEBUG: Krita canvas mode is overridden:" << (useNativeSurfaceForCanvas ? "native surface" : "legacy mode") << useNativeSurfaceForCanvas << qEnvironmentVariableIsSet("KRITA_USE_NATIVE_CANVAS_SURFACE");
    }

    if (useNativeSurfaceForCanvas) {
        setAttribute(Qt::WA_NativeWindow, true);
    }

    setUpdateBehavior(PartialUpdate);

    // we should make sure the texture doesn't have alpha channel,
    // otherwise blending will not work correctly.
    if (KisOpenGLModeProber::instance()->useHDRMode()) {
        setTextureFormat(GL_RGBA16F);
    } else {
        /**
         * When in pure OpenGL mode, the canvas surface will have alpha
         * channel. Therefore, if our canvas blending algorithm produces
         * semi-transparent pixels (and it does), then Krita window itself
         * will become transparent. Which is not good.
         *
         * In Angle mode, GL_RGB8 is not available (and the transparence effect
         * doesn't exist at all).
         */
        if (!KisOpenGL::hasOpenGLES()) {
            setTextureFormat(GL_RGB8);
        }
    }

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    connect(KisConfigNotifier::instance(), SIGNAL(pixelGridModeChanged()), SLOT(slotPixelGridModeChanged()));

    connect(canvas->viewManager()->canvasResourceProvider(), SIGNAL(sigEffectiveCompositeOpChanged()), SLOT(slotUpdateCursorColor()));
    connect(canvas->viewManager()->canvasResourceProvider(), SIGNAL(sigPaintOpPresetChanged(KisPaintOpPresetSP)), SLOT(slotUpdateCursorColor()));

    slotConfigChanged();
    slotPixelGridModeChanged();
    cfg.writeEntry("canvasState", "OPENGL_SUCCESS");
}

KisOpenGLCanvas2::~KisOpenGLCanvas2()
{
    /**
     * Since we delete openGL resources, we should make sure the
     * context is initialized properly before they are deleted.
     * Otherwise resources from some other (current) context may be
     * deleted due to resource id aliasing.
     *
     * The main symptom of resources being deleted from wrong context,
     * the canvas being locked/backened-out after some other document
     * is closed.
     */

    makeCurrent();

    delete d;

    doneCurrent();
}

void KisOpenGLCanvas2::setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter)
{
    KisOpenGLContextSwitchLockSkipOnQt5 contextLock(this);
    d->renderer->setDisplayFilter(displayFilter);
}

void KisOpenGLCanvas2::notifyImageColorSpaceChanged(const KoColorSpace *cs)
{
    KisOpenGLContextSwitchLockSkipOnQt5 contextLock(this);
    d->renderer->notifyImageColorSpaceChanged(cs);
}

void KisOpenGLCanvas2::setWrapAroundViewingMode(bool value)
{
    d->renderer->setWrapAroundViewingMode(value);
    update();
}

bool KisOpenGLCanvas2::wrapAroundViewingMode() const
{
    return d->renderer->wrapAroundViewingMode();
}

void KisOpenGLCanvas2::setWrapAroundViewingModeAxis(WrapAroundAxis value)
{
    d->renderer->setWrapAroundViewingModeAxis(value);
    update();
}

WrapAroundAxis KisOpenGLCanvas2::wrapAroundViewingModeAxis() const
{
    return d->renderer->wrapAroundViewingModeAxis();
}

void KisOpenGLCanvas2::initializeGL()
{
    d->renderer->initializeGL();
    KisOpenGLSync::init(context());
}

void KisOpenGLCanvas2::resizeGL(int width, int height)
{
    d->renderer->resizeGL(width, height);
    d->canvasImageDirtyRect = QRect(0, 0, width, height);
}

void KisOpenGLCanvas2::paintGL()
{
#if KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    if (d->shouldSkipRenderingPass) {
        return;
    }
#endif

    const QRect updateRect = d->updateRect ? *d->updateRect : QRect();

    if (!OPENGL_SUCCESS) {
        KisConfig cfg(false);
        cfg.writeEntry("canvasState", "OPENGL_PAINT_STARTED");
    }

    KisOpenglCanvasDebugger::instance()->notifyPaintRequested();
    QRect canvasImageDirtyRect = d->canvasImageDirtyRect & rect();
    d->canvasImageDirtyRect = QRect();
    d->renderer->paintCanvasOnly(canvasImageDirtyRect, updateRect);
    {
        QPainter gc(this);
        if (!updateRect.isEmpty()) {
            gc.setClipRect(updateRect);
        }

        QRect decorationsBoundingRect = coordinatesConverter()->imageRectInWidgetPixels().toAlignedRect();

        if (!updateRect.isEmpty()) {
            decorationsBoundingRect &= updateRect;
        }

        drawDecorations(gc, decorationsBoundingRect);
    }

    d->repaintDbg.paint(this, updateRect.isEmpty() ? rect() : updateRect);

    // We create the glFenceSync object here instead of in KisOpenGLRenderer,
    // because the glFenceSync object should be created after all render
    // commands in a frame, not just the OpenGL canvas itself. Putting it
    // outside of KisOpenGLRenderer allows the canvas widget to do extra
    // rendering, which a QtQuick2-based canvas will need.
    d->glSyncObject.reset(new KisOpenGLSync());

    if (!OPENGL_SUCCESS) {
        KisConfig cfg(false);
        cfg.writeEntry("canvasState", "OPENGL_SUCCESS");
        OPENGL_SUCCESS = true;
    }
}

void KisOpenGLCanvas2::paintEvent(QPaintEvent *e)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!d->updateRect);

    if (qFuzzyCompare(devicePixelRatioF(), qRound(devicePixelRatioF()))) {
        /**
         * Enable partial updates **only** for the case when we have
         * integer scaling. There is a bug in Qt that causes artifacts
         * otherwise:
         *
         * See https://bugs.kde.org/show_bug.cgi?id=441216
         */
        d->updateRect = e->rect();
    } else {
        d->updateRect = this->rect();
    }

#if KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    /**
     * When using Qt with a proper update paint event compression, then we don't
     * need to implement our own one in KisCanvas2, instead we should just skip
     * frames in paintEvent(), when the previous frame hasn't completed yet.
     */
    if (isBusy()) {
        //qWarning() << "WARNING: paint event delivered with the canvas non-ready, rescheduling...";
        d->shouldSkipRenderingPass = true;
        QOpenGLWidget::paintEvent(e);
        d->shouldSkipRenderingPass = false;
        QTimer::singleShot(0, this,
            [this, updateRect = *d->updateRect] () {
                if (updateRect.isEmpty()) {
                    this->update();
                } else {
                    this->update(updateRect);
                }
            });
    } else
#endif
    {
        QOpenGLWidget::paintEvent(e);
    }

    d->updateRect = boost::none;
}

void KisOpenGLCanvas2::paintToolOutline(const KisOptimizedBrushOutline &path, int thickness)
{
    /**
     * paintToolOutline() is called from drawDecorations(), which has clipping
     * set only for QPainter-based painting; here we paint in native mode, so we
     * should care about clipping manually
     *
     * `d->updateRect` might be empty in case the fractional DPI workaround
     * is active.
     */
    const QRect updateRect = d->updateRect ? *d->updateRect : QRect();

    d->renderer->paintToolOutline(path, updateRect, thickness);
}

bool KisOpenGLCanvas2::isBusy() const
{
    const bool isBusyStatus = d->glSyncObject && !d->glSyncObject->isSignaled();
    KisOpenglCanvasDebugger::instance()->notifySyncStatus(isBusyStatus);
    return isBusyStatus;
}

void KisOpenGLCanvas2::setLodResetInProgress(bool value)
{
    d->renderer->setLodResetInProgress(value);
}

void KisOpenGLCanvas2::slotConfigChanged()
{
    d->renderer->updateConfig();

    notifyConfigChanged();
}

void KisOpenGLCanvas2::slotPixelGridModeChanged()
{
    d->renderer->updatePixelGridMode();

    update();
}

void KisOpenGLCanvas2::slotUpdateCursorColor()
{
    d->renderer->updateCursorColor();
}

void KisOpenGLCanvas2::slotShowFloatingMessage(const QString &message, int timeout, bool priority)
{
    canvas()->imageView()->showFloatingMessage(message, QIcon(), timeout, priority ? KisFloatingMessage::High : KisFloatingMessage::Medium);
}

QVariant KisOpenGLCanvas2::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return processInputMethodQuery(query);
}

void KisOpenGLCanvas2::inputMethodEvent(QInputMethodEvent *event)
{
    processInputMethodEvent(event);
}

void KisOpenGLCanvas2::focusInEvent(QFocusEvent *event)
{
    processFocusInEvent(event);
}

void KisOpenGLCanvas2::focusOutEvent(QFocusEvent *event)
{
    processFocusOutEvent(event);
}

void KisOpenGLCanvas2::hideEvent(QHideEvent *e)
{
    QOpenGLWidget::hideEvent(e);
    notifyDecorationsWindowMinimized(true);
}

void KisOpenGLCanvas2::showEvent(QShowEvent *e)
{
    QOpenGLWidget::showEvent(e);
    notifyDecorationsWindowMinimized(false);
}

void KisOpenGLCanvas2::setDisplayColorConverter(KisDisplayColorConverter *colorConverter)
{
    KisOpenGLContextSwitchLockSkipOnQt5 contextLock(this);
    d->renderer->setDisplayColorConverter(colorConverter);
}

void KisOpenGLCanvas2::channelSelectionChanged(const QBitArray &channelFlags)
{
    d->renderer->channelSelectionChanged(channelFlags);
}


void KisOpenGLCanvas2::finishResizingImage(qint32 w, qint32 h)
{
    KisOpenGLContextSwitchLockSkipOnQt5 contextLock(this);
    d->renderer->finishResizingImage(w, h);
}

KisUpdateInfoSP KisOpenGLCanvas2::startUpdateCanvasProjection(const QRect & rc)
{
    return d->renderer->startUpdateCanvasProjection(rc);
}


QRect KisOpenGLCanvas2::updateCanvasProjection(KisUpdateInfoSP info)
{
    return d->renderer->updateCanvasProjection(info);
}

QVector<QRect> KisOpenGLCanvas2::updateCanvasProjection(const QVector<KisUpdateInfoSP> &infoObjects)
{
    KisOpenGLContextSwitchLockSkipOnQt5 contextLock(this);
    return KisCanvasWidgetBase::updateCanvasProjection(infoObjects);
}

void KisOpenGLCanvas2::updateCanvasImage(const QRect &imageUpdateRect)
{
    d->canvasImageDirtyRect |= imageUpdateRect;
    update(imageUpdateRect);
}

void KisOpenGLCanvas2::updateCanvasDecorations(const QRect &decoUpdateRect)
{
    update(decoUpdateRect);
}
bool KisOpenGLCanvas2::callFocusNextPrevChild(bool next)
{
    return focusNextPrevChild(next);
}

KisOpenGLImageTexturesSP KisOpenGLCanvas2::openGLImageTextures() const
{
    return d->renderer->openGLImageTextures();
}
