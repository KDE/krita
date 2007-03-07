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
#include <QPixmap>
#include <QSize>

#include <KoCanvasBase.h>
#include <krita_export.h>
#include <kis_types.h>

class KoToolProxy;
class KoColorProfile;

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
     * @param canvasType determines which kind of canvas widget the
     *                   canvas initially creates.
     */
    KisCanvas2(KoViewConverter * viewConverter, KisView2 * view, KoShapeControllerBase * sc);

    virtual ~KisCanvas2();

    void setCanvasWidget(QWidget * widget);

public: // KoCanvasBase implementation

    void gridSize(double *horizontal, double *vertical) const;

    bool snapToGrid() const;

    void addCommand(QUndoCommand *command);

    KoShapeManager *shapeManager() const;

    void updateCanvas(const QRectF& rc);

    KoViewConverter *viewConverter();

    QWidget* canvasWidget();

    QImage canvasCache();

    KoUnit unit();

    KoToolProxy* toolProxy();

    KoColorProfile * monitorProfile();

    /**
     * Prescale the canvas represention of the image (if necessary, it
     * is for QPainter, not for OpenGL).
     */
    void preScale();

    void connectCurrentImage();

    void disconnectCurrentImage();

    void resetCanvas();

    // Temporary!
#ifdef __GNUC__
#warning: "Make the tools get the current paint device from the shape selection instead of directly from the canvas!"
#endif
    KisImageSP currentImage();

public: // KisCanvas2 methods

    KisImageSP image();
    KisView2* view();

    bool isOpenGLCanvas() { return false; }

public slots:

    /// Update the entire canvas area
    void updateCanvas();

    /// The image projection has changed, now update the canvas
    /// representation of it.
    void updateCanvasProjection( const QRect & rc );

    void setImageSize(qint32 w, qint32 h);

private slots:

    /**
     * Called whenever the view widget needs to show a different part of
     * the document
     *
     * @param documentOffset the offset in widget pixels
     */
    void documentOffsetMoved( const QPoint &documentOffset );

private:

    void resetMonitorProfile();

    KisCanvas2(const KisCanvas2&);

private:

    class KisCanvas2Private;
    KisCanvas2Private * m_d;
};

#endif
