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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
#include "kis_selection.h"
#include "visitors/kis_flatten.h"

template <typename cond_t>
class KisMerge : public KisPaintDeviceVisitor {
public:
	KisMerge(KisImageSP img, bool keepOld)
	{
		m_keepOld = keepOld;
		m_img = img;
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

			if (!visit(gc, layer))
				return false;
		}

		return true;
	}

	virtual bool visit(KisPainter& gc, KisLayerSP layer)
	{
		if (m_img -> index(layer) < 0)
			return false;

		KisFlatten<cond_t> flatten(0, 0, layer -> width(), layer -> height());

		flatten(gc, layer);

		if (!m_keepOld && m_test(layer.data()))
			m_img -> rm(layer);

		return true;
	}

	virtual bool visit(KisPainter&, KisSelectionSP)
	{
		return false;
	}

private:
	KisImageSP m_img;
	cond_t m_test;
	QRect m_rc;
	bool m_keepOld;
};

#endif // KIS_MERGE_H_

