/*
 *  kis_previewwidget.cc - part of Krita
 *
 *  Copyright (c) 2001 John Califf  <jwcaliff@compuzone.net>
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
#include <qpainter.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qgroupbox.h>

#include <kdebug.h>
#include <ktoolbarbutton.h>
#include <ktoolbar.h>

#include "kis_undo_adapter.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_colorspace.h"
#include "kis_meta_registry.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_config.h"
#include "kis_profile.h"

#include "kis_previewwidgetbase.h"
#include "kis_previewwidget.h"
#include "imageviewer.h"

KisPreviewWidget::KisPreviewWidget( QWidget* parent, const char* name )
    : PreviewWidgetBase( parent, name )
{
    m_image = new KisImage(0, 0, 0, KisMetaRegistry::instance()->csRegistry()->getRGB8(), "preview image");
    m_autoupdate = true;
    m_previewIsDisplayed = true;

    kToolBar1->insertButton("viewmag+",0, true, "Zoom In");
    connect(kToolBar1->getButton(0),SIGNAL(clicked()), this, SLOT(zoomIn()));
    
    kToolBar1->insertButton("viewmag-",1, true, "Zoom Out");
    connect(kToolBar1->getButton(1),SIGNAL(clicked()), this, SLOT(zoomOut()));

// these currently don't yet work, reenable when they do work :)  (TZ-12-2005)
// TODO reenable these
//   kToolBar1->insertLineSeparator();
//   kToolBar1->insertButton("reload",2, true, "Update");
//   connect(kToolBar1->getButton(2),SIGNAL(clicked()),this,SLOT(forceUpdate()));

//   kToolBar1->insertButton("",3, true, "Auto Update");
//   connect(kToolBar1->getButton(3),SIGNAL(clicked()),this,SLOT(toggleAutoUpdate()));

//   kToolBar1->insertButton("",4, true, "Switch");
//   connect(kToolBar1->getButton(4),SIGNAL(clicked()),this,SLOT(toggleImageDisplayed()));
//   kToolBar1->insertButton("",5, true, "Popup Original and Preview");
}

void KisPreviewWidget::forceUpdate()
{
    kdDebug() << "forceUpdate\n";
    if(m_previewIsDisplayed)
    {
        m_groupBox->setTitle(m_origDevice->name());
        emit updated();
    }
}

void KisPreviewWidget::slotSetDevice(KisPaintDeviceImplSP dev)
{
    //kdDebug() << "slotSetLayer\n";
    
    Q_ASSERT(dev);
    if (!dev) return;

    m_origDevice = dev;
    
    KisConfig cfg;
    QString monitorProfileName = cfg.monitorProfile();
    m_profile = KisMetaRegistry::instance()->csRegistry() -> getProfileByName(monitorProfileName);

    QRect r = dev->exactBounds();
    //kdDebug() << "layer size: " << r.width() << ", " << r.height() << "\n";
    
    m_unscaledSource = dev->convertToQImage(m_profile, 0, 0, r.width(), r.height());
    //kdDebug() << "preview size: " << m_preview->width() << ", "  << m_preview->height() << "\n";

    m_groupBox->setTitle(dev->name());
    m_previewIsDisplayed = true;

    m_zoom = (double)m_preview->width() / (double)m_unscaledSource.width();
    zoomChanged();
    //kdDebug() << "initial zoom = " << m_zoom << "\n";
}


KisPaintDeviceImplSP KisPreviewWidget::getDevice()
{
    //kdDebug() << "getLayer\n";
    return m_previewDevice;
}

void KisPreviewWidget::slotUpdate()
{
    //kdDebug() << "slotUpdate\n";
    m_scaledPreview = m_previewDevice->convertToQImage(m_profile, 0, 0, m_scaledPreview.width(), m_scaledPreview.height());
    m_preview->setImage(m_scaledPreview);
}

void KisPreviewWidget::slotSetAutoUpdate(bool set) {
    //kdDebug() << "slotSetAutoUpdate\n";
    m_autoupdate = set;
}

void KisPreviewWidget::toggleAutoUpdate()
{
    //kdDebug() << "m_autoupdate = " << m_autoupdate << endl;
    m_autoupdate = !m_autoupdate;
}

void KisPreviewWidget::toggleImageDisplayed()
{
    //kdDebug() << "toggleImageDisplayed\n";
    if(m_previewIsDisplayed)
    {
        m_groupBox->setTitle(i18n("Original"));
        //m_preview->setDisplayImage(m_unscaledSourceImage);
    } else {
        m_groupBox->setTitle(i18n("Preview"));
        //m_preview->setDisplayImage(m_previewImage);
    }
    m_previewIsDisplayed = !m_previewIsDisplayed;
}

void KisPreviewWidget::needUpdate()
{
    //kdDebug() << "needUpdate\n";
    if(m_previewIsDisplayed)
        m_groupBox->setTitle(i18n("Preview (needs update)"));
}

bool KisPreviewWidget::getAutoUpdate()  const {
    return m_autoupdate;
}

bool KisPreviewWidget::zoomChanged()
{
    kdDebug() << "zoomChanged " << m_zoom << "\n";
    int w, h;
    w = (int) (m_unscaledSource.width() * m_zoom + 0.5);
    h = (int) (m_unscaledSource.height() * m_zoom + 0.5);

    kdDebug() << "   width: " << w << "\n";
    kdDebug() << "   height: " << h << "\n";
    if( w == 0 || h == 0 )
	   return false; 
    m_scaledPreview = m_unscaledSource.smoothScale(w, h, QImage::ScaleMax);

    m_image->resize(m_scaledPreview.width(), m_scaledPreview.height());
    m_previewDevice = new KisPaintDeviceImpl(m_image, m_image->colorSpace());
    m_previewDevice->convertFromQImage(m_scaledPreview, ""); //should perhaps be m_profile->name
    
    emit updated();
    return true;
 }

void KisPreviewWidget::zoomIn() {
    double oldZoom = m_zoom;
    if (m_zoom > 0 && m_zoom * 1.5 < 8) {
        m_zoom = m_zoom * 1.5;
        if( !zoomChanged() )
	    m_zoom = oldZoom;
    }
}

void KisPreviewWidget::zoomOut() {
    double oldZoom = m_zoom; 
    if (m_zoom > 0 && m_zoom / 1.5 > 1/8) {
        m_zoom = m_zoom / 1.5;
	if( !zoomChanged() )
	   m_zoom = oldZoom;
   }
}



#include "kis_previewwidget.moc"
