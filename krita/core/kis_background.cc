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
#include "kistilemgr.h"
#include "kistile.h"

namespace {
	void fillBgTile(KisTileSP tile, Q_INT32 nbc )
	{
		QUANTUM *p;
		QUANTUM *q;

		if (!tile)
			return;

		tile -> lock();
		p = tile -> data();

		for (Q_INT32 y = 0; y < tile -> height(); y++) {
			for (Q_INT32 x = 0; x < tile -> width(); x++) {
				QUANTUM v = 128 + 63 * ((x / 16 + y / 16) % 2);

				v = upscale(v);
				q = p + (y * tile -> width() + x) * tile -> depth();
				for(Q_INT32 c = 0; c < nbc; c++)
				{
					q[c] = v;
				}
				q[nbc] = OPACITY_OPAQUE;
			}
		}

		tile -> release();
	}
}

KisBackground::KisBackground(KisImageSP img, Q_INT32 width, Q_INT32 height) :
	super(img, width, height, "background flyweight", OPACITY_OPAQUE)
{
	KisTileMgrSP tm;
	KisTileSP tile;

	tm = data();
	tile = tm -> tile(0, TILEMODE_WRITE);
	fillBgTile(tile,  ::imgTypeDepth( img->imgTypeWithAlpha() ));

	for (Q_UINT32 i = 0, k = 0; i < tm -> nrows(); i++)
		for (Q_UINT32 j = 0; j < tm -> ncols(); j++, k++)
			tm -> attach(tile, k);

}

KisBackground::~KisBackground()
{
}

Q_INT32 KisBackground::tileNum(Q_INT32, Q_INT32) const
{
	return 0;
}

