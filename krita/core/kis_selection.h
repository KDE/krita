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
#if !defined KIS_SELECTION_H_
#define KIS_SELECTION_H_

#include "qvaluevector.h"

#include "kis_global.h"
#include "kis_types.h"
#include "kis_layer.h"

/**
 * KisSelection contains a byte-map representation of a layer, where
 * the value of a byte signifies whether a corresponding pixel is selected, or not,
 * and an array or QPointArray's that represent the boundaries of the areas of a
 * possibly discontinuous selection. The points in the point array 'walk around'
 * the selected area clock-wise.
 */
class KisSelection {

public:
	KisSelection(KisPaintDeviceSP layer, const QString& name);
	virtual ~KisSelection();

	void setSelected(Q_UINT8 s, Q_UINT32 x, Q_UINT32 y);
	Q_UINT8 isSelected(Q_UINT32 x, Q_UINT32 y);

private:
	KisPaintDeviceSP m_layer;
	// XXX: check whether the STL vector is faster/better match.
	// a big chunk of memory is _not_ an alternative.
	typedef QValueVector<Q_UINT8> MaskVector;
	MaskVector * m_mask;
};

#endif // KIS_FLOATINGSELECTION_H_

