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
#if !defined KIS_LAYER_H_
#define KIS_LAYER_H_

#include "kis_paint_device.h"

class KisLayer : public KisPaintDevice {
	typedef KisPaintDevice super;

public:
	KisLayer(Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name);
	KisLayer(KisImageSP img, Q_INT32 width, Q_INT32 height, const QString& name, QUANTUM opacity);
	KisLayer(KisTileMgrSP tiles, KisImageSP img, const QString& name, QUANTUM opacity);
	KisLayer(const KisLayer& rhs);
	virtual ~KisLayer();

public:
	virtual const bool visible() const;
	virtual void visible(bool v);

public:
	KisMaskSP createMask(Q_INT32 maskType);
	KisMaskSP addMask(KisMaskSP mask);
	void applyMask(Q_INT32 mode);

	void translate(Q_INT32 x, Q_INT32 y);
	void addAlpha();

	KisMaskSP mask() const;

	QUANTUM opacity() const;
	void setOpacity(QUANTUM val);

	bool linked() const;
	void linked(bool l);

private:
	QUANTUM m_opacity;
	bool m_preserveTransparency;
	KisMaskSP m_mask;
	bool m_initial;
	bool m_linked;
	Q_INT32 m_dx;
	Q_INT32 m_dy;
};

#endif // KIS_LAYER_H_

