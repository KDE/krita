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
#include "kis_types.h"
#include "kis_channel.h"
#include "kis_image.h"
#include "kis_colorspace_registry.h"

KisChannel::KisChannel(KisImageSP img, Q_INT32 width, Q_INT32 height, const QString& name, const KoColor&) 
	: super(img, width, height, KisColorSpaceRegistry::singleton()->colorSpace("Grayscale + Alpha"), name)
{
}

KisChannel::KisChannel(const KisChannel& rhs) : super(rhs)
{
}

KisChannel::~KisChannel()
{
}

QUANTUM KisChannel::opacity() const
{
	return QUANTUM_MAX;
}

void KisChannel::opacity(QUANTUM)
{
}

