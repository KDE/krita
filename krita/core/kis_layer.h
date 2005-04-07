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
#ifndef KIS_LAYER_H_
#define KIS_LAYER_H_

#include "kis_paint_device.h"
#include "kis_types.h"
#include <koffice_export.h>

class KRITACORE_EXPORT KisLayer : public KisPaintDevice {
	typedef KisPaintDevice super;

	Q_OBJECT

public:
	KisLayer(KisStrategyColorSpaceSP colorStrategy, const QString& name);
	KisLayer(KisImage *img, const QString& name, QUANTUM opacity);
	KisLayer(const KisLayer& rhs);
	virtual ~KisLayer();

public:
	virtual const bool visible() const;
	virtual void setVisible(bool v);

public:

	void translate(Q_INT32 x, Q_INT32 y);

	QUANTUM opacity() const;
	void setOpacity(QUANTUM val);

	bool linked() const;
	void setLinked(bool l);

	bool locked() const;
	void setLocked(bool l);

private:
	QUANTUM m_opacity;
	bool m_preserveTransparency;

	bool m_initial;
	bool m_linked;
	bool m_locked;

	Q_INT32 m_dx;
	Q_INT32 m_dy;

};

#endif // KIS_LAYER_H_

