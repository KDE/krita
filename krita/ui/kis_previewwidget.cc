/*
 *  kis_previewwidget.cc - part of Krita
 *
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

#include <koColor.h>

#include "kis_undo_adapter.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "color_strategy/kis_strategy_colorspace.h"

#include "kis_previewwidget.h"
#include "dialogs/kis_previewwidgetbase.h"

/*FIXME make this unnecessary */
#define SIZE 150

KisPreviewWidget::KisPreviewWidget( QWidget* parent, const char* name )
    : PreviewWidgetBase( parent, name ), m_undo(0), /* seems to work nicely */
      m_pixmap(RENDER_WIDTH, RENDER_HEIGHT)
{
	connect(pushButton1/*plus*/, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(pushButton2/*minus*/, SIGNAL(clicked()), this, SLOT(zoomOut()));
	m_pos = QPoint(0,0);
	m_zoom = 1;
}

void KisPreviewWidget::slotSetPreview(KisLayerSP lay)
{
	m_layer = lay;
	KisPainter gc;
	KisPaintDeviceSP pd(m_layer.data());

	/* left original image */
	KisImageSP img1;
	img1 = new KisImage(m_undo, SIZE, SIZE, lay->colorStrategy(), "previewOriginal");
	layerNew1 = new KisLayer(img1, SIZE, SIZE, img1 -> nextLayerName(), OPACITY_OPAQUE);
	gc.begin(layerNew1.data());
	
	gc.bitBlt(0, 0, COMPOSITE_OVER, pd);
	gc.end();
	img1 -> add(layerNew1, -1);
	m_original = img1;

	/* right preview image */
	KisImageSP img2;
	img2 = new KisImage(m_undo, SIZE, SIZE, lay->colorStrategy(), "previewPreview");
	layerNew2 = new KisLayer(img2, SIZE, SIZE, img2 -> nextLayerName(), OPACITY_OPAQUE);
	gc.begin(layerNew2.data());
	gc.bitBlt(0, 0, COMPOSITE_OVER, pd);
	gc.end();
	img2 -> add(layerNew2, -1);
	m_preview = img2;

	repaint();
	emit updated();
}

void KisPreviewWidget::zoomIn() {
	if (m_zoom * 1.5 < 8) {
		m_zoom *= 1.5;
		clampDelta(m_pos);
		updateWidgets(m_pos);
		emit updated();
	}
}

void KisPreviewWidget::zoomOut() {
	if (m_zoom / 1.5 > 1/8) {
		m_zoom /= 1.5;
		clampDelta(m_pos);
		updateWidgets(m_pos);
		emit updated();
	}
}

void KisPreviewWidget::slotRenewLayer() {
	updateWidgets(m_pos);
}

KisLayerSP KisPreviewWidget::getLayer()
{
	return layerNew2;
}

void KisPreviewWidget::slotUpdate()
{
	repaint();
}

void KisPreviewWidget::paintEvent(QPaintEvent*)
{
	QPainter paintOriginal(m_originalWidget);
	render(paintOriginal, m_original, m_zoom, m_zoom);
	QPainter paintPreview(m_previewWidget);
	render(paintPreview, m_preview, m_zoom, m_zoom);
}

void KisPreviewWidget::mousePressEvent(QMouseEvent * e)
{
	m_startDrag = QPoint(static_cast<int>(e->pos().x()/m_zoom),
		static_cast<int>(e->pos().y()/m_zoom));
}

void KisPreviewWidget::mouseMoveEvent(QMouseEvent * e)
{
	if (! m_original || ! m_preview)
		return;

	QPoint zoomedPos(static_cast<int>(e->pos().x()/m_zoom),
		static_cast<int>(e->pos().y()/m_zoom));

	QPoint delta = m_pos - (zoomedPos - m_startDrag);
	clampDelta(delta);
	updateWidgets(delta);
	repaint();
}

void KisPreviewWidget::mouseReleaseEvent(QMouseEvent * e)
{
	mouseMoveEvent(e);
	QPoint zoomedPos(static_cast<int>(e->pos().x()/m_zoom),
		static_cast<int>(e->pos().y()/m_zoom));
	m_pos -= zoomedPos - m_startDrag;
	clampDelta(m_pos);

	emit updated();
}

void KisPreviewWidget::updateWidgets(QPoint delta)
{
	if (!m_layer) return;

	KisPainter gc;
	KisPaintDeviceSP pd(m_layer.data());

	/* left */
	gc.begin(layerNew1.data());
	gc.bitBlt(0, 0, COMPOSITE_OVER, pd, delta.x(), delta.y(), -1, -1);
	gc.end();

	/* right */
	gc.begin(layerNew2.data());
	gc.bitBlt(0, 0, COMPOSITE_OVER, pd, delta.x(), delta.y(), -1, -1);
	gc.end();
}

void KisPreviewWidget::clampDelta(QPoint& delta)
{
	if (delta.x() < 0)
		delta.rx() = 0;
	if (delta.y() < 0)
		delta.ry() = 0;
	if (delta.x() + SIZE / m_zoom >= m_layer -> width())
		delta.rx() = m_layer -> width() - static_cast<int>(SIZE / m_zoom) - 1;
	if (delta.y() + SIZE / m_zoom >= m_layer -> height())
		delta.ry() = m_layer -> height() - static_cast<int>(SIZE / m_zoom) - 1;
}

void KisPreviewWidget::render(QPainter &painter, KisImageSP image, double zoomX, double zoomY)
{
	if( image == 0 ) // This is usefull only for Qt/Designer
		return;
	Q_INT32 x1 = 0;
	Q_INT32 y1 = 0;
	Q_INT32 x2 = image -> width();
	Q_INT32 y2 = image -> height();
	Q_INT32 tileno;

	if (!image)
		return;

	if (zoomX != 1.0 || zoomY != 1.0)
		painter.scale(zoomX, zoomY);

	for (Q_INT32 y = y1; y <= y2; y += TILE_HEIGHT - (y % TILE_HEIGHT)) {
		for (Q_INT32 x = x1; x <= x2; x += TILE_WIDTH - (x % TILE_WIDTH)) {
			if ((tileno = image -> tileNum(x, y)) < 0)
				continue;

			image -> renderToProjection(tileno);
		}
	}

	for (Q_INT32 y = y1; y < y2; y += RENDER_HEIGHT)
		for (Q_INT32 x = x1; x < x2; x += RENDER_WIDTH) {
			Q_INT32 w = QMIN(x2 - x, RENDER_WIDTH);
			Q_INT32 h = QMIN(y2 - y, RENDER_HEIGHT);

			QImage img = image -> projection() -> convertToImage(x, y, w, h);

			if (!img.isNull()) {
				m_pixio.putImage(&m_pixmap, 0, 0, &img);
				painter.drawPixmap(x, y, m_pixmap, 0, 0, w, h);
			}
		}
}

#include "kis_previewwidget.moc"
