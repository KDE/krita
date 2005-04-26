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

#include <kdebug.h>

#include "kis_global.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_painter.h"

#define DEBUG_LAYERS 0

#if DEBUG_LAYERS
static int numLayers = 0;
#endif

KisLayer::KisLayer(KisStrategyColorSpaceSP colorStrategy, const QString& name)
	: super(colorStrategy, name),
	  m_opacity(OPACITY_OPAQUE),
	  m_linked(false)
{
#if DEBUG_LAYERS
	numLayers++;
	kdDebug() << "LAYER " << name << " CREATED total now = " << numLayers << endl;
#endif
}

KisLayer::KisLayer(KisImage *img, const QString& name, QUANTUM opacity)
	: super(img, img -> colorStrategy(), name),
	  m_opacity(opacity),
	  m_linked(false)
{
#if DEBUG_LAYERS
	numLayers++;
	kdDebug() << "LAYER " << name << " CREATED total now = " << numLayers << endl;
#endif
}

KisLayer::KisLayer(KisImage *img, const QString& name, QUANTUM opacity, KisStrategyColorSpaceSP colorStrategy)
	: super(img, colorStrategy, name),
	  m_opacity(opacity),
	  m_linked(false)
{
#if DEBUG_LAYERS
	numLayers++;
	kdDebug() << "LAYER " << name << " CREATED total now = " << numLayers << endl;
#endif
}

KisLayer::KisLayer(const KisLayer& rhs) : super(rhs)
{
#if DEBUG_LAYERS
	numLayers++;
	kdDebug() << "LAYER " << rhs.name() << " copy CREATED total now = " << numLayers << endl;
#endif
	if (this != &rhs) {
		m_opacity = rhs.m_opacity;
		m_preserveTransparency = rhs.m_preserveTransparency;
		m_initial = rhs.m_initial;
		m_linked = rhs.m_linked;
		m_dx = rhs.m_dx;
		m_dy = rhs.m_dy;

/*		if (rhs.m_mask)
			m_mask = new KisMask(*rhs.m_mask);*/
	}
}

KisLayer::~KisLayer()
{
#if DEBUG_LAYERS
	numLayers--;
	kdDebug() << "LAYER " << name() << " DESTROYED total now = " << numLayers << endl;
#endif
}



void KisLayer::translate(Q_INT32 x, Q_INT32 y)
{
	m_dx = x;

	m_dy = y;
}


QUANTUM KisLayer::opacity() const
{
	return m_opacity;
}

void KisLayer::setOpacity(QUANTUM val)
{
	m_opacity = val;
}

bool KisLayer::linked() const
{
	return m_linked;
}

void KisLayer::setLinked(bool l)
{
	m_linked = l;
}

const bool KisLayer::visible() const
{
	return super::visible() && m_opacity != OPACITY_TRANSPARENT;
}

void KisLayer::setVisible(bool v)
{
	super::setVisible(v);
}

bool KisLayer::locked() const
{
	return m_locked;
}

void KisLayer::setLocked(bool l)
{
	m_locked = l;
}


#include "kis_layer.moc"
