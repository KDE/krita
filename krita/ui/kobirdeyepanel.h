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

#include <qwidget.h>

class QPixmap;
class KoDocument;

class WdgBirdEye;

/**
 * The zoom listener interface defines methods that the bird eye
 * panel will call whenever the zoomlevel is changed through one
 * of the panel actions.
 */
class KoZoomListener {

    public:

    KoZoomListener();
    virtual ~KoZoomListener();

    /**
     * Set the zoom level to the specified percentage.
     */
    virtual void zoomTo(int) = 0;
    
    /**
     * Zoom one step in.
     */
    virtual void zoomIn() = 0;
    
    /**
     * Zoom one step out.
     */
    virtual void zoomOut() = 0;
    
    /**
     * Get the minimum zoom percentage that this listener supports.
     */
    virtual int getMinZoom() = 0;

    /**
     * Get the maximum zoom percentage that this listener supports.
     */
    virtual int getMaxZoom() = 0;

};


class KoThumbnailProvider
{
    public:
    
        KoThumbnailProvider();
        ~KoThumbnailProvider();
        
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
         * to 100%; the bird eye widget takes care of zoom levels.
         *
         * @param r the rect that is to be rendered onto the QImage
         */
        virtual QPixmap image(QRect r) = 0;
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
     * @param doc a KOffice document. The initial thumbnail will be generated with paintContents
     * @param parent the parent widget
     * @param name the QObject name of this bird eye widget
     * @param f the widget flags (@see QWidget)
     */
    KoBirdEyePanel( KoZoomListener * zoomListener, 
                    KoThumbnailProvider * thumbnailProvider,
                    QWidget * parent,
                    const char * name = 0,
                    WFlags f = 0 );

    virtual ~KoBirdEyePanel();

public slots:

    void setZoomListener( KoZoomListener * zoomListener) { m_zoomListener = zoomListener; }
    
    void setThumbnailProvider( KoThumbnailProvider * thumbnailProvider ) { m_thumbnailProvider = thumbnailProvider; }
    /**
     * Connect to this slot to inform the bird's eye view of changes in
     * the zoom level of your canvas or view. The value is taken as a percentage,
     * with 100 being zoomed to 100% of size.
     */
    void slotCanvasZoomChanged(int);
    
    /**
     * Connect to this slot if a (rectangular) area of your document is changed.
     * 
     * @param r The rect that has been changed: this is unzoomed.
     * @param img An image that contains the unzoomed new data for rect r
     * @param docrect The boundaries of the entire document we thumbnail 
     */
    void slotUpdate(const QRect & r, const QImage & img, const QRect & docrect);
    
private:
    
    WdgBirdEye * m_page;
    
    KoZoomListener * m_zoomListener;
    KoThumbnailProvider * m_thumbnailProvider;
    
    QPixmap * m_pixmap;
};

#endif
