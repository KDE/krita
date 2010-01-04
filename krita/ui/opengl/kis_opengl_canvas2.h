/*
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

#ifndef KIS_OPENGL_CANVAS_2_H
#define KIS_OPENGL_CANVAS_2_H

#include <opengl/kis_opengl.h>

#ifdef HAVE_OPENGL

#include <QGLWidget>

#include <KoCanvasBase.h>

#include "canvas/kis_abstract_canvas_widget.h"
#include "opengl/kis_opengl_image_textures.h"

#include "krita_export.h"

class QWidget;
class QPaintEvent;
class KisCanvas2;


/**
 * KisOpenGLCanvas is the widget that shows the actual image using OpenGL
 *
 * NOTE: if you change something in the event handling here, also change it
 * in the qpainter canvas.
 *
 */
class KRITAUI_EXPORT KisOpenGLCanvas2 : public QGLWidget, public KisAbstractCanvasWidget
{

    Q_OBJECT

public:

    KisOpenGLCanvas2(KisCanvas2 * canvas, QWidget * parent, KisOpenGLImageTexturesSP imageTextures);

    virtual ~KisOpenGLCanvas2();

    /** 
     * Prepare the canvas for rendering using native OpenGL 
     * commands. This sets the projection and model view matrices so
     * that primitives can be rendered using coordinates returned 
     * from pixelToView(). 
     */ 
    void beginOpenGL(void);

    /** 
     * Notify the canvas that rendering using native OpenGL commands
     * has finished. This restores the state so that the canvas can 
     * be painted on using a QPainter. 
     */ 
    void endOpenGL(void);

    /**
     * Set the projection and model view matrices so that primitives can be
     * rendered using image pixel coordinates. This handles zooming and scrolling
     * of the canvas.
     */
    void setPixelToViewTransformation(void);

public: // QWidget

    /// reimplemented method from superclass
    void enterEvent(QEvent* e);

    /// reimplemented method from superclass
    void leaveEvent(QEvent* e);


    /// reimplemented method from superclass
    void keyPressEvent(QKeyEvent *e);

    /// reimplemented method from superclass
    void mouseMoveEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void contextMenuEvent(QContextMenuEvent *e);

    /// reimplemented method from superclass
    void mousePressEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void mouseReleaseEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void mouseDoubleClickEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void keyReleaseEvent(QKeyEvent *e);

    /// reimplemented method from superclass
    void paintEvent(QPaintEvent * ev);

    /// reimplemented method from superclass
    void tabletEvent(QTabletEvent *e);

    /// reimplemented method from superclass
    void wheelEvent(QWheelEvent *e);

    /// reimplemented method from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    /// reimplemented method from superclass
    virtual void inputMethodEvent(QInputMethodEvent *event);

signals:
    /**
     * This signal is emitted when the document origin has changed.
     * The document origin is the point (in pixel) on the virtual
     * canvas where the documents origin (0,0) or the top left
     * corner of the page is. Copied from Karbon
     */
    void documentOriginChanged(const QPoint &origin);

protected:

    void initializeGL();
    void resizeGL(int w, int h);

    /// these methods take origin coordinate into account, basically it means (point - origin)
    QPoint widgetToView(const QPoint& p) const;
    QRect widgetToView(const QRect& r) const;
    QPoint viewToWidget(const QPoint& p) const;
    QRect viewToWidget(const QRect& r) const;

public: // KisAbstractCanvasWidget

    QWidget * widget() {
        return this;
    }

    KoToolProxy * toolProxy();

    void documentOffsetMoved(const QPoint & pt);
    virtual QPoint documentOrigin();
    void adjustOrigin();


private:
    class Private;
    Private * const m_d;

    void draw();
    void drawBorder();
    void drawImage();
    void drawBackground();

    void setupMatrices();
    void applyZoomScalingToModelView();

    qreal zoomScaleX() const;
    qreal zoomScaleY() const;

    void saveGLState();
    void restoreGLState();

};

#endif // HAVE_OPENGL
#endif // KIS_OPENGL_CANVAS_2_H
