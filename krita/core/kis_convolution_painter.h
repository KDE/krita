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

enum KisConvolutionBorderOp {
	BORDER_NOOP = 0,
#if 0
	BORDER_DEFAULT_FILL = 1,
	BORDER_WRAP = 2,
	BORDER_REPEAT = 3,
	BORDER_AVOID = 4
#endif	
};

struct KisKernel {
	Q_UINT32 width;
	Q_UINT32 height;
	Q_INT32 offset;
	Q_INT32 factor;
	QValueVector<Q_INT32> data;
};

class KisConvolutionPainter : public KisPainter
{

	typedef KisPainter super;

public:

        KisConvolutionPainter();
        KisConvolutionPainter(KisPaintDeviceSP device);

	void applyMatrix(KisMatrix3x3* matrix, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

	/**
	 * Convolve all channels in src using the specified kernel; there is only one kernel for all
	 * channels possible. By default the the border pixels are not convolved, that is, convolving
	 * starts with at (x + kernel.width/2, y + kernel.height) and stops at w - (kernel.width/2)
	 * and h - (kernel.width/2)
	 */
	void applyMatrix(KisKernel * kernel, KisPaintDeviceSP src, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, KisConvolutionBorderOp borderOp = BORDER_NOOP);

	/**
	 * Convolve all channels in src using the specified matrix. Only the first matrix of the array is 
	 * used, since this method is simply a front-end for the above method.
	 */
	void applyMatrix(KisMatrix3x3* matrix, KisPaintDeviceSP src, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);


};
#endif //KIS_CONVOLUTION_PAINTER_H_
