/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#include <stdlib.h>
#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>
#include <koColor.h>
#include "kis_global.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_floatingselection.h"
#include "kis_undo_adapter.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kispixeldata.h"
#include "kis_iterators_quantum.h"
#include "kis_iterators_pixel.h"

namespace {
	class KisResetFirstMoveCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		KisResetFirstMoveCmd(KisFloatingSelectionSP selection) : super("reset selection first move flag")
		{
			m_selection = selection;
		}

		virtual ~KisResetFirstMoveCmd()
		{
		}

	public:
		virtual void execute()
		{
			m_selection -> clearParentOnMove(false);
		}

		virtual void unexecute()
		{
			m_selection -> clearParentOnMove(true);
		}

	private:
		KisFloatingSelectionSP m_selection;
	};
}


KisFloatingSelection::KisFloatingSelection(Q_INT32 width, Q_INT32 height, KisStrategyColorSpaceSP colorStrategy, const QString& name)
	: super(width, height, colorStrategy, name)
{
	m_clearOnMove = true;
	m_endMacroOnAnchor = false;
	setVisible(false);
	setName(name);
}

KisFloatingSelection::KisFloatingSelection(KisPaintDeviceSP parent, KisImage *img, const QString& name, QUANTUM opacity) 
	: super(img, 0, 0, name, opacity)
{
	Q_ASSERT(parent);
	Q_ASSERT(parent -> visible());
	Q_ASSERT(img);
	m_parent = parent;
	m_selectionMask = 0;
	m_img = img;
	m_name = name;
	m_firstMove = true;
	m_clearOnMove = true;
	m_endMacroOnAnchor = false;
	connect(m_parent, SIGNAL(visibilityChanged(KisPaintDeviceSP)), SLOT(parentVisibilityChanged(KisPaintDeviceSP)));
}

KisFloatingSelection::~KisFloatingSelection()
{
}

void KisFloatingSelection::commit()
{
	if (m_parent) {
		KisUndoAdapter *adapter = image() -> undoAdapter();
		KisImageSP img;
		KisPainter gc;
		QRect rc = clip();
		QPoint pt(0, 0);
		Q_INT32 w = width();
		Q_INT32 h = height();

		if (!rc.isEmpty()) {
			w = rc.width();
			h = rc.height();
		}

		if (x() < 0) {
			pt.setX(-x());
			setX(0);
		}

		if (y() < 0) {
			pt.setY(-y());
			setY(0);
		}

		img = m_parent -> image();

		if (x() < m_parent -> x() || y() < m_parent -> y())
			m_parent -> offsetBy(abs(m_parent -> x() - x()), abs(m_parent -> y() - y()));

		if (img -> width() < x() + w)
			w = img -> width() - x();

		if (img -> height() < y() + h)
			h = img -> height() - y();

		if (x() + w > m_parent -> x() + m_parent -> width() || y() + h > m_parent -> y() + m_parent -> height())
			m_parent -> expand(x() + w - m_parent -> x(), y() + h - m_parent -> y());

		gc.begin(m_parent);
		gc.beginTransaction("copy selection to parent");
		Q_ASSERT(w <= width());
		Q_ASSERT(h <= height());
		gc.bitBlt(x() - m_parent -> x(), y() - m_parent -> y(), COMPOSITE_COPY, this, opacity(), pt.x(), pt.y(), w, h);
		adapter -> addCommand(gc.endTransaction());
		gc.end();
	}
}

