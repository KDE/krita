/*
 *  Copyright (c) 2004 Kivio Team
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <qpixmap.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qframe.h>
#include <qcursor.h>

#include <kaction.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kiconloader.h>

#include <koGlobal.h>
#include <kozoomhandler.h>


#include "kis_view.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_global.h"
#include "kis_birdeye_box.h"


#define TOGGLE_ACTION(X) ((KToggleAction*)child(X))

KisBirdEyeBox::KisBirdEyeBox(KisView* view, QWidget* parent, const char* name)
	: WdgBirdEye(parent, name), 
	  m_view(view), 
	  m_doc(view->doc())
{
	m_handlePress = false;
	m_buffer = new QPixmap();
	m_zoomHandler = new KoZoomHandler;

// 	connect( m_doc, SIGNAL( sig_updateView(KivioPage*)), SLOT(slotUpdateView(KivioPage*)) );
// 	connect( m_view, SIGNAL(zoomChanged(int)), SLOT(canvasZoomChanged()));
// 	connect( m_pCanvas, SIGNAL(visibleAreaChanged()), SLOT(updateVisibleArea()));

	zoomIn = new KAction( i18n("Zoom In"), "kivio_zoom_plus", 0, this, SLOT(zoomPlus()), this, "zoomIn" );
	zoomOut = new KAction( i18n("Zoom Out"), "kivio_zoom_minus", 0, this, SLOT(zoomMinus()), this, "zoomOut" );
	KToggleAction* act3 = new KToggleAction( i18n("Show Page Border"),BarIcon("view_pageborder", KivioFactory::global()), 0, this, "pageBorder" );
#if KDE_IS_VERSION(3,2,90)
	act3->setCheckedState(i18n("Hide Page Border"));
#endif
	KAction* act5 = new KAction( i18n("Autoresize"), "window_nofullscreen", 0, this, SLOT(doAutoResizeMin()), this, "autoResizeMin" );
	KAction* act6 = new KAction( i18n("Autoresize"), "window_fullscreen", 0, this, SLOT(doAutoResizeMax()), this, "autoResizeMax" );

	connect( act3, SIGNAL(toggled(bool)), SLOT(togglePageBorder(bool)));

	zoomIn->plug(bar);
	zoomOut->plug(bar);
	act3->plug(bar);
	act5->plug(bar);
	act6->plug(bar);

	togglePageBorder(true);

	canvasZoomChanged();
}

KisBirdEyeBox::~KisBirdEyeBox()
{
	delete m_buffer;
	delete m_zoomHandler;
}

void KisBirdEyeBox::zoomChanged(int z)
{
//  debug(QString("zoomChanged %1 %2").arg(z).arg((double)(z/100.0)));
	m_view->viewZoom(z);
}

void KisBirdEyeBox::zoomMinus()
{
	m_pCanvas->zoomOut(QPoint(m_pCanvas->width()/2, m_pCanvas->height()/2));
}

void KisBirdEyeBox::zoomPlus()
{
	m_pCanvas->zoomIn(QPoint(m_pCanvas->width()/2, m_pCanvas->height()/2));
}

void KisBirdEyeBox::canvasZoomChanged()
{
	int iz = m_view->zoomHandler()->zoom();
	slider->blockSignals(true);
	zoomBox->blockSignals(true);

	zoomBox->setValue(iz);
	slider->setMaxValue(QMAX(iz,500));
	slider->setValue(iz);

	zoomBox->blockSignals(false);
	slider->blockSignals(false);

	slotUpdateView(m_view->activePage());
}

void KisBirdEyeBox::slotUpdateView(KivioPage* page)
{
	if (!isVisible() || !page || m_view->activePage() != page)
		return;

	updateView();
}

bool KisBirdEyeBox::eventFilter(QObject* o, QEvent* ev)
{
	if (o == canvas && ev->type() == QEvent::Show) {
		updateView();
	}

	if (o == canvas && ev->type() == QEvent::Resize) {
		m_buffer->resize(canvas->size());
		slotUpdateView(m_view->activePage());
	}

	if (o == canvas && ev->type() == QEvent::Paint) {
		updateVisibleArea();
		return true;
	}

	if (o == canvas && ev->type() == QEvent::MouseMove) {
		QMouseEvent* me = (QMouseEvent*)ev;
		if (me->state() == LeftButton)
			handleMouseMoveAction(me->pos());
		else
			handleMouseMove(me->pos());

		lastPos = me->pos();
		return true;
	}

	if (o == canvas && ev->type() == QEvent::MouseButtonPress) {
		QMouseEvent* me = (QMouseEvent*)ev;
		if (me->button() == LeftButton)
			handleMousePress(me->pos());

		return true;
	}

	return WdgBirdEye::eventFilter(o, ev);
}

void KisBirdEyeBox::updateView()
{
	// FIXME: This whole function needs fixing!
	QSize s1 = canvas->size();
	QSize s2;
	KoPageLayout pl = m_view->activePage()->paperLayout();

	int pw = m_view->zoomHandler()->zoomItX(pl.ptWidth);
	int ph = m_view->zoomHandler()->zoomItY(pl.ptHeight);
	s2 = QSize(pw,ph);

	double zx = (double)s1.width()/(double)s2.width();
	double zy = (double)s1.height()/(double)s2.height();
	double zxy = QMIN(zx,zy);

	m_zoomHandler->setZoomAndResolution(qRound(zxy * 100), KoGlobal::dpiX(),
					    KoGlobal::dpiY());

	pw = m_zoomHandler->zoomItX(pl.ptWidth);
	ph = m_zoomHandler->zoomItY(pl.ptHeight);
	int px0 = (s1.width()-pw)/2;
	int py0 = (s1.height()-ph)/2;

	int pcw = m_zoomHandler->zoomItX(s2.width());
	int pch = m_zoomHandler->zoomItY(s2.height());
	int pcx0 = (s1.width()-pcw)/2;
	int pcy0 = (s1.height()-pch)/2;

	cMinSize = QSize((int)(s2.width()*QMIN(zx,zy)), (int)(s2.height()*QMIN(zx,zy)));
	cMaxSize = QSize((int)(s2.width()*QMAX(zx,zy)), (int)(s2.height()*QMAX(zx,zy)));

	QPoint p0 = QPoint(px0,py0);

	QRect rect(QPoint(0,0),s1);

	KivioScreenPainter kpainter;
	kpainter.start(m_buffer);
	kpainter.painter()->fillRect(rect, QColor(120, 120, 120));

	if (m_bShowPageBorders) {
		kpainter.painter()->fillRect(pcx0, pcy0, pcw, pch, QColor(200, 200, 200));
		kpainter.painter()->fillRect(px0, py0, pw, ph, white);
	} else {
		kpainter.painter()->fillRect(pcx0, pcy0, pcw, pch, white);
	}

	kpainter.painter()->translate(px0, py0);
	m_doc->paintContent(kpainter, rect, false, m_view->activePage(), p0, m_zoomHandler, false);
	kpainter.stop();

	updateVisibleArea();
}

void KisBirdEyeBox::togglePageBorder(bool b)
{
	TOGGLE_ACTION("pageBorder")->setChecked(b);
	m_bShowPageBorders = b;

	slotUpdateView(m_view->activePage());
}

void KisBirdEyeBox::doAutoResizeMin()
{
	parentWidget()->resize(parentWidget()->width() - canvas->width() + cMinSize.width(), parentWidget()->height() - canvas->height() + cMinSize.height());
}

void KisBirdEyeBox::doAutoResizeMax()
{
	parentWidget()->resize(parentWidget()->width() - canvas->width() + cMaxSize.width(), parentWidget()->height() - canvas->height() + cMaxSize.height());
}

void KisBirdEyeBox::show()
{
	WdgBirdEye::show();
//   doAutoResizeMax();
}

void KisBirdEyeBox::updateVisibleArea()
{
	bitBlt(canvas,0,0,m_buffer);

	KoRect vr = m_pCanvas->visibleArea();
	QSize s1 = canvas->size();
	KoPageLayout pl = m_view->activePage()->paperLayout();
	int pw = m_zoomHandler->zoomItX(pl.ptWidth);
	int ph = m_zoomHandler->zoomItY(pl.ptHeight);
	int px0 = (s1.width()-pw)/2;
	int py0 = (s1.height()-ph)/2;

	int x = m_zoomHandler->zoomItX(vr.x()) + px0;
	int y = m_zoomHandler->zoomItY(vr.y()) + py0;
	int w = m_zoomHandler->zoomItX(vr.width());
	int h = m_zoomHandler->zoomItX(vr.height());

	QPainter painter(canvas,canvas);
	painter.setPen(red);
	painter.drawRect(x, y, w, h);
	painter.setPen(red.light());
	painter.drawRect(x-1, y-1, w+2, h+2);
	painter.end();

	varea.setRect(x,y,w,h);
}

void KisBirdEyeBox::handleMouseMove(QPoint p)
{
	m_handlePress = true;

	QRect r1 = QRect(varea.x()-1, varea.y()-1, 3, varea.height()+2);
	if (r1.contains(p)) {
		canvas->setCursor(sizeHorCursor);
		apos = AlignLeft;
		return;
	}

	r1.moveBy(varea.width(),0);
	if (r1.contains(p)) {
		canvas->setCursor(sizeHorCursor);
		apos = AlignRight;
		return;
	}

	QRect r2 = QRect(varea.x()-1, varea.y()-1, varea.width()+2, 3);
	if (r2.contains(p)) {
		canvas->setCursor(sizeVerCursor);
		apos = AlignTop;
		return;
	}

	r2.moveBy(0, varea.height());
	if (r2.contains(p)) {
		canvas->setCursor(sizeVerCursor);
		apos = AlignBottom;
		return;
	}

	if (varea.contains(p)) {
		canvas->setCursor(sizeAllCursor);
		apos = AlignCenter;
		return;
	}

	canvas->setCursor(arrowCursor);
	m_handlePress = false;
}

void KisBirdEyeBox::handleMouseMoveAction(QPoint p)
{
	if (!m_handlePress)
		return;

	p -= lastPos;

	if (apos == AlignCenter) {
		double zy = m_view->zoomHandler()->zoomedResolutionY() / m_zoomHandler->zoomedResolutionY();
		double zx = m_view->zoomHandler()->zoomedResolutionX() / m_zoomHandler->zoomedResolutionX();
		m_pCanvas->setUpdatesEnabled(false);
		m_pCanvas->scrollDx(-(int)(p.x()*zx));
		m_pCanvas->scrollDy(-(int)(p.y()*zy));
		m_pCanvas->setUpdatesEnabled(true);
		return;
	}

	double dx = m_zoomHandler->unzoomItX(p.x());
	double dy = m_zoomHandler->unzoomItY(p.y());

	KoRect vr = m_pCanvas->visibleArea();
	if (apos == AlignRight) {
		vr.setWidth(QMAX(10.0, vr.width() + dx));
		m_pCanvas->setVisibleAreaByWidth(vr);
	}
	else if (apos == AlignLeft) {
		vr.setX(vr.x() + dx);
		vr.setWidth(QMAX(10.0, vr.width() - dx));
		m_pCanvas->setVisibleAreaByWidth(vr);
	}
	else if (apos == AlignTop) {
		vr.setY(vr.y() + dy);
		vr.setHeight(QMAX(10.0 ,vr.height() - dy));
		m_pCanvas->setVisibleAreaByHeight(vr);
	}
	else if (apos == AlignBottom) {
		vr.setHeight(QMAX(10.0 ,vr.height() + dy));
		m_pCanvas->setVisibleAreaByHeight(vr);
	}
}

void KisBirdEyeBox::handleMousePress(QPoint p)
{
	if (m_handlePress)
		return;

	QSize s1 = canvas->size();
	KoPageLayout pl = m_view->activePage()->paperLayout();
	int pw = m_zoomHandler->zoomItX(pl.ptWidth);
	int ph = m_zoomHandler->zoomItY(pl.ptHeight);
	int px0 = (s1.width()-pw)/2;
	int py0 = (s1.height()-ph)/2;

	double x = m_zoomHandler->unzoomItX(p.x() - px0);
	double y = m_zoomHandler->unzoomItY(p.y() - py0);

	m_pCanvas->setViewCenterPoint(KoPoint(x,y));
}
#include "kis_birdeye_box.moc"
