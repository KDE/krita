/*
 *  kis_previewwidget.cc - part of Krita
 *
 *  Copyright (c) 2001 John Califf  <jwcaliff@compuzone.net>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qpainter.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcolor.h>

#include <kdebug.h>

#include "kis_undo_adapter.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "color_strategy/kis_strategy_colorspace.h"

#include "kis_previewwidget.h"
#include "dialogs/kis_previewwidgetbase.h"

// The View
KisPreviewView::KisPreviewView(QWidget* parent, const char * name, WFlags f)
	: QWidget(parent, name, f), m_undo(0), /* seems to work nicely */
	  m_pos(QPoint(0,0)), m_zoom(1.0)
{
	m_moving = false;
	updateView(m_pos);
}

KisLayerSP KisPreviewView::getSourceLayer()
{
	return m_sourcelayer;
}

KisLayerSP KisPreviewView::getPreviewLayer()
{
	return m_clippedview;
}

void KisPreviewView::updateView()
{
	updateView(m_pos);
}

void KisPreviewView::updateView(QPoint delta)
{
	if (!m_clippedview || !m_sourcelayer) return;

	KisPainter gc;
	KisPaintDeviceSP pd(m_sourcelayer.data());

	gc.begin(m_clippedview.data());
	gc.bitBlt(0, 0, COMPOSITE_OVER, pd, delta.x(), delta.y(), -1, -1);
	gc.end();
}

void KisPreviewView::setSourceLayer(KisLayerSP lay)
{
	m_sourcelayer = lay;
	KisPainter gc;
	KisPaintDeviceSP pd(m_sourcelayer.data());
    
	Q_INT32 w = static_cast<Q_INT32>(size().width() / m_zoom);
	Q_INT32 h = static_cast<Q_INT32>(size().height() / m_zoom);

	m_image = new KisImage(m_undo, w, h, lay->colorStrategy(), "preview");

	m_image -> setProfile(lay -> profile());
	m_clippedview = new KisLayer(m_image, m_image -> nextLayerName(), OPACITY_OPAQUE);
	gc.begin(m_clippedview.data());
	
	gc.bitBlt(0, 0, COMPOSITE_OVER, pd, m_pos.x(), m_pos.y(), -1, -1);
	gc.end();
	m_image -> add(m_clippedview, -1);
	update();
	emit updated();
}

void KisPreviewView::setZoom(double zoom) {
	m_zoom = zoom;
	clampDelta(m_pos);
	setSourceLayer(m_sourcelayer); // so that it automatically resizes m_clippedview
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
	update();
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
	image -> renderToPainter(0, 0, image -> width(), image -> height(), painter);

}

void KisPreviewView::clampDelta(QPoint& delta)
{
#if 0 //AUTOLAYER
    if (delta.x() < 0)
        delta.rx() = 0;
    if (delta.y() < 0)
        delta.ry() = 0;
    if (delta.x() + size().width() / m_zoom >= m_sourcelayer -> width())
        delta.rx() = m_sourcelayer -> width() - static_cast<int>(size().width() / m_zoom) - 1;
    if (delta.y() + size().height() / m_zoom >= m_sourcelayer -> height())
        delta.ry() = m_sourcelayer -> height() - static_cast<int>(size().height() / m_zoom) - 1;
#endif // AUTOLAYER
}

void KisPreviewView::slotStartMoving(QPoint startDrag)
{
	m_startDrag = startDrag;
}

void KisPreviewView::slotMoving(QPoint zoomedPos)
{
	QPoint delta = m_pos - (zoomedPos - m_startDrag);
	m_moving = true;
	clampDelta(delta);
	updateView(delta);
	update();
}

void KisPreviewView::slotMoved(QPoint zoomedPos)
{
	m_pos -= zoomedPos - m_startDrag;
	clampDelta(m_pos);
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
	clampDelta(m_pos);
	setSourceLayer(m_sourcelayer); // so that it automatically resizes m_clippedview
}


// The Standard Preview Widget

KisPreviewWidget::KisPreviewWidget( QWidget* parent, const char* name )
	: PreviewWidgetBase( parent, name )
{
	connect(m_preview, SIGNAL(updated()), this, SLOT(redirectUpdated()));
}

void KisPreviewWidget::redirectUpdated() {
	emit updated();
}

void KisPreviewWidget::slotSetLayer(KisLayerSP lay)
{
	m_original->setSourceLayer(lay);
	m_preview->setSourceLayer(lay);
}

void KisPreviewWidget::slotRenewLayer() {
	m_preview->updateView();
}

KisLayerSP KisPreviewWidget::getLayer()
{
	return m_preview->getPreviewLayer();
}

void KisPreviewWidget::slotUpdate()
{
	m_preview->updatedPreview();
}

double KisPreviewWidget::getZoom()
{
	return m_preview->getZoom();
}

QPoint KisPreviewWidget::getPos()
{
	return m_preview->getPos();
}
#include "kis_previewwidget.moc"
