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
#include "kis_types.h"

class KisLayer : public KisPaintDevice {
	typedef KisPaintDevice super;

public:
	KisLayer(Q_INT32 width, Q_INT32 height, KisStrategyColorSpaceSP colorStrategy, const QString& name);
	KisLayer(KisImageSP img, Q_INT32 width, Q_INT32 height, const QString& name, QUANTUM opacity);
	KisLayer(KisTileMgrSP tiles, KisImageSP img, const QString& name, QUANTUM opacity);
	KisLayer(const KisLayer& rhs);
	virtual ~KisLayer();

public:
	virtual const bool visible() const;
	virtual void visible(bool v);

public:
	// XXX: Masks were already out of order before I started on
	// Krita, and I don't know what they were for.
	KisMaskSP createMask(Q_INT32 maskType);
	KisMaskSP addMask(KisMaskSP mask);
	void applyMask(Q_INT32 mode);
	KisMaskSP mask() const;

	// Selection stuff. XXX: is it necessary to make the actual
	// selection object available outside the layer?

	/** Set the specified selection object as the active selection for this layer */
	void setActiveSelection(KisSelectionSP selection);

	/** Adds the specified selection to the currently active selection for this layer */
	void addSelection(KisSelectionSP selection);

	/** Subtracts the specified selection from the currently active selection for this layer */
	void subtractSelection(KisSelectionSP selection);

	/** Whether there is a valid selection for this layer. */
	bool hasSelection() const;

	/** Removes the current selection for this layer. */
	void removeSelection();

	/**
	 * Returns the selection state for the pixel designated by X
	 * and Y: a pixel can be fully selected (QUANTUM_MASK) or not
	 * selected at all (0).
	 */
	QUANTUM selected(Q_INT32 x, Q_INT32 y) const;

	/**
	 * Sets pixel x,y to selection state s; returns the previous
	 * state. If the layer has no valid selection, creates a new
	 * valid selection.
	 */
	QUANTUM setSelected(Q_INT32 x, Q_INT32 y, QUANTUM s);
	
	void translate(Q_INT32 x, Q_INT32 y);
	void addAlpha();


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

	// Whether there is a selection valid for this layer
	bool m_hasSelection;

	// Contains the actual selection. For now, there can be only
	// one selection per layer. XXX: is this a limitation? 
	KisSelectionSP m_selection;

};

#endif // KIS_LAYER_H_

