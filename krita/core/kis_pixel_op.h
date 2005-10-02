/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_PIXEL_OP_H_
#define KIS_PIXEL_OP_H_

#include <map>
#include <qvaluelist.h>

#include "kis_id.h"
#include "kis_colorspace.h"
#include "koffice_export.h"


/**
 * A KisPixelOp is an abstraction of the idea of messing with one or more pixels as
 * expressed by an array of Q_UINT8's. A given colorstrategy can provide a given pixelop
 * natively, or return a default implementation; a default implementation implements the
 * same algorithm, but may have to downscale or upscale the pixel data or convert it to
 * a colorspace that may be smaller.
 *
 * If a filter or a plugin or even a core part of Krita needs to mess with some pixels,
 * it will ask the colormodel of the current paint device for an implementation; it then
 * needs to downcast the implementation to the needed subclass. For instance:
 *
    // The cast is necessary because we need to be able to call the sepcialized process method which may have any
    // amount and type of parameters. No need for genericity here, because we _know_ what we want :-).
    KisApplyAdjustmentOp * adjustop = dynamic_cast<KisApplyAdjustmentOp*>(dev->colorSpace()->getOp("applyAdjustment");
    if (!adjustop) {
        
    }
    if (!adjustop->isNative()) {
        KMessageBox::questionYesNo(0, i18n("Brightness/contrast adjustment may cause information loss when used with this image. Do you want to continue?"));
    }
    KisBrightnessContrastFilterConfiguration* configBC = (KisBrightnessContrastFilterConfiguration*) config;

    KisColorAdjustment *adj = src->colorSpace()->createBrightnessContrastAdjustment(configBC->transfer);
        
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! srcIt.isDone()  && !cancelRequested())
    {
        Q_UINT32 npix;
        npix = srcIt.nConseqPixels();
        
        // change the brightness and contrast
        adjustOp->applyAdjustment(srcIt.oldRawData(), dstIt.rawData(), adj, npix);
                    
        srcIt+=npix;
        dstIt+=npix;

        pixelsProcessed++;
        setProgress(pixelsProcessed);
    }

 *
 * If a plugin wants to add a new pixelop, it can create one or more implementations for one or more
 * colormodels and register them with the colormodels. You can also implement a generic op that converts
 * pixel data using toQColor and fromQColor, for instance, or through XYZ, applies the operation and
 * then converts it back.
 */
 
class KRITACORE_EXPORT KisPixelOp {

public:

    KisPixelOp();
    KisPixelOp(KisColorSpace * cs);
	virtual ~KisPixelOp(){}
    KisID id() const { return m_id; }

    bool isValid() const { return m_valid; }
    virtual bool isNative() const { return false; }
    KisColorSpace * colorSpace() { return m_cs; }
//     
private:

    bool operator==(const KisPixelOp& other) const;
    bool operator!=(const KisPixelOp& other) const;
    KisPixelOp(const KisPixelOp & rhs);
    
private:
    KisID m_id;
    bool m_valid;
    KisColorSpace * m_cs;
};

#endif // KIS_PIXEL_OP_H
