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

#include <qrect.h>
#include <qwidget.h>

#include <KoPoint.h>
#include <KoRect.h>

class QPixmap;
class KAction;
class KoDocument;
class WdgBirdEye;


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
        ~KoThumbnailAdapter();
        
        /**
         * Returns the size of the document in pixels.
         * If the document is a KoDocument that uses a KoPageLayout, the same
         * formula as in the generatePreview() method should be used to go from points
         * to pixels.
         *
         * @returns the size in pixels.
         */
        virtual QRect pixelSize() = 0;
        
        /**
         * Returns the specified rectangle as a QImage. The image should zoomed
         * to 100%; the bird eye widget takes care of fitting it into the frame.
         *
         * @param r the rect that is to be rendered onto the QImage
         */
        virtual QImage image(QRect r) = 0;
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
                    WFlags f = 0 );

    virtual ~KoBirdEyePanel();

    bool eventFilter(QObject*, QEvent*);

public slots:

    void setZoomListener( KoZoomAdapter * zoomListener) { m_zoomListener = zoomListener; }
    
    void setThumbnailProvider( KoThumbnailAdapter * thumbnailProvider ) { m_thumbnailProvider = thumbnailProvider; }
    /**
     * Connect to this slot to inform the bird's eye view of changes in
     * the zoom level of your canvas or view. The value is taken as a percentage,
     * with 100 being zoomed to 100% of size.
     */
    void slotCanvasZoomChanged(int);
    
    void cursorPosChanged(Q_INT32 xpos, Q_INT32 ypos);

    void zoomMinus();
    void zoomPlus();

    /**
     * Connect to this slot if a (rectangular) area of your document is changed.
     * 
     * @param r The rect that has been changed: this is unzoomed.
     * @param img An image that contains the unzoomed new data for rect r
     * @param docrect The boundaries of the entire document we thumbnail 
     */
    void slotUpdate(const QRect & r, const QImage & img, const QRect & docrect);

protected slots:

    void updateVisibleArea();
    void zoomValueChanged(int zoom);
    void zoom100();
    void sliderChanged(int);
    
protected:
    void setZoom(int zoom);
            
    void updateView();
    void handleMouseMove(QPoint);
    void handleMouseMoveAction(QPoint);
    void handleMousePress(QPoint);

private:
    
    WdgBirdEye * m_page;
    
    KoZoomAdapter * m_zoomListener;
    KoThumbnailAdapter * m_thumbnailProvider;
    KoCanvasAdapter * m_canvas;
    
    KAction* m_zoomIn;
    KAction* m_zoomOut;
    QPixmap m_buffer;

    QRect m_visibleArea;
    AlignmentFlags m_aPos;
    bool m_handlePress;
    QPoint m_lastPos;

};

#endif
