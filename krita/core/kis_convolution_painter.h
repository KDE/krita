/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_CONVOLUTION_PAINTER_H_
#define KIS_CONVOLUTION_PAINTER_H_

#include <qbrush.h>
#include <qcolor.h>
#include <qfontinfo.h>
#include <qfontmetrics.h>
#include <qpen.h>
#include <qregion.h>
#include <qwmatrix.h>
#include <qimage.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qstring.h>
#include <qpainter.h>
#include <qvaluevector.h>

#include <kcommand.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_point.h"
#include "kis_matrix.h"
#include "kis_progress_subject.h"
#include "kis_painter.h"
#include "koffice_export.h"

enum KisConvolutionBorderOp {
	BORDER_DEFAULT_FILL = 0, // Use the default pixel to make up for the missing pixels on the border or the pixel that lies beyond
	                         // the rect we are convolving.
	BORDER_WRAP = 1, // Use the pixel on the opposite side to make up for the missing pixels on the border. XXX: Not implemented yet
	BORDER_REPEAT = 2, // Use the border for the missing pixels, too. XXX: Not implemented yet.
	BORDER_AVOID = 3 // Skip convolving the border pixels at all.
};

enum KisConvolutionChannelFlags {
	CONVOLVE_COLOR = 1,
	CONVOLVE_ALPHA = (1 << 1),
	CONVOLVE_SUBSTANCE = (1 << 2),
	CONVOLVE_SUBSTRATE = (1 << 3)
};

struct KisKernel {
	Q_UINT32 width;
	Q_UINT32 height;
	Q_INT32 offset;
	Q_INT32 factor;
	QValueVector<Q_INT32> data;
};

class KRITACORE_EXPORT KisConvolutionPainter : public KisPainter
{

	typedef KisPainter super;

public:

        KisConvolutionPainter();
        KisConvolutionPainter(KisPaintDeviceSP device);

	/**
	 * Convolve all channels in the current paint device using the specified matrix. Only the first matrix of the array is
	 * used.
	 */
	void applyMatrix(KisMatrix3x3* matrix, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

	/**
	 * Convolve all channels in src using the specified matrix. Only the first matrix of the array is 
	 * used.
	 */
	void applyMatrix(KisMatrix3x3* matrix, KisPaintDeviceSP src, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

	/**
	 * Convolve all channels in src using the specified kernel; there is only one kernel for all
	 * channels possible. By default the the border pixels are not convolved, that is, convolving
	 * starts with at (x + kernel.width/2, y + kernel.height) and stops at w - (kernel.width/2)
	 * and h - (kernel.width/2)
	 *
	 * The border op decides what to do with pixels too close to the edge of the rect as defined above.
	 *
	 * The channels flag determines which set out of color channels, alpha channels, substance or substrate
	 * channels we convolve.
	 *
	 * Note that we do not (currently) support different kernels for different channels _or_ channel types.
	 */
	void applyMatrix(KisKernel * kernel, KisPaintDeviceSP src, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h,
		KisConvolutionBorderOp borderOp = BORDER_AVOID,
		KisConvolutionChannelFlags channelFlags = CONVOLVE_COLOR);



};
#endif //KIS_CONVOLUTION_PAINTER_H_
