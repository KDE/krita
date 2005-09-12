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

#include "kis_previewwidgetbase.h"
#include "kis_previewwidget.h"
#include "kis_previewview.h"

KisPreviewWidget::KisPreviewWidget( QWidget* parent, const char* name )
    : PreviewWidgetBase( parent, name )
{
    m_autoupdate = true;
    m_previewisdiplayed = true;
    connect(m_preview, SIGNAL(updated()), this, SLOT(redirectUpdated()));
    kToolBar1->insertButton("viewmag+",0, true, "zoom+");
    connect(kToolBar1->getButton(0),SIGNAL(clicked()),m_preview,SLOT(zoomIn()));
    kToolBar1->insertButton("viewmag-",1, true, "zoom-");
    connect(kToolBar1->getButton(1),SIGNAL(clicked()),m_preview,SLOT(zoomOut()));
    kToolBar1->insertLineSeparator();
    kToolBar1->insertButton("reload",2, true, "update");
    connect(kToolBar1->getButton(2),SIGNAL(clicked()),this,SLOT(forceUpdate()));
    kToolBar1->insertButton("",3, true, "autoupdate");
    connect(kToolBar1->getButton(3),SIGNAL(clicked()),this,SLOT(toggleAutoUpdate()));
    kToolBar1->insertButton("",4, true, "switch");
    connect(kToolBar1->getButton(4),SIGNAL(clicked()),this,SLOT(toggleImageDisplayed()));
    kToolBar1->insertButton("",5, true, "popup original and preview");
}

void KisPreviewWidget::forceUpdate()
{
    if(m_previewisdiplayed)
        emit updated();
}

void KisPreviewWidget::redirectUpdated() {
    if (m_autoupdate && m_previewisdiplayed)
        emit updated();
}

void KisPreviewWidget::slotSetLayer(KisLayerSP lay)
{
    Q_ASSERT(lay);
    if (!lay) return;
    
    Q_INT32 w = static_cast<Q_INT32>(ceil(size().width() / m_preview->getZoom()));
    Q_INT32 h = static_cast<Q_INT32>(ceil(size().height() / m_preview->getZoom()));
    
    m_sourceImage = new KisImage(0, w, h, lay->colorSpace(), "preview");
    Q_CHECK_PTR(m_sourceImage);
    m_sourceImage->setProfile(lay -> profile());
    m_sourceLayer = new KisLayer( *lay );
    m_sourceLayer->setImage(m_sourceImage);
    m_sourceImage->add(m_sourceLayer, -1);
    
    m_previewImage = new KisImage(0, w, h, lay->colorSpace(), "preview");
    Q_CHECK_PTR(m_previewImage);
    m_previewImage -> setProfile(lay -> profile());
    m_previewLayer = new KisLayer(m_previewImage, m_previewImage -> nextLayerName(), OPACITY_OPAQUE);
    Q_CHECK_PTR(m_previewImage);
    
    KisPainter gc;
    KisPaintDeviceImplSP pd(m_sourceLayer.data());
    gc.begin(m_previewLayer.data());
    
    gc.bitBlt(0, 0, COMPOSITE_OVER, pd, m_preview->getPos().x(), m_preview->getPos().y(), -1, -1);
    gc.end();
    m_previewImage -> add(m_previewLayer, -1);
    slotRenewLayer();
    m_preview->setDisplayImage(m_previewImage);
    redirectUpdated();
    m_previewisdiplayed = true;
}

void KisPreviewWidget::slotRenewLayer() {
    if (!m_previewLayer || !m_sourceLayer) return;

    KisPaintDeviceImplSP pd(m_sourceLayer.data());
    KisPainter gc;
    QPoint delta = m_preview->getPos();
    gc.begin(m_previewLayer.data());
    gc.bltSelection(0, 0, COMPOSITE_COPY, pd, OPACITY_OPAQUE, delta.x(), delta.y(), m_previewImage->width(), m_previewImage->height());
    gc.end();
}

KisLayerSP KisPreviewWidget::getLayer()
{
	return m_previewLayer;
}

void KisPreviewWidget::slotUpdate()
{
    m_preview->updatedPreview();
}

void KisPreviewWidget::slotSetAutoUpdate(bool set) {
    m_autoupdate = set;
}

void KisPreviewWidget::toggleAutoUpdate()
{
    kdDebug() << "m_autoupdate = " << m_autoupdate << endl;
    m_autoupdate = !m_autoupdate;
}

void KisPreviewWidget::toggleImageDisplayed()
{
    if(m_previewisdiplayed)
    {
        kdDebug() << "Display source image" << endl;
        m_preview->setDisplayImage(m_sourceImage);
    } else {
        kdDebug() << "Display preview image" << endl;
        m_preview->setDisplayImage(m_previewImage);
        emit redirectUpdated();
    }
    m_previewisdiplayed = !m_previewisdiplayed;
}


double KisPreviewWidget::getZoom()
{
    return m_preview->getZoom();
}

QPoint KisPreviewWidget::getPos()
{
    return m_preview->getPos();
}

bool KisPreviewWidget::getAutoUpdate() {
    return m_autoupdate;
}
#include "kis_previewwidget.moc"
