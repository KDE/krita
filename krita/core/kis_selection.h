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

#include <qimage.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_layer.h"


/**
 * KisSelection contains a byte-map representation of a layer, where
 * the value of a byte signifies whether a corresponding pixel is selected, or not,
 * and an array or QPointArray's that represent the boundaries of the areas of a
 * possibly discontinuous selection. The points in the point array 'walk around'
 * the selected area clock-wise.
 *
 * Other types of selection could store a rect, a circle, a path -- whatever. Optimisation,
 * not implemented yet.
 *
 */
class KisSelection : public KShared {

public:
	KisSelection(KisLayerSP layer, const QString& name);
	virtual ~KisSelection();

	// Returns selectedness, or 0 if invalid coordinates
	QUANTUM selected(Q_INT32 x, Q_INT32 y) const;

	// Sets selectedness, and returns previous selectedness or 0
	// if invalid coordinates.
	QUANTUM setSelected(Q_INT32 x, Q_INT32 y, QUANTUM s);

	QImage maskImage() const;

	void reset();

private:
	KisLayerSP m_layer;

	// An 8-bit QImage is a pretty efficient way of storing the
	// 8-bit selection mask, and has as a plus that it's really,
	// really fast to show a preview of the mask.
	QImage m_mask;
	QString m_name;
};

#endif // KIS_FLOATINGSELECTION_H_

