/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_SELECTION_H_
#define KIS_SELECTION_H_

#include <qrect.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_paint_device.h"

class KoColor;

// Note: this is intentionally not namespaced, because it's meant for all of Krita 
// that wants to determine selectedness.
#if (QUANTUM_DEPTH == 8)
// XXX: swap when special color strategy for selections is done?
const QUANTUM MAX_SELECTED = OPACITY_TRANSPARENT;
const QUANTUM MIN_SELECTED = OPACITY_OPAQUE;
#endif


/**
 * KisSelection contains a byte-map representation of a layer, where
 * the value of a byte signifies whether a corresponding pixel is selected, or not,
 * and an array or QPointArray's that represent the boundaries of the areas of a
 * possibly discontinuous selection. The points in the point array 'walk around'
 * the selected area clock-wise.
 *
 * Other types of selection could store a rect, a circle, a path --
 * whatever. Optimisation, not implemented yet.
 *
 */
class KisSelection : public KisPaintDevice {

	typedef KisPaintDevice super;

public:
	KisSelection(KisLayerSP layer, const QString& name);
	KisSelection(KisLayerSP layer, const QString& name, KoColor c);

	virtual ~KisSelection();

	// Returns selectedness, or 0 if invalid coordinates
	QUANTUM selected(Q_INT32 x, Q_INT32 y);

	void setSelected(Q_INT32 x, Q_INT32 y, QUANTUM s);

	QImage maskImage() const;

	void selectAll();

	void clear();

	// Clear the selection and set the mask to color c
	// Note: it is intentional to deep-copy the color
	// since the selection will want to own its own copy.
	void clear(const KoColor c);

	// Keep the selection but set the mask to color c
	// Note: it is intentional to deep-copy the color
	// since the selection will want to own its own copy.
	void setMaskColor(const KoColor c);

	/**
	 * Set the area that encloses all selected pixels. This
	 * will over the lifetime of the selection grow, never
	 * shrink.
	 */
	void setSelectedRect(QRect r) { m_selectedRect |= r; }
	QRect selectedRect();

private:
	KisLayerSP m_parentLayer;
	KisColorSpaceAlphaSP m_alpha;
	KoColor m_maskColor;
	QRect m_selectedRect;
};

#endif // KIS_SELECTION_H_
