/*
 * SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_OPENGL_CANVAS_2_H
#define KIS_OPENGL_CANVAS_2_H

#include <QOpenGLWidget>
#ifndef Q_OS_MACOS
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_2_Core>
#endif
#include "canvas/kis_canvas_widget_base.h"
#include "opengl/kis_opengl_image_textures.h"

#include "kritaui_export.h"
#include "kis_ui_types.h"

class KisCanvas2;
class KisDisplayColorConverter;
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
 * KisOpenGLCanvas is the widget that shows the actual image using OpenGL
 *
 * NOTE: if you change something in the event handling here, also change it
 * in the qpainter canvas.
 *
 */
class KRITAUI_EXPORT KisOpenGLCanvas2
        : public QOpenGLWidget
#ifndef Q_MOC_RUN
        , protected GLFunctions
#endif
        , public KisCanvasWidgetBase
{
    Q_OBJECT

public:

    KisOpenGLCanvas2(KisCanvas2 *canvas, KisCoordinatesConverter *coordinatesConverter, QWidget *parent, KisImageWSP image, KisDisplayColorConverter *colorConverter);

    ~KisOpenGLCanvas2() override;

public: // QOpenGLWidget

    void resizeGL(int width, int height) override;
    void initializeGL() override;
    void paintGL() override;
    void paintEvent(QPaintEvent *e) override;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    void inputMethodEvent(QInputMethodEvent *event) override;

public:
    void renderCanvasGL(const QRect &updateRect);
    void renderDecorations(const QRect &updateRect);
    void paintToolOutline(const QPainterPath &path);


public: // Implement kis_abstract_canvas_widget interface
    void setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter) override;
    void notifyImageColorSpaceChanged(const KoColorSpace *cs) override;

    void setWrapAroundViewingMode(bool value) override;
    bool wrapAroundViewingMode() const override;

    void channelSelectionChanged(const QBitArray &channelFlags) override;
    void setDisplayColorConverter(KisDisplayColorConverter *colorConverter) override;
    void finishResizingImage(qint32 w, qint32 h) override;
    KisUpdateInfoSP startUpdateCanvasProjection(const QRect & rc, const QBitArray &channelFlags) override;
    QRect updateCanvasProjection(KisUpdateInfoSP info) override;
    QVector<QRect> updateCanvasProjection(const QVector<KisUpdateInfoSP> &infoObjects) override;

    QWidget *widget() override {
        return this;
    }

    bool isBusy() const override;
    void setLodResetInProgress(bool value) override;

    void setDisplayFilterImpl(QSharedPointer<KisDisplayFilter> displayFilter, bool initializing);

    KisOpenGLImageTexturesSP openGLImageTextures() const;

public Q_SLOTS:
    void slotConfigChanged();
    void slotPixelGridModeChanged();

private Q_SLOTS:
    void slotShowFloatingMessage(const QString &message, int timeout, bool priority);

protected: // KisCanvasWidgetBase
    bool callFocusNextPrevChild(bool next) override;

private:
    void initializeShaders();
    void initializeDisplayShader();

    void reportFailedShaderCompilation(const QString &context);
    void drawBackground(const QRect &updateRect);
    void drawImage(const QRect &updateRect);
    void drawImageTiles(int firstCol, int lastCol, int firstRow, int lastRow, qreal scaleX, qreal scaleY, const QPoint &wrapAroundOffset);
    void drawCheckers(const QRect &updateRect);
    void drawGrid(const QRect &updateRect);
    QSize viewportDevicePixelSize() const;
    QSizeF widgetSizeAlignedToDevicePixel() const;

private:

    struct Private;
    Private * const d;

};

#endif // KIS_OPENGL_CANVAS_2_H
