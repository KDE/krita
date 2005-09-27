/*
 *  kis_previewview.cc - part of Krita
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

#include <math.h>

#include <qpainter.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcolor.h>

#include <kdebug.h>
#include <kglobalsettings.h>

#include "kis_undo_adapter.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_config.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_profile.h"

#include "kis_colorspace.h"

#include "kis_previewview.h"

// The View
KisPreviewView::KisPreviewView(QWidget* parent, const char * name, WFlags f)
    : QWidget(parent, name, f), /* seems to work nicely */
      m_pos(QPoint(0,0)), m_zoom(1.0)
{
    m_moving = false;
}

void KisPreviewView::setDisplayImage(KisImageSP i)
{
    i->notify();
    if (m_image != 0) {
        //QObject::disconnect(m_image, SIGNAL(sigImageUpdated(KisImageSP, rc)), this);
    }
    connect(m_image, SIGNAL(sigImageUpdated(KisImageSP, QRect)), this, SLOT(slotUpdate(KisImageSP, QRect)));

    m_image = i;
    
    updatedPreview();
}


void KisPreviewView::setZoom(double zoom) {
    m_zoom = zoom;
    emit updated();
}

void KisPreviewView::zoomIn() {
    if (m_zoom * 1.5 < 8) {
        setZoom(m_zoom * 1.5);
    }
}

void KisPreviewView::zoomOut() {
    if (m_zoom / 1.5 > 1/8) {
        setZoom(m_zoom / 1.5);
    }
}

void KisPreviewView::updatedPreview() {
    repaint(false);
}

void KisPreviewView::render(QPainter &painter, KisImageSP image)
{
    if( image == 0 ) // This is usefull only for Qt/Designer
        return;

    if (!image)
        return;

    if (m_zoom != 1.0)
        painter.scale(m_zoom, m_zoom);

    // XXX: Do we always need to render the complete image?
    KisConfig cfg;
    QString monitorProfileName = cfg.monitorProfile();

    KisProfile *  monitorProfile = KisColorSpaceFactoryRegistry::instance() -> getProfileByName(monitorProfileName);
    painter.fillRect(0, 0, width(), height(), KGlobalSettings::baseColor());
    image -> renderToPainter(0, 0, image -> width(), image -> height(), painter, monitorProfile);

}

void KisPreviewView::slotStartMoving(QPoint startDrag)
{
    m_startDrag = startDrag;
}

void KisPreviewView::slotMoving(QPoint zoomedPos)
{
	QPoint delta = m_pos - (zoomedPos - m_startDrag);
	m_moving = true;
	repaint(false);
}

void KisPreviewView::slotMoved(QPoint zoomedPos)
{
    m_pos -= zoomedPos - m_startDrag;
    m_moving = false;

    emit updated();
}

void KisPreviewView::paintEvent(QPaintEvent*)
{
    setUpdatesEnabled(false);
    QPainter painter(this);
    render(painter, m_image);
    setUpdatesEnabled(true);
}

void KisPreviewView::mouseMoveEvent(QMouseEvent * e)
{
    QPoint zoomedPos(static_cast<int>(e->pos().x()/m_zoom),
             static_cast<int>(e->pos().y()/m_zoom));

    slotMoving(zoomedPos);
    emit moving(zoomedPos);
}

void KisPreviewView::mousePressEvent(QMouseEvent * e)
{
    m_startDrag = QPoint(static_cast<int>(e->pos().x()/m_zoom),
                 static_cast<int>(e->pos().y()/m_zoom));
    emit startMoving(m_startDrag);
}

void KisPreviewView::mouseReleaseEvent(QMouseEvent * e)
{
    mouseMoveEvent(e);
    QPoint zoomedPos(static_cast<int>(e->pos().x()/m_zoom),
             static_cast<int>(e->pos().y()/m_zoom));
    slotMoved(zoomedPos);
    emit moved(zoomedPos);
}

void KisPreviewView::resizeEvent(QResizeEvent *) {
    emit updated();
}

void KisPreviewView::slotUpdate(KisImageSP img, QRect r) 
{
    kdDebug() << "slotUpdate called with rect: " << r.x() << ", " << r.y() << ", " << r.width() << ", " << r.height() << "\n";
    // Assume that the preview image is just as big as the image we're previewing.
    if (m_image) m_image->notify(r);
}

#include "kis_previewview.moc"
