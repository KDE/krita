/*
 *  kis_previewwidget.h - part of Krita
 *
 *  Copyright (c) 2001 John Califf  <jcaliff@compuzone.net>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Benjamin Schleimer <bensch128@yahoo.com>
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
#ifndef __kis_previewwidget_h__
#define __kis_previewwidget_h__

#include <qimage.h>
#include <qevent.h>

#include "kis_types.h"

#include "kis_previewwidgetbase.h"

class QWidget;
class KisProfile;
class KisFilter;
class KisFilterConfiguration;
class QTimer;
class KisLabelProgress;

/**
 * A widget that can be used by plugins to show a preview of the effect of the
 * plugin to the user. This is a convenience class thand handily packs a source and a
 * preview view together with a zoom button.
 * It would be nice if every plugin that needs to show a preview
 * (maybe not those that create a new image) would use this. This can prevent the distracting
 * effect the GIMP has with a different preview for almost every filter.
 */
class KisPreviewWidget : public PreviewWidgetBase
{
    Q_OBJECT

public:
    /** Constructs the widget */
    KisPreviewWidget( QWidget* parent = 0, const char* name = 0 );
    virtual ~KisPreviewWidget();

    /** returns if the preview is automatically updated */
    bool getAutoUpdate() const;

    void wheelEvent(QWheelEvent * e);

    /** Instructs the KisPreviewWidget to eventually update the preview. 
     * KisPreviewWidget delays the actual running of the filter for 500ms 
     * so if the user is changing a configuration setting, it won't run multiple time.
     * @param filter to run on the image
     * @config to use when filtering.
     */
    void runFilter(KisFilter * filter, KisFilterConfiguration * config);

public slots:

    /** Sets the preview to use the layer specified as argument */
    void slotSetDevice(KisPaintDeviceSP dev);

    /** Enables or disables the automatically updating of the preview */
    void slotSetAutoUpdate(bool set);

    /** Toggle between display preview and display original */
    void setPreviewDisplayed(bool v);
    
    /** use to indicate that the preview need to be updated. */
    void needUpdate();
    
signals:
    /** This is emitted when the position or zoom factor of the widget has changed */
    void updated();

private slots:

    void zoomIn();
    void zoomOut();
    void zoomOneToOne();

    /**
     * Called when the "Force Update" button is clicked
     */
    void forceUpdate();

    /**
     * Updates the zoom and redisplays either the original or the preview (filtered) image
     */
    void updateZoom();

    /** Internal method which actually runs the filter
     */
    void runFilterHelper();

private:
    /**
     * Recalculates the zoom factor
     */
    void zoomChanged(const double zoom);

    bool m_autoupdate; /// Flag indicating that the widget should auto update whenever a setting is changed
    bool m_previewIsDisplayed; /// Flag indicating whether the filtered or original image is displayed

    QImage m_scaledOriginal; /// QImage copy of the original image
    bool m_dirtyOriginal; /// flag indicating that the original image is dirty
    KisPaintDeviceSP m_origDevice; /// Pointer to the original image
    
    QImage m_scaledPreview; /// QImage copy of the filtered image
    bool m_dirtyPreview; /// flag indicating that the preview image is dirty
    KisPaintDeviceSP m_previewDevice; /// Pointer to the preview image
    KisImageSP m_scaledImage; /// Scaled image copied from the original

    double m_filterZoom; /// Zoom amount when the filtering occurred
    double m_zoom; /// Current zoom amount
    KisProfile * m_profile; /// the color profile to use when converting to QImage

    KisLabelProgress *m_progress; /// Progress bar of the preview.

    QTimer * m_zoomTimer; /// Timer used to update the view whenever the zoom changes
    QTimer * m_filterTimer; /// Timer used to update the view whenever the filter changes
    KisFilter * m_filter; /// Filter used
    KisFilterConfiguration * m_config; /// Configuration used
    bool m_firstFilter; /// Flag to determine if we should delay the first filter or not
    bool m_firstZoom; ///  Flag to determine if we should delay the first zoom or not
};

#endif
