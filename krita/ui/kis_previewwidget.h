/*
 *  kis_previewwidget.h - part of Krita
 *
 *  Copyright (c) 2001 John Califf  <jcaliff@compuzone.net>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

    /** @return the scaled down copy of the layer, so the dialog can apply its effect on it. */
    KisPaintDeviceSP getDevice();

    /** returns if the preview is automatically updated */
    bool getAutoUpdate() const;

    void wheelEvent(QWheelEvent * e);
    
public slots:

    /** Sets the preview to use the layer specified as argument */
    void slotSetDevice(KisPaintDeviceSP dev);

    /**
     * Call this when the effect has finished updating the layer. Makes the preview
     * repaint itself. */
    void slotUpdate();

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

    void forceUpdate();
    
private:

    bool zoomChanged();
    
    bool m_autoupdate, m_previewIsDisplayed;

    QImage m_scaledOriginal;
    QImage m_scaledPreview;
    KisPaintDeviceSP m_previewDevice;
    
    double m_zoom;
    KisProfile * m_profile;

    KisPaintDeviceSP m_origDevice;
};

#endif
