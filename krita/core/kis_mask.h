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
#if !defined KIS_MASK_H_
#define KIS_MASK_H_

#include "kis_channel.h"

class KisMask : public KisChannel {
	typedef KisChannel super;

public:
	KisMask(KisImage *img, Q_INT32 width, Q_INT32 height, const QString& name, const KoColor& color);
	KisMask(const KisMask& rhs);
	virtual ~KisMask();

public:
	KisLayerSP layer() const;
	void layer(KisLayerSP owner);

	Q_INT32 apply() const;
	void apply(Q_INT32 mask);

	bool edit() const;
	void edit(bool val);
};

#endif // KIS_MASK_H_

