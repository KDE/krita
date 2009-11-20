/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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

#include <KoCanvasBase.h>
#include <krita_export.h>
#include <kis_types.h>

class KoToolProxy;
class KoColorProfile;

class KisCanvasDecoration;
class KisView2;

enum KisCanvasType {
    QPAINTER,
    OPENGL,
    MITSHM
};

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
    KisCanvas2(KoViewConverter * viewConverter, KisView2 * view, KoShapeControllerBase * sc);

    virtual ~KisCanvas2();

    void setCanvasWidget(QWidget * widget);

public: // KoCanvasBase implementation

    void gridSize(qreal *horizontal, qreal *vertical) const;

    bool snapToGrid() const;

    void addCommand(QUndoCommand *command);

    virtual void startMacro(const QString &title);
    virtual void stopMacro();

    virtual QPoint documentOrigin() const;
    QPoint documentOffset() const;

    /**
     * Return the right shape manager for the current layer. That is
     * to say, if the current layer is a shape layer, return the shape
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

    virtual const KoViewConverter *viewConverter() const;

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

    void connectCurrentImage();

    void disconnectCurrentImage();

    // Temporary! Either get the current layer and image from the
    // resource provider, or use this, which gets them from the
    // current shape selection.
    KisImageWSP currentImage();

public: // KisCanvas2 methods

    KisImageWSP image();
    KisView2* view();

    bool usingHDRExposureProgram();
    void addDecoration(KisCanvasDecoration* deco);
    KisCanvasDecoration* decoration(const QString& id);

signals:

    void documentOriginChanged();

    void imageChanged(KisImageWSP image);

    void canvasDestroyed(QWidget *);

public slots:

    /// Update the entire canvas area
    void updateCanvas();

    /// The image projection has changed, now update the canvas
    /// representation of it.
    void updateCanvasProjection(const QRect & rc);

    void setImageSize(qint32 w, qint32 h);

    /// adjust the origin of the document
    void adjustOrigin();

    /// documentOffset changed, now update the rulers in zoom manager
    void updateRulers();

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

private:

    friend class KisView2;

    void resetCanvas();

    KisCanvas2(const KisCanvas2&);

    void createCanvas();
    void createQPainterCanvas();
    void createOpenGLCanvas();

    /**
     * Returns a rect in widget pixels that is translated for document
     * offset, resolution and zoom and that is guaranteed to be inside
     * the widget.
     */
    QRect viewRectFromDoc(const QRectF & docRect);

private:

    class KisCanvas2Private;
    KisCanvas2Private * const m_d;
};

#endif
