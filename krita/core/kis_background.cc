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
#include "kis_global.h"
#include "kis_background.h"
#include "kis_image.h"
#include "kis_iterators_pixel.h"

KisBackground::KisBackground(KisImage *img, Q_INT32 /*width*/, Q_INT32 /*height*/) :
	super(img, "background flyweight", OPACITY_OPAQUE)
{
	Q_INT32 y;
        Q_UINT8 src[depth()]; // XXX: Change KoColor to KisColor
	Q_UINT32 d = depth();

	Q_ASSERT( colorStrategy() != 0 );
	
	for (y = 0; y < 64; y++)
	{
		KisHLineIteratorPixel hiter = createHLineIterator(0, y, 64, false);
		while( ! hiter.isDone())
		{
			QUANTUM v = 128 + 63 * ((hiter.x() / 16 + y / 16) % 2);
			QColor c(v,v,v);
			colorStrategy() -> nativeColor(c, OPACITY_OPAQUE, src);
			
			memcpy((Q_UINT8 *)hiter, src, d);
			
			hiter++;
		}
	}
}

KisBackground::~KisBackground()
{
}
