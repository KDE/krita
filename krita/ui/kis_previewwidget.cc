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
    : PreviewWidgetBase( parent, name ), m_undo(0) /* seems to work nicely */
{
	connect(pushButton1/*plus*/, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(pushButton2/*minus*/, SIGNAL(clicked()), this, SLOT(zoomOut()));
	m_pos = QPoint(0,0);
	m_moved = false;
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
		m_startDrag *= 1.5;
		updateWidgets(m_startDrag);
		emit updated();
	}
}

void KisPreviewWidget::zoomOut() {
	if (m_zoom / 1.5 > 1/8) {
		m_zoom /= 1.5;
		m_startDrag /= 1.5;
		updateWidgets(m_startDrag);
		emit updated();
	}
}

void KisPreviewWidget::slotRenewLayer() {
	updateWidgets(m_startDrag);
}

KisLayerSP KisPreviewWidget::getLayer()
{
	return layerNew2;
}

void KisPreviewWidget::slotUpdate()
{
	m_preview->invalidate();
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
	if(!m_moved) {
		m_startDrag = e->pos();
		m_moved = true;
	} else {
		m_startDrag = e->pos() - m_startDrag;
	}
}

void KisPreviewWidget::mouseMoveEvent(QMouseEvent * e)
{
	if (! m_original || ! m_preview)
		return;
	QPoint delta = (e->pos() - m_startDrag); // m_zoom;
	if (delta.x() > 0)
		delta.rx() = 0;
	if (delta.y() > 0)
		delta.ry() = 0;
	updateWidgets(delta);
	repaint();
}

void KisPreviewWidget::mouseReleaseEvent(QMouseEvent * e)
{
	mouseMoveEvent(e);
	emit updated();
	m_startDrag = e->pos() - m_startDrag;
	if (m_startDrag.x() > 0)
		m_startDrag.rx() = 0;
	if (m_startDrag.y() > 0)
		m_startDrag.ry() = 0;
}

void KisPreviewWidget::updateWidgets(QPoint delta)
{

	if (!m_layer) return;

	KisPainter gc;
	KisPaintDeviceSP pd(m_layer.data());

	//delta *= m_zoom;
	/* left */
	gc.begin(layerNew1.data());
	gc.bitBlt(0, 0, COMPOSITE_OVER, pd, -delta.x(), -delta.y(), -1, -1);
	gc.end();
	m_original->invalidate();

	/* right */
	gc.begin(layerNew2.data());
	gc.bitBlt(0, 0, COMPOSITE_OVER, pd, -delta.x(), -delta.y(), -1, -1);
	gc.end();
	m_preview->invalidate();
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

	KisStrategyColorSpaceSP colorstate = image->colorStrategy();
		
	if (zoomX != 1.0 || zoomY != 1.0)
		painter.scale(zoomX, zoomY);

	for (Q_INT32 y = y1; y <= y2; y += TILE_HEIGHT - (y % TILE_HEIGHT)) {
		for (Q_INT32 x = x1; x <= x2; x += TILE_WIDTH - (x % TILE_WIDTH)) {
			if ((tileno = image -> tileNum(x, y)) < 0)
				continue;

			image -> validate(tileno);
		}
	}

	for (Q_INT32 y = y1; y < y2; y += RENDER_HEIGHT)
		for (Q_INT32 x = x1; x < x2; x += RENDER_WIDTH)
			colorstate -> render(	image,
						painter,
						x, y,
						QMIN(x2 - x, RENDER_WIDTH),
						QMIN(y2 - y, RENDER_HEIGHT));
}

#include "kis_previewwidget.moc"
