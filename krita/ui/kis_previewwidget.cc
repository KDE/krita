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

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qcursor.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kpushbutton.h>

#include <kis_cursor.h>
#include <kis_colorspace.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_config.h>
#include <kis_filter_strategy.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_meta_registry.h>
#include <kis_painter.h>
#include <kis_profile.h>
#include <kis_types.h>
#include <kis_undo_adapter.h>

#include "kis_previewwidgetbase.h"
#include "kis_previewwidget.h"
#include "imageviewer.h"

KisPreviewWidget::KisPreviewWidget( QWidget* parent, const char* name )
    : PreviewWidgetBase( parent, name )/*, m_image(0)*/
{
    m_autoupdate = true;
    m_previewIsDisplayed = true;

    btnZoomIn->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "viewmag+", KIcon::MainToolbar, 16 ));
    connect(btnZoomIn, SIGNAL(clicked()), this, SLOT(zoomIn()));
    btnZoomOut->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "viewmag-", KIcon::MainToolbar, 16 ));
    connect(btnZoomOut, SIGNAL(clicked()), this, SLOT(zoomOut()));
    btnUpdate->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "reload", KIcon::MainToolbar, 16 ));
    connect(btnUpdate, SIGNAL(clicked()), this, SLOT(forceUpdate()));

    connect(radioBtnPreview, SIGNAL(toggled(bool)), this, SLOT(setPreviewDisplayed(bool)));

    connect(checkBoxAutoUpdate, SIGNAL(toggled(bool)), this, SLOT(slotSetAutoUpdate(bool)));
    btnZoomOneToOne->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "viewmag1", KIcon::MainToolbar, 16 ));
    connect(btnZoomOneToOne, SIGNAL(clicked()), this, SLOT(zoomOneToOne()));



/*    kToolBar1->insertLineSeparator();
    kToolBar1->insertButton("reload",2, true, i18n("Update"));
    connect(kToolBar1->getButton(2),SIGNAL(clicked()),this,SLOT(forceUpdate()));

    kToolBar1->insertButton("",3, true, i18n("Auto Update"));
    connect(kToolBar1->getButton(3),SIGNAL(clicked()),this,SLOT(toggleAutoUpdate()));

    kToolBar1->insertButton("",4, true, i18n("Switch"));
    connect(kToolBar1->getButton(4),SIGNAL(clicked()),this,SLOT(toggleImageDisplayed()));*/
// these currently don't yet work, reenable when they do work :)  (TZ-12-2005)
// TODO reenable these
//   kToolBar1->insertButton("",5, true, i18n("Popup Original and Preview"));
}

void KisPreviewWidget::forceUpdate()
{
    if (!m_origDevice) return;
    if(m_previewIsDisplayed)
    {
        m_groupBox->setTitle(m_origDevice->name());
        emit updated();
    }
}

void KisPreviewWidget::slotSetDevice(KisPaintDeviceSP dev)
{
    Q_ASSERT( dev );

    if (!dev) return;

    m_origDevice = dev;

    KisConfig cfg;
    QString monitorProfileName = cfg.monitorProfile();
    m_profile = KisMetaRegistry::instance()->csRegistry()->getProfileByName(monitorProfileName);

    QRect r = dev->exactBounds();

    m_groupBox->setTitle(i18n("Preview: ") + dev->name());
    m_previewIsDisplayed = true;

    m_zoom = (double)m_preview->width() / (double)r.width();
    zoomChanged();
}


KisPaintDeviceSP KisPreviewWidget::getDevice()
{
    return m_previewDevice;
}

void KisPreviewWidget::slotUpdate()
{
    QRect r = m_previewDevice->exactBounds();
    m_scaledPreview = m_previewDevice->convertToQImage(m_profile, 0, 0, r.width(), r.height());
    if(m_zoom > 1.0)
    {
        int w, h;
        w = (int) ceil(r.width() * m_zoom );
        h = (int) ceil(r.height() * m_zoom );
        m_scaledPreview = m_scaledPreview.smoothScale (w,h, QImage::ScaleMax);
    }
    if(m_previewIsDisplayed)
    {
        m_preview->setImage(m_scaledPreview);
    }
}

void KisPreviewWidget::slotSetAutoUpdate(bool set) {
    m_autoupdate = set;
}

void KisPreviewWidget::wheelEvent(QWheelEvent * e)
{
    if (e->delta() > 0)
        zoomIn();
    else
        zoomOut();
    e->accept();
}

void KisPreviewWidget::setPreviewDisplayed(bool v)
{
    if (!m_origDevice) return;
    if (!m_preview) return;
    if (m_scaledPreview == 0) return;

    m_previewIsDisplayed = v;
    if(m_previewIsDisplayed)
    {
        m_groupBox->setTitle(i18n("Preview: ") + m_origDevice->name());
        m_preview->setImage(m_scaledPreview);
    } else {
        m_groupBox->setTitle(i18n("Original: ") + m_origDevice->name());
        m_preview->setImage(m_scaledOriginal);
    }
}

void KisPreviewWidget::needUpdate()
{
    if(m_previewIsDisplayed)
        m_groupBox->setTitle(i18n("Preview (needs update)"));
}

bool KisPreviewWidget::getAutoUpdate()  const {
    return m_autoupdate;
}

bool KisPreviewWidget::zoomChanged()
{
    QApplication::setOverrideCursor(KisCursor::waitCursor());
    if (!m_origDevice) return false;

    QRect r = m_origDevice->exactBounds();
    int w = (int) ceil(r.width() * m_zoom );
    int h = (int) ceil(r.height() * m_zoom );

    if( w == 0 || h == 0 )
        return false;

    if(m_zoom < 1.0) // if m_zoom > 1.0, we will scale after applying the filter
    {
        m_previewDevice = m_origDevice->createThumbnailDevice(w, h); 
    }
    else {
        m_previewDevice = new KisPaintDevice( *m_origDevice );
    }
    
    m_scaledOriginal = m_previewDevice->convertToQImage(m_profile, 0, 0, w, h);


    if(!m_previewIsDisplayed)
    {
        m_preview->setImage(m_scaledOriginal);
    }
    
    emit updated();

    QApplication::restoreOverrideCursor();

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

void KisPreviewWidget::zoomOneToOne() {
    double oldZoom = m_zoom;
    m_zoom = 1;
    if( !zoomChanged() ) m_zoom = oldZoom;
}


#include "kis_previewwidget.moc"