void KisFloatingSelection::copySelection(KisSelectionSP selection) {
	m_selectionMask = selection;
	if (m_img) {
		KisPainter gc;

		// TODO if the parent is linked... copy from all linked layers?!?
		QRect r = selection -> selectedRect();

		configure(m_img, r.width(), r.height(), m_img -> colorStrategy(), m_name, COMPOSITE_OVER);


		gc.begin(this);
		gc.bitBlt(0, 0, COMPOSITE_COPY, m_parent, r.x() - m_parent -> x(), r.y() - m_parent -> y(), r.width(), r.height());

		// XXX: switch to proper iterators
		KisTileCommand* ktc = new KisTileCommand("apply mask", (KisPaintDeviceSP) this ); // Create a command
		
		KoColor c;
		QUANTUM opacity;
		Q_INT32 x = 0;
		Q_INT32 y = 0;

		KisIteratorLineQuantum lineIt = selection -> iteratorQuantumSelectionBegin(ktc, r.x(), r.x() + r.width() - 1, r.y() );
		KisIteratorLineQuantum lastLine = selection -> iteratorQuantumSelectionEnd(ktc, r.x(), r.x() + r.width() - 1, r.y() + r.height() - 1);
		while( lineIt <= lastLine )
		{
			KisIteratorQuantum quantumIt = *lineIt;
			KisIteratorQuantum lastQuantum = lineIt.end();
			while( quantumIt <= lastQuantum )
			{
				// XXX: roundabout way of setting opacity
				pixel(x, y, &c, &opacity);
				if ((opacity - quantumIt) < OPACITY_TRANSPARENT) {
					setPixel(x, y, c, OPACITY_TRANSPARENT);
				}
				else {
					setPixel(x, y, c, opacity - quantumIt);
				}
				++quantumIt; // the alphamask has just one byte per pixel.
				++x;
			}
			++lineIt;
			x = 0;
			++y;
		}
		super::move(r.x(), r.y());
	}
	kdDebug() << "Selection copied: "
		  << r.x() << ", "
		  << r.y() << ", "
		  << r.width() << ", "
		  << r.height() << "\n";


}

bool KisFloatingSelection::shouldDrawBorder() const
{
	return true;
}

void KisFloatingSelection::move(Q_INT32 x, Q_INT32 y)
{
	QRect rc = clip();

	if (m_clearOnMove && m_firstMove && m_parent) {
		KisFillPainter painter(m_parent);
		KisUndoAdapter *adapter = image() -> undoAdapter();

		adapter -> beginMacro(i18n("Move Selection"));
		adapter -> addCommand(new KisResetFirstMoveCmd(this));
		painter.beginTransaction("clear the parent's background from KisFloatingSelection::move");
		painter.eraseRect(this -> x() - m_parent -> x(), this -> y() - m_parent -> y(), width(), height());
		m_firstMove = false;
		adapter -> addCommand(painter.endTransaction());
		m_endMacroOnAnchor = true;
	}

	super::move(x, y);
}

void KisFloatingSelection::parentVisibilityChanged(KisPaintDeviceSP parent)
{
	setVisible(parent -> visible());
}

void KisFloatingSelection::setParent(KisPaintDeviceSP parent)
{
	m_parent = parent;
}

KisPaintDeviceSP KisFloatingSelection::parent() const
{
	return m_parent;
}

void KisFloatingSelection::anchor()
{
	if (m_endMacroOnAnchor) {
		KisUndoAdapter *adapter = m_parent -> image() -> undoAdapter();

		adapter -> endMacro();
		m_endMacroOnAnchor = false;
	}

	super::anchor();
}

void KisFloatingSelection::clearParentOnMove(bool f)
{
	m_clearOnMove = f;
	m_firstMove = true;
}

QImage KisFloatingSelection::toImage()
{
	KisTileMgrSP tm = data();
	KisPixelDataSP raw;
	Q_INT32 stride;
	QUANTUM *src;

	if (tm) {
		if (tm -> width() == 0 || tm -> height() == 0)
			return QImage();

		raw = tm -> pixelData(0, 0, tm -> width() - 1, tm -> height() - 1, TILEMODE_READ);

		if (raw == 0)
			return QImage();

		if (m_clipImg.width() != tm -> width() || m_clipImg.height() != tm -> height())
			m_clipImg.create(tm -> width(), tm -> height(), 32);

		stride = tm -> depth();
		src = raw -> data;

		for (Q_INT32 y = 0; y < tm -> height(); y++) {
			for (Q_INT32 x = 0; x < tm -> width(); x++) {
				// XXX Different img formats
				// XXX Alpha channel
				m_clipImg.setPixel(x, y, qRgb(downscale(src[PIXEL_RED]), downscale(src[PIXEL_GREEN]), downscale(src[PIXEL_BLUE])));
				src += stride;
			}
		}
	}

	return m_clipImg;
}

#include "kis_floatingselection.moc"

