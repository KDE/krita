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
#include "kistile.h"
#include "kistilemgr.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_channel.h"
#include "kis_mask.h"
#include "kis_selection.h"

KisLayer::KisLayer(Q_INT32 width, Q_INT32 height, KisStrategyColorSpaceSP colorStrategy, const QString& name) : 
	super(width, height, colorStrategy, name),
	m_opacity(OPACITY_OPAQUE),
	m_linked(false),
	m_hasSelection(false),
	m_selection(0)
{
}

KisLayer::KisLayer(KisImageSP img, Q_INT32 width, Q_INT32 height, const QString& name, QUANTUM opacity)
	: super(img, width, height, img -> colorStrategy(), name),
	  m_opacity(opacity),
	  m_linked(false),
	  m_hasSelection(false),
	  m_selection(0)

{
}

KisLayer::KisLayer(const KisLayer& rhs) : super(rhs)
{
	if (this != &rhs) {
		m_opacity = rhs.m_opacity;
		m_preserveTransparency = rhs.m_preserveTransparency;
		m_initial = rhs.m_initial;
		m_linked = rhs.m_linked;
		m_dx = rhs.m_dx;
		m_dy = rhs.m_dy;

		if (rhs.m_mask)
			m_mask = new KisMask(*rhs.m_mask);

		m_hasSelection = false;
		m_selection = 0;
	}
}

KisLayer::KisLayer(KisTileMgrSP tm, KisImageSP img, const QString& name, QUANTUM opacity) : 
	super(tm, img, name),
	m_opacity(opacity),
	m_linked(false),
	m_hasSelection(false)
{
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


KisMaskSP KisLayer::mask() const
{
	return 0;
}

void KisLayer::setActiveSelection(KisSelectionSP selection)
{
	m_selection = selection;
	m_hasSelection = true;
}

void KisLayer::addSelection(KisSelectionSP /*selection*/)
{
// 	m_selection = m_selection - selection;
}

void KisLayer::subtractSelection(KisSelectionSP /*selection*/)
{
// 	m_selection = m_selection + selection;
}


bool KisLayer::hasSelection() const
{
	return m_hasSelection;
}


void KisLayer::removeSelection()
{
	m_selection = 0;
	m_hasSelection = false;
}

QUANTUM KisLayer::selected(Q_INT32 x, Q_INT32 y) const
{
	if (m_hasSelection) {
		return m_selection -> selected(x, y);
	}
	else {
		return 0;
	}
}


QUANTUM KisLayer::setSelected(Q_INT32 x, Q_INT32 y, QUANTUM s) 
{
	if (!m_hasSelection) {
		m_selection = new KisSelection(this, "layer selection for: " + name());
		m_hasSelection = true;
	}
	return m_selection -> setSelected(x, y, s);
}


void KisLayer::translate(Q_INT32 x, Q_INT32 y)
{
	m_dx = x;
	m_dy = y;
}

	

void KisLayer::addAlpha()
{
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

