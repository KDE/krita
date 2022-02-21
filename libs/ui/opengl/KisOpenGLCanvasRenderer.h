/*
 * SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_OPENGL_CANVAS_RENDERER_H
#define KIS_OPENGL_CANVAS_RENDERER_H

#include <QtGlobal>

#ifndef Q_OS_MACOS
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_2_Core>
#endif
#include "opengl/kis_opengl_image_textures.h"

#include "kritaui_export.h"
#include "kis_ui_types.h"

class KisCanvas2;
class KisCoordinatesConverter;
class KisDisplayColorConverter;
class KisDisplayFilter;
class QOpenGLShaderProgram;
class QPainterPath;

#ifndef Q_MOC_RUN
#ifndef Q_OS_MACOS
#define GLFunctions QOpenGLFunctions
#else
#define GLFunctions QOpenGLFunctions_3_2_Core
#endif

#endif
/**
 * KisOpenGLCanvasRenderer is the class that shows the actual image using OpenGL
 *
 */
class KisOpenGLCanvasRenderer
        : private GLFunctions
{
public:
    class CanvasBridge;

    KisOpenGLCanvasRenderer(CanvasBridge *canvasBridge, KisImageWSP image, KisDisplayColorConverter *colorConverter);

    ~KisOpenGLCanvasRenderer();

    Q_DISABLE_COPY(KisOpenGLCanvasRenderer)

public:
    void resizeGL(int width, int height);
    void initializeGL();

    /**
     * Paint only the canvas background and image tiles.
     */
    void paintCanvasOnly(const QRect &canvasImageDirtyRect, const QRect &viewportUpdateRect = QRect());

private:
    void updateSize(const QSize &viewportSize);
    void renderCanvasGL(const QRect &updateRect);

public:
    void paintToolOutline(const QPainterPath &path);

    void setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter);
    void notifyImageColorSpaceChanged(const KoColorSpace *cs);

    void setWrapAroundViewingMode(bool value);
    bool wrapAroundViewingMode() const;

    void channelSelectionChanged(const QBitArray &channelFlags);
    void setDisplayColorConverter(KisDisplayColorConverter *colorConverter);
    void finishResizingImage(qint32 w, qint32 h);
    KisUpdateInfoSP startUpdateCanvasProjection(const QRect & rc, const QBitArray &channelFlags);
    QRect updateCanvasProjection(KisUpdateInfoSP info);

    void setLodResetInProgress(bool value);

private:
    void setDisplayFilterImpl(QSharedPointer<KisDisplayFilter> displayFilter, bool initializing);

public:
    KisOpenGLImageTexturesSP openGLImageTextures() const;

    void updateConfig();
    void updatePixelGridMode();

private:
    void initializeShaders();
    void initializeDisplayShader();

    void reportFailedShaderCompilation(const QString &context);
    void drawBackground(const QRect &updateRect);
    void drawImage(const QRect &updateRect);
    void drawImageTiles(int firstCol, int lastCol, int firstRow, int lastRow, qreal scaleX, qreal scaleY, const QPoint &wrapAroundOffset);
    void drawCheckers(const QRect &updateRect);
    void drawGrid(const QRect &updateRect);

    QRectF widgetToSurface(const QRectF &rc);
    QRectF surfaceToWidget(const QRectF &rc);

private:
    struct Private;
    Private * const d;

    KisCanvas2 *canvas() const;
    QOpenGLContext *context() const;
    qreal devicePixelRatioF() const;
    KisCoordinatesConverter *coordinatesConverter() const;
    QColor borderColor() const;
};

class KisOpenGLCanvasRenderer::CanvasBridge
{
    friend class KisOpenGLCanvasRenderer;

public:
    CanvasBridge() = default;
    virtual ~CanvasBridge() = default;

    Q_DISABLE_COPY(CanvasBridge)

protected:
    virtual KisCanvas2 *canvas() const = 0;
    virtual QOpenGLContext *openglContext() const = 0;
    virtual qreal devicePixelRatioF() const = 0;
    virtual KisCoordinatesConverter *coordinatesConverter() const = 0;
    virtual QColor borderColor() const = 0;
};

#endif // KIS_OPENGL_CANVAS_RENDERER_H
