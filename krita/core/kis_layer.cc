/*
 *  copyright (c) 2002 patrick julien <freak@codepimps.org>
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
}

KisLayer::KisLayer(KisTileMgr tiles, KisImageSP img, const QString& name, QUANTUM opacity)
	: super(img, img -> width(), img -> height(), img -> imgType(), name)
{
}

KisLayer::~KisLayer()
{
}

void KisLayer::copy(const KisPaintDevice& rhs, bool addAlpha)
{
}

bool KisLayer::checkScaling(Q_INT32 width, Q_INT32 height)
{
	return false;
}

KisMaskSP KisLayer::createMask(Q_INT32 maskType)
{
	return 0;
}

KisMaskSP KisLayer::addMask(KisMaskSP mask)
{
	return 0;
}

void KisLayer::applyMask(Q_INT32 mode)
{
}

void KisLayer::translate(Q_INT32 x, Q_INT32 y)
{
}

void KisLayer::addAlpha()
{
}

void KisLayer::scaleFactor(double wfactor, double hfactor)
{
}

void KisLayer::scale(Q_INT32 width, Q_INT32 height, Q_INT32 interpolation, bool localOrigin)
{
}

void KisLayer::scale(const QSize& size, Q_INT32 interpolation, bool localOrigin)
{
}

void KisLayer::resize(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
}

void KisLayer::resize(const QRect& rc)
{
}

void KisLayer::resize()
{
}

void KisLayer::boundary(const vKisSegments& segments)
{
}

void KisLayer::invalidateBounds()
{
}

KisMaskSP KisLayer::mask() const
{
	return 0;
}

bool KisLayer::isFloatingSel() const
{
	return false;
}

QUANTUM KisLayer::opacity() const
{
	return 0;
}

void KisLayer::opacity(QUANTUM val)
{
}

bool KisLayer::linked() const
{
	return m_linked;
}

void KisLayer::linked(bool l)
{
	m_linked = l;
}

