/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_SELECTION_H_
#define KIS_SELECTION_H_

#include <qrect.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device_impl.h"
#include <koffice_export.h>

class QColor;


enum enumSelectionMode {
    SELECTION_ADD,
    SELECTION_SUBTRACT
};

/**
 * KisSelection contains a byte-map representation of a layer, where
 * the value of a byte signifies whether a corresponding pixel is selected, or not.
 *
 */
class KRITACORE_EXPORT KisSelection : public KisPaintDeviceImpl {

    typedef KisPaintDeviceImpl super;

public:
    KisSelection(KisPaintDeviceImplSP layer, const QString& name);

    virtual ~KisSelection();

    // Returns selectedness, or 0 if invalid coordinates
    QUANTUM selected(Q_INT32 x, Q_INT32 y);

    void setSelected(Q_INT32 x, Q_INT32 y, QUANTUM s);

    QImage maskImage();

    void select(QRect r);

    void invert();

    void clear(QRect r);

    void clear();

    // Tests if the the rect is totally outside the selection
    bool isTotallyUnselected(QRect r);

    /**
     * Rough, but fastish way of determining the area
     * of the tiles used by the selection.
     */
    QRect selectedRect();

    /**
     * Slow, but exact way of determining the rectangle
     * that encloses the selection
     */
    QRect selectedExactRect();
    
    void paintSelection(QImage img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
    
private:
    KisPaintDeviceImplSP m_parentLayer;
};

#endif // KIS_SELECTION_H_
