/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KIS_FLATTEN_H_
#define KIS_FLATTEN_H_

#include <qrect.h>

#include <kdebug.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_paint_device_visitor.h"
#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_selection.h"

struct flattenAll {
	const bool operator()(const KisPaintDeviceSP) const
	{
		return true;
	}
};

struct flattenAllVisible {
	const bool operator()(const KisPaintDeviceSP dev) const
	{
		return dev -> visible();
	}
};

struct flattenAllLinked {
	const bool operator()(const KisPaintDeviceSP dev) const
	{
		const KisLayer *layer = dynamic_cast<const KisLayer*>(dev.data());

		return layer && layer -> linked();
	}
};

template <typename cond_t>
class KisFlatten : public KisPaintDeviceVisitor {
public:
	KisFlatten(Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height)
	{
		m_rc.setRect(x, y, width, height);
	}

	KisFlatten(QRect& rc)
	{
		m_rc = rc;
	}

	virtual ~KisFlatten()
	{
	}

public:
	virtual bool visit(KisPainter& gc, KisPaintDeviceSP dev)
	{
		visit(gc, dev, OPACITY_OPAQUE);
		return true;
	}

	virtual bool visit(KisPainter& gc, vKisPaintDeviceSP& devs)
	{
		for (Q_INT32 i = devs.size() - 1; i >= 0; i--)
			visit(gc, devs[i], OPACITY_OPAQUE);

		return true;
	}

	virtual bool visit(KisPainter& gc, vKisLayerSP& layers)
	{
		for (Q_INT32 i = layers.size() - 1; i >= 0; i--) {
			KisLayerSP& layer = layers[i];

			visit(gc, layer.data(), layer -> opacity());
		}

		return true;
	}

	virtual bool visit(KisPainter& gc, KisLayerSP layer)
	{
		visit(gc, layer.data(), layer -> opacity());
		return true; 
	}

	virtual bool visit(KisPainter& gc, KisSelectionSP selection)
	{
		int OPACITY = 200; // XXX: Hardcoded opacity of the
				   // selection mas. make an option.

		visit(gc, selection.data(), OPACITY);
		return true; 
	}

private:
	void visit(KisPainter& gc, KisPaintDeviceSP dev, Q_INT32 opacity)
	{
		Q_INT32 w = m_rc.width();
		Q_INT32 h = m_rc.height();
		Q_INT32 dx = m_rc.x();
		Q_INT32 dy = m_rc.y();
		Q_INT32 sx;
		Q_INT32 sy;

		if (!m_test(dev))
			return;
		sx = m_rc.x() - dev-> getX();
		sy = m_rc.y() - dev-> getY();
// 		kdDebug() << "Visiting on: " << dev
// 			  << " dx: " << dx
// 			  << " dy: " << dy
// 			  << " sx: " << sx
// 			  << " sy: " << sy
// 			  << " w: " << w
// 			  << " h " << h
// 			  << "\n";

		gc.bitBlt(dx, dy, dev -> compositeOp(), dev, opacity, sx, sy, w, h);
	}

private:
	cond_t m_test;
	QRect m_rc;
};

#endif // KIS_FLATTEN_H_

