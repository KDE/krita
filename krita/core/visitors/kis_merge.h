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
#if !defined KIS_MERGE_H_
#define KIS_MERGE_H_

#include <qrect.h>
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_paint_device_visitor.h"
#include "kis_painter.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_floatingselection.h"
#include "kis_selection.h"

struct All {
	const bool operator()(const KisPaintDeviceSP) const
	{
		return true;
	}
};

struct isVisible {
	const bool operator()(const KisPaintDeviceSP dev) const
	{
		return dev -> visible();
	}
};

struct isLinked {
	const bool operator()(const KisPaintDeviceSP dev) const
	{
		const KisLayer *layer = dynamic_cast<const KisLayer*>(dev.data());

		return layer && layer -> linked();
	}
};

template <typename merge_cond_t, typename remove_cond_t>
class KisMerge : public KisPaintDeviceVisitor {
public:
	KisMerge(KisImageSP img)
	{
		m_img = img;
		m_insertMergedAboveLayer = 0;
		m_haveFoundInsertionPlace = false;
	}

public:
	virtual bool visit(KisPainter&, KisPaintDeviceSP)
	{
		return false;
	}

	virtual bool visit(KisPainter&, vKisPaintDeviceSP&)
	{
		return false;
	}

	virtual bool visit(KisPainter& gc, vKisLayerSP& layers)
	{
		for (Q_INT32 i = layers.size() - 1; i >= 0; i--) {
			KisLayerSP& layer = layers[i];

			visit(gc, layer);
		}

		return true;
	}

	virtual bool visit(KisPainter& gc, KisLayerSP layer)
	{
		if (m_img -> index(layer) < 0)
			return false;

		if (m_mergeTest(layer.data())) {
			Q_INT32 sx;
			Q_INT32 sy;
			Q_INT32 dx;
			Q_INT32 dy;
			Q_INT32 w;
			Q_INT32 h;

			sx = layer -> x();
			sy = layer -> y();
			dx = layer -> x();
			dy = layer -> y();
			w = layer -> width();
			h = layer -> height();

			if (sx < 0) {
				w += sx;
				sx *= -1;
				dx = 0;
			} else {
				sx = 0;
			}

			if (sy < 0) {
				h += sy;
				sy *= -1;
				dy = 0;
			} else {
				sy = 0;
			}

			gc.bitBlt(dx, dy, layer -> compositeOp() , layer.data(), layer -> opacity(), sx, sy, w, h);

			if (!m_haveFoundInsertionPlace) {

				if (m_img -> index(layer) != m_img -> nlayers() - 1) {
					m_insertMergedAboveLayer = m_img -> layer(m_img -> index(layer) + 1);
				}
				else {
					m_insertMergedAboveLayer = 0;
				}

				m_haveFoundInsertionPlace = true;
			}
		}

		if (m_removeTest(layer.data())) {
			m_img -> rm(layer);
		}

		return true;
	}



	virtual bool visit(KisPainter&, KisSelectionSP)
	{
		return false;
	}

	// The laye

	virtual bool visit(KisPainter&, KisFloatingSelectionSP)
	{
		return false;
	}

	// The layer the merged layer should be inserted above, or 0 if
	// the merged layer should go to the bottom of the stack.
	KisLayerSP insertMergedAboveLayer() const { return m_insertMergedAboveLayer; }

private:
	KisImageSP m_img;
	merge_cond_t m_mergeTest;
	remove_cond_t m_removeTest;
	QRect m_rc;
	KisLayerSP m_insertMergedAboveLayer;
	bool m_haveFoundInsertionPlace;
};

#endif // KIS_MERGE_H_

