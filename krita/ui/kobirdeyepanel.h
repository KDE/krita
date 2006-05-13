/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KO_BIRD_EYE_PANEL
#define KO_BIRD_EYE_PANEL

#include <QRect>
#include <QWidget>
//Added by qt3to4:
#include <QPixmap>
#include <QEvent>
#include <QPaintEvent>

#include <KoPoint.h>
#include <KoRect.h>

#include "ui_wdgbirdeye.h"

class QPixmap;
class KAction;
class KoDocument;
class WdgBirdEye;

class WdgBirdEye : public QWidget, public Ui::WdgBirdEye
{
    Q_OBJECT

    public:
        WdgBirdEye(QWidget *parent, const char *name) : QWidget(parent) { setObjectName(name); setupUi(this); }
};

class KoCanvasAdapter {

public:
    
    KoCanvasAdapter();
    virtual ~KoCanvasAdapter();

    /**
     * Returns the area of the document that is visible, in pixels
     */
    virtual KoRect visibleArea() = 0;
    
    /**
     * Returns the total area of the document in pixels. Use KoPageLayout and KoZoomhandler
     * to take care of zoom, points and whatnot when computing this.
     */
    virtual QRect size() = 0;
    
    /**
     * Return the current canvas zoom factor.
     */
    virtual double zoomFactor() = 0;

    /**
     * Show pt in the center of the view
     */
    virtual void setViewCenterPoint(double x, double y) = 0;
};

/**
 * The zoom listener interface defines methods that the bird eye
 * panel will call whenever the zoomlevel is changed through one
 * of the panel actions.
 */
class KoZoomAdapter {

public:

    KoZoomAdapter();
    virtual ~KoZoomAdapter();

    /**
     * Zoom to the specified factor around the point x and y
     */
    virtual void zoomTo(double x, double y, double factor ) = 0;
    
    /**
     * Zoom one step in.
     */
    virtual void zoomIn() = 0;
    
    /**
     * Zoom one step out.
     */
    virtual void zoomOut() = 0;
    
    /**
     * Get the minimum zoom factor that this listener supports.
     */
    virtual double getMinZoom() = 0;

    /**
     * Get the maximum zoom factor that this listener supports.
     */
    virtual double getMaxZoom() = 0;

};


class KoThumbnailAdapter
{
    public:
    
        KoThumbnailAdapter();
        virtual ~KoThumbnailAdapter();
        
        /**
         * Returns the size of the document in pixels.
         * If the document is a KoDocument that uses a KoPageLayout, the same
         * formula as in the generatePreview() method should be used to go from points
         * to pixels.
         *
         * @returns the size in pixels.
         */
        virtual QSize pixelSize() = 0;
        
        /**
         * Returns the specified rectangle of the thumbnail as a QImage. thumbnailSize
         * gives the dimensions of the whole document thumbnail, and r specifies a rectangle
         * within that.
         *
         * @param r the rectangle in the thumbnail to be rendered
         * @param thumbnailSize the size in pixels of the full thumbnail
         */
        virtual QImage image(QRect r, QSize thumbnailSize) = 0;
};

/**
 * A complex widget that provides an overview of a document
 * with a red panning rectangle to and a zoom slider and a toolbar
 * with a couple of useful functions.
 */
class KoBirdEyePanel : public QWidget {

    Q_OBJECT

public:

    /**
     * Create a new bird eye panel.
     *
     * @param zoomListener the object that listens to the zoom instructions we give
     * @param thumbnailProvider the class that creates the small image at the right
     *        zoomlevel
     * @param canvas the place the document is painted.
     * @param parent the parent widget
     * @param name the QObject name of this bird eye widget
     * @param f the widget flags (@see QWidget)
     */
    KoBirdEyePanel( KoZoomAdapter * zoomListener, 
                    KoThumbnailAdapter * thumbnailProvider,
                    KoCanvasAdapter * canvas,
                    QWidget * parent,
                    const char * name = 0,
                    Qt::WFlags f = 0 );

    virtual ~KoBirdEyePanel();

    bool eventFilter(QObject*, QEvent*);

public slots:

    void setZoomListener( KoZoomAdapter * zoomListener) { m_zoomListener = zoomListener; }

    /**
     * Set a new thumbnail provider. This will first delete the existing provider.
     **/
    void setThumbnailProvider( KoThumbnailAdapter * thumbnailProvider );

    /**
     * Connect to this slot to inform the bird's eye view of changes in
     * the view transformation, i.e. zoom level or scroll changes.
     */
    void slotViewTransformationChanged();
    
    void cursorPosChanged(qint32 xpos, qint32 ypos);

    void zoomMinus();
    void zoomPlus();

    /**
     * Connect to this slot if a (rectangular) area of your document is changed.
     * 
     * @param r The rect that has been changed: this is unzoomed.
     */
    void slotUpdate(const QRect & r);

protected slots:

    void updateVisibleArea();
    void zoomValueChanged(int zoom);
    void zoom100();
    void sliderChanged(int);
    
protected:
    void setZoom(int zoom);
            
    void handleMouseMove(QPoint);
    void handleMouseMoveAction(QPoint);
    void handleMousePress(QPoint);
    void fitThumbnailToView();
    void renderView();
    void resizeViewEvent(QSize size);
    void paintViewEvent(QPaintEvent *e);
    void makeThumbnailRectVisible(const QRect& r);

    enum enumDragHandle {
        DragHandleNone,
        DragHandleLeft,
        DragHandleCentre,
        DragHandleRight,
        DragHandleTop,
        DragHandleBottom
    };

    /*
     * Returns the drag handle type at point p in thumbnail coordinates.
     */
    enumDragHandle dragHandleAt(QPoint p);

    /**
     * Returns the rectangle in the thumbnail covered by the given document rectangle.
     */
    QRect documentToThumbnail(const KoRect& docRect);

    /**
     * Returns the rectangle in the document covered by the given thumbnail rectangle.
     */
    KoRect thumbnailToDocument(const QRect& thumbnailRect);

    /**
     * Converts a point in the view to a point in the thumbnail.
     */
    QPoint viewToThumbnail(const QPoint& viewPoint);

private:
    
    WdgBirdEye * m_page;
    
    KoZoomAdapter * m_zoomListener;
    KoThumbnailAdapter * m_thumbnailProvider;
    KoCanvasAdapter * m_canvas;
    
    KAction* m_zoomIn;
    KAction* m_zoomOut;
    QPixmap m_viewBuffer;
    QPixmap m_thumbnail;

    QSize m_documentSize;
    QRect m_visibleAreaInThumbnail;
    bool m_dragging;
    enumDragHandle m_dragHandle;
    QPoint m_lastDragPos;

};

#endif
