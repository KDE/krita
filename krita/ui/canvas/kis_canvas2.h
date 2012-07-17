/* This file is part of the KDE project
 * Copyright (C) 2006, 2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2011       Silvio Heinrich <plassy@web.de>
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

#ifndef KIS_CANVAS_H
#define KIS_CANVAS_H

#include <QObject>
#include <QWidget>
#include <QSize>
#include <QString>

#include <KoColorSpace.h>
#include <KoCanvasBase.h>
#include <krita_export.h>
#include <kis_types.h>
#include <KoPointerEvent.h>

#include "kis_ui_types.h"
#include "kis_coordinates_converter.h"

class KoToolProxy;
class KoColorProfile;

class KisCanvasDecoration;
class KisView2;
class KisPaintopBox;
class KoFavoriteResourceManager;
class KisDisplayFilter;

enum KisCanvasType {
    QPAINTER,
    OPENGL
};

class KisCoordinatesConverter;
class KoViewConverter;

/**
 * KisCanvas2 is not an actual widget class, but rather an adapter for
 * the widget it contains, which may be either a QPainter based
 * canvas, or an OpenGL based canvas: that are the real widgets.
 */
class KRITAUI_EXPORT KisCanvas2 : public QObject, public KoCanvasBase
{

    Q_OBJECT

public:

    /**
     * Create a new canvas. The canvas manages a widget that will do
     * the actual painting: the canvas itself is not a widget.
     *
     * @param viewConverter the viewconverter for converting between
     *                       window and document coordinates.
     */
    KisCanvas2(KisCoordinatesConverter* coordConverter, KisView2* view, KoShapeBasedDocumentBase* sc);

    virtual ~KisCanvas2();

    void setCanvasWidget(QWidget * widget);

    void notifyZoomChanged();

    virtual void disconnectCanvasObserver(QObject *object);

public: // KoCanvasBase implementation

    bool canvasIsOpenGL();

    void gridSize(qreal *horizontal, qreal *vertical) const;

    bool snapToGrid() const;

    void addCommand(KUndo2Command *command);

    virtual void startMacro(const QString &title);
    virtual void stopMacro();

    virtual QPoint documentOrigin() const;
    QPoint documentOffset() const;

    /**
     * Return the right shape manager for the current layer. That is
     * to say, if the current layer is a vector layer, return the shape
     * layer's canvas' shapemanager, else the shapemanager associated
     * with the global krita canvas.
     */
    KoShapeManager * shapeManager() const;

    /**
     * Return the shape manager associated with this canvas
     */
    KoShapeManager * globalShapeManager() const;

    void updateCanvas(const QRectF& rc);

    virtual void updateInputMethodInfo();

    const KisCoordinatesConverter* coordinatesConverter() const;
    virtual KoViewConverter *viewConverter() const;

    virtual QWidget* canvasWidget();

    virtual const QWidget* canvasWidget() const;

    virtual KoUnit unit() const;

    virtual KoToolProxy* toolProxy() const;

    KoColorProfile* monitorProfile();

    /**
     * Prescale the canvas represention of the image (if necessary, it
     * is for QPainter, not for OpenGL).
     */
    void preScale();

    // FIXME:
    // Temporary! Either get the current layer and image from the
    // resource provider, or use this, which gets them from the
    // current shape selection.
    KisImageWSP currentImage();

public: // KisCanvas2 methods

    KisImageWSP image();
    KisView2* view();

    bool usingHDRExposureProgram();
    /// @return true if the canvas image should be displayed in vertically mirrored mode
    void addDecoration(KisCanvasDecoration* deco);
    KisCanvasDecoration* decoration(const QString& id);

signals:

    void documentOriginChanged();
    void scrollAreaSizeChanged();

    void imageChanged(KisImageWSP image);

    void canvasDestroyed(QWidget *);

    void favoritePaletteCalled(const QPoint&);

    void sigCanvasCacheUpdated(KisUpdateInfoSP);
    void sigContinueResizeImage(qint32 w, qint32 h);

public slots:

    /// Update the entire canvas area
    void updateCanvas();

    /// The image projection has changed, now start an update
    /// of the canvas representation.
    void startUpdateCanvasProjection(const QRect & rc);
    void updateCanvasProjection(KisUpdateInfoSP info);

    void startUpdateInPatches(QRect imageRect);

    void setMonitorProfile(KoColorProfile* monitorProfile,
                           KoColorConversionTransformation::Intent renderingIntent,
                           KoColorConversionTransformation::ConversionFlags conversionFlags);



    void setDisplayFilter(KisDisplayFilter *displayFilter);

    void startResizingImage(qint32 w, qint32 h);
    void finishResizingImage(qint32 w, qint32 h);

    /// adjust the origin of the document
    void adjustOrigin();

    /// slot for setting the mirroring
    void mirrorCanvas(bool mirror);
    /// canvas rotation in degrees
    qreal rotationAngle() const;
    void rotateCanvas(qreal angle, bool updateOffset=true);
    void rotateCanvasRight15();
    void rotateCanvasLeft15();
    void resetCanvasTransformations();

    void setSmoothingEnabled(bool smooth);

private slots:

    /**
     * Called whenever the view widget needs to show a different part of
     * the document
     *
     * @param documentOffset the offset in widget pixels
     */
    void documentOffsetMoved(const QPoint &documentOffset);

    /**
     * Called whenever the configuration settings change.
     */
    void slotConfigChanged();

    /**
     * Called whenever the display monitor profile resource changes
     */
    void slotSetDisplayProfile(const KoColorProfile * profile);

    void slotCanvasDestroyed(QWidget* w);

    void setCursor(const QCursor &cursor);

public:
//    friend class KisView2;

    // interface for KisView2 only
    void connectCurrentImage();
    void disconnectCurrentImage();
    void resetCanvas(bool useOpenGL);

    void createFavoriteResourceManager(KisPaintopBox*);
    KoFavoriteResourceManager* favoriteResourceManager();
    bool handlePopupPaletteIsVisible();

private:
    Q_DISABLE_COPY(KisCanvas2);

    void pan(QPoint shift);
    void createCanvas(bool useOpenGL);
    void createQPainterCanvas();
    void createOpenGLCanvas();

private:

    class KisCanvas2Private;
    KisCanvas2Private * const m_d;
};

#endif
