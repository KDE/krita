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
#ifndef KIS_CHANNEL_H_
#define KIS_CHANNEL_H_

#include "kis_paint_device.h"

class KisChannel : public KisPaintDevice {
	typedef KisPaintDevice super;

public:
	KisChannel(KisImage *img, Q_INT32 width, Q_INT32 height, const QString& name, const KoColor& color);
	KisChannel(const KisChannel& rhs);
	virtual ~KisChannel();

public:
	QUANTUM opacity() const;
	void opacity(QUANTUM val);

	KoColor color() const;
	void color(KoColor clr);

	KisChannelSP createMask(Q_INT32 w, Q_INT32 h);
	bool bounds(QRect& rc);

	Q_INT32 value(Q_INT32 x, Q_INT32 y);
	bool empty();

	void feather();
	void clear();
	void invert();
	void border(Q_INT32 xradius, Q_INT32 yradius);
	void grow(Q_INT32 xradius, Q_INT32 yradius);
	void shrink(Q_INT32 xradius, Q_INT32 yradius);
	void translate(Q_INT32 x, Q_INT32 y);
};

#endif // KIS_CHANNEL_H_

