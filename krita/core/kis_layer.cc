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
#include "kistile.h"
#include "kistilemgr.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_channel.h"
#include "kis_mask.h"

KisLayer::KisLayer(KisImageSP img, Q_INT32 width, Q_INT32 height, const QString& name, QUANTUM opacity)
	: super(img, width, height, img -> imgType(), name)
{
	m_linked = false;
	m_opacity = opacity;
}

KisLayer::KisLayer(KisTileMgr, KisImageSP img, const QString& name, QUANTUM opacity)
	: super(img, img -> width(), img -> height(), img -> imgType(), name)
{
	m_linked = false;
	m_opacity = opacity;
}

KisLayer::~KisLayer()
{
}

KisMaskSP KisLayer::createMask(Q_INT32 )
{
	return 0;
}

KisMaskSP KisLayer::addMask(KisMaskSP )
{
	return 0;
}

void KisLayer::applyMask(Q_INT32 )
{
}

void KisLayer::translate(Q_INT32 , Q_INT32 )
{
}

void KisLayer::addAlpha()
{
}

void KisLayer::resize(Q_INT32 , Q_INT32 , Q_INT32 , Q_INT32 )
{
}

void KisLayer::resize(const QRect& )
{
}

void KisLayer::resize()
{
}

void KisLayer::boundary(const vKisSegments& )
{
}

KisMaskSP KisLayer::mask() const
{
	return 0;
}

QUANTUM KisLayer::opacity() const
{
	return m_opacity;
}

void KisLayer::opacity(QUANTUM val)
{
	m_opacity = val;
}

bool KisLayer::linked() const
{
	return m_linked;
}

void KisLayer::linked(bool l)
{
	m_linked = l;
}

