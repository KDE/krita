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

#include <stdlib.h>
#include <string.h>
#include <cfloat>

#include "qbrush.h"
#include "qcolor.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#include "qpen.h"
#include "qregion.h"
#include "qwmatrix.h"
#include <qimage.h>
#include <qmap.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qrect.h>
#include <qstring.h>

#include <kdebug.h>
#include <klocale.h>

#include <qcolor.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_image.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "kis_rect.h"
#include "kis_strategy_colorspace.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_selection.h"
#include "kis_convolution_painter.h"

namespace {

	enum enumConvolutionPixels {
		CONVOLUTION_PIXEL_LEFTTOP = 0,
		CONVOLUTION_PIXEL_TOP = 1,
		CONVOLUTION_PIXEL_RIGHTTOP = 2,
		CONVOLUTION_PIXEL_LEFT = 3,
		CONVOLUTION_PIXEL_CUR = 4,
		CONVOLUTION_PIXEL_RIGHT = 5,
		CONVOLUTION_PIXEL_LEFTBOTTOM = 6,
		CONVOLUTION_PIXEL_BOTTOM = 7,
		CONVOLUTION_PIXEL_RIGHTBOTTOM = 8,
	};
}

KisConvolutionPainter::KisConvolutionPainter()
	: super()
{
}

KisConvolutionPainter::KisConvolutionPainter(KisPaintDeviceSP device) : super(device)
{
}

void KisConvolutionPainter::applyConvolutionColorTransformation(KisMatrix3x3* matrix)
{
	applyConvolutionColorTransformation(matrix, m_device);
}
void KisConvolutionPainter::applyConvolutionColorTransformation(KisMatrix3x3* matrix, KisPaintDeviceSP src)
{
	// XXX: add checking of selections

	Q_INT32 depth = src -> colorStrategy() -> pixelSize();
	Q_INT32 y, top, left, w, h;
	src->extent(left, top, w, h);

	Q_INT32 above=top;
	Q_INT32 below=top+1;
	Q_INT32 dstY=top;
	y=top;

	KisPixelRO pixels[9];
	{
		KisHLineIteratorPixel curIt = src->createHLineIterator(y,left,w,false);
		KisHLineIteratorPixel dstIt = src->createHLineIterator(dstY,left,w,true);
		KisHLineIteratorPixel afterIt = src->createHLineIterator(below,left,w,false);
		// Corner : left top
		KisPixel currentPixel = dstIt.value();
		dstIt++;
		pixels[ CONVOLUTION_PIXEL_CUR ] = curIt.oldPixelValue();
		curIt++;
		pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixelValue();
		curIt++;
		pixels[ CONVOLUTION_PIXEL_BOTTOM ] = afterIt.oldPixelValue();
		afterIt++;
		pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ] = afterIt.oldPixelValue();
		afterIt++;
		for(int i = 0; i < depth; i++)
		{
			int sum = matrix[i][1][1] + matrix[i][1][2] + matrix[i][2][1] + matrix[i][2][2];
			sum = (sum == 0) ? 1 : sum;
			currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
								(     pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
									  + pixels[ CONVOLUTION_PIXEL_RIGHT ][i] * matrix[i][1][2]
									  + pixels[ CONVOLUTION_PIXEL_BOTTOM ][i] * matrix[i][2][1]
									  + pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ][i] * matrix[i][2][2] )
									  * matrix[i].sum() / matrix[i].factor() / sum + matrix[i].offset() ) );
		}
		// Border : top
		int sums[depth];
		for(int i = 0; i < depth; i++)
		{
			sums[i] = matrix[i][1][1] + matrix[i][1][0] + matrix[i][1][2] + matrix[i][2][0] +  matrix[i][2][1] + matrix[i][2][2];
			sums[i] = (sums[i] == 0) ? 1 : sums[i];
		}

		while( ! curIt.isDone() )
		{
			currentPixel = dstIt.value();
			dstIt++;
			memmove( pixels, pixels + 1, 8 * sizeof(KisPixelRO));
			pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixelValue();
			curIt++;
			pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ] = afterIt.oldPixelValue();
			afterIt++;
			for(int i = 0; i < depth; i++)
			{
				currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
									(     pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
										  + pixels[ CONVOLUTION_PIXEL_LEFT ][i] * matrix[i][1][0]
										  + pixels[ CONVOLUTION_PIXEL_RIGHT ][i] * matrix[i][1][2]
										  + pixels[ CONVOLUTION_PIXEL_LEFTBOTTOM ][i] * matrix[i][2][0]
										  + pixels[ CONVOLUTION_PIXEL_BOTTOM ][i] * matrix[i][2][1]
										  + pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ][i] * matrix[i][2][2] )
									* matrix[i].sum() / matrix[i].factor() / sums[i] + matrix[i].offset() ) );
			}
		}
		// Corner : right top
		currentPixel = dstIt.value();
		for(int i = 0; i < depth; i++)
		{
			int sum = matrix[i][1][1] + matrix[i][1][0] + matrix[i][2][0] + matrix[i][2][1];
			sum = (sum == 0) ? 1 : sum;
			currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
								(     pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
									  + pixels[ CONVOLUTION_PIXEL_RIGHT - 1 ][i] * matrix[i][1][0]
									  + pixels[ CONVOLUTION_PIXEL_BOTTOM - 1 ][i] * matrix[i][2][0]
									  + pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM - 1 ][i] * matrix[i][2][1] )
									  * matrix[i].sum() / matrix[i].factor() / sum + matrix[i].offset() ) );
		}
	}

	// Body
	int rightSums[depth];
	int leftSums[depth];
	for(int i = 0; i < depth; i++)
	{
		rightSums[i] = matrix[i][1][1] + matrix[i][0][1] + matrix[i][0][2] + matrix[i][1][2] + matrix[i][2][1] + matrix[i][2][2];
		rightSums[i] = (rightSums[i] == 0) ? 1 : rightSums[i];
		leftSums[i] = matrix[i][1][1] + matrix[i][0][0] + matrix[i][0][1] + matrix[i][1][0] + matrix[i][2][0] + matrix[i][2][1];
		leftSums[i] = (leftSums[i] == 0) ? 1 : leftSums[i];
	}
	y++;
	dstY++;
	below++;
	while( y < top+h )
	{
		KisHLineIteratorPixel beforeIt = src->createHLineIterator(left, above, w, false);
		KisHLineIteratorPixel curIt = src->createHLineIterator(left, y, w, false);
		KisHLineIteratorPixel dstIt = src->createHLineIterator(left, dstY, w, true);
		KisHLineIteratorPixel afterIt = src->createHLineIterator(left, below, w, false);

		// Body : left border
		KisPixel currentPixel = dstIt.value();
		dstIt++;
		pixels[ CONVOLUTION_PIXEL_TOP ] = beforeIt.oldPixelValue();
		beforeIt++;
		pixels[ CONVOLUTION_PIXEL_RIGHTTOP ] = beforeIt.oldPixelValue();
		beforeIt++;
		pixels[ CONVOLUTION_PIXEL_CUR ] = curIt.oldPixelValue();
		curIt++;
		pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixelValue();
		curIt++;
		pixels[ CONVOLUTION_PIXEL_BOTTOM ] = afterIt.oldPixelValue();
		afterIt++;
		pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ] = afterIt.oldPixelValue();
		afterIt++;
		for(int i = 0; i < depth; i++)
		{
			currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
									(   pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
									  + pixels[ CONVOLUTION_PIXEL_TOP ][i] * matrix[i][0][1]
									  + pixels[ CONVOLUTION_PIXEL_RIGHTTOP ][i] * matrix[i][0][2]
									  + pixels[ CONVOLUTION_PIXEL_RIGHT ][i] * matrix[i][1][2]
									  + pixels[ CONVOLUTION_PIXEL_BOTTOM ][i] * matrix[i][2][1]
									  + pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ][i] * matrix[i][2][2] )
									* matrix[i].sum() / matrix[i].factor() / rightSums[i] + matrix[i].offset() ) );
		}
		// Body : body
		while( ! curIt.isDone() )
		{
			currentPixel = dstIt.value();
			dstIt++;
			memmove( pixels, pixels + 1, 8 * sizeof(KisPixelRO));
			pixels[ CONVOLUTION_PIXEL_RIGHTTOP ] = beforeIt.oldPixelValue();
			beforeIt++;
			pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixelValue();
			curIt++;
			pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ] = afterIt.oldPixelValue();
			afterIt++;
			for(int i = 0; i < depth; i++)
			{
				currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
										(   pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
										  + pixels[ CONVOLUTION_PIXEL_LEFTTOP ][i] * matrix[i][0][0]
										  + pixels[ CONVOLUTION_PIXEL_TOP ][i] * matrix[i][0][1]
										  + pixels[ CONVOLUTION_PIXEL_RIGHTTOP ][i] * matrix[i][0][2]
										  + pixels[ CONVOLUTION_PIXEL_LEFT ][i] * matrix[i][1][0]
										  + pixels[ CONVOLUTION_PIXEL_RIGHT ][i] * matrix[i][1][2]
										  + pixels[ CONVOLUTION_PIXEL_LEFTBOTTOM ][i] * matrix[i][2][0]
										  + pixels[ CONVOLUTION_PIXEL_BOTTOM ][i] * matrix[i][2][1]
										  + pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ][i] * matrix[i][2][2] )
										/ matrix[i].factor() + matrix[i].offset() ) );
			}
		}
		// Body : right
		currentPixel = dstIt.value();
		for(int i = 0; i < depth; i++)
		{
			currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
									( pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
									  + pixels[ CONVOLUTION_PIXEL_TOP - 1 ][i] * matrix[i][0][0]
									  + pixels[ CONVOLUTION_PIXEL_RIGHTTOP - 1 ][i] * matrix[i][0][1]
									  + pixels[ CONVOLUTION_PIXEL_RIGHT - 1 ][i] * matrix[i][1][0]
									  + pixels[ CONVOLUTION_PIXEL_BOTTOM - 1 ][i] * matrix[i][2][0]
									  + pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM - 1 ][i] * matrix[i][2][1] )
									* matrix[i].sum() / matrix[i].factor() / leftSums[i] + matrix[i].offset() ) );
		}
		above++;
		y++;
		dstY++;
		below++;
	}
	{
		KisHLineIteratorPixel beforeIt = src->createHLineIterator(left, above, w, false);
		KisHLineIteratorPixel curIt = src->createHLineIterator(left, y, w, false);
		KisHLineIteratorPixel dstIt = src->createHLineIterator(left, dstY, w, true);

		// Corner : left bottom
		pixels[ CONVOLUTION_PIXEL_TOP ] = beforeIt.oldPixelValue();
		beforeIt++;
		pixels[ CONVOLUTION_PIXEL_RIGHTTOP ] = beforeIt.oldPixelValue();
		beforeIt++;
		KisPixel currentPixel = dstIt.value();
		dstIt++;
		pixels[ CONVOLUTION_PIXEL_CUR ] = curIt.oldPixelValue();
		curIt++;
		pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixelValue();
		curIt++;
		for(int i = 0; i < depth; i++)
		{
			int sum = matrix[i][1][1] + matrix[i][0][1] + matrix[i][0][2] + matrix[i][1][2];
			sum = (sum == 0) ? 1 : sum;
			currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
									(   pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
									  + pixels[ CONVOLUTION_PIXEL_TOP ][i] * matrix[i][0][1]
									  + pixels[ CONVOLUTION_PIXEL_RIGHTTOP ][i] * matrix[i][0][2]
									  + pixels[ CONVOLUTION_PIXEL_RIGHT ][i] * matrix[i][1][2] ) * matrix[i].sum()
									/ matrix[i].factor() / sum + matrix[i].offset() ) );
		}
		// Border : bottom
		int sums[depth];
		for(int i = 0; i < depth; i++)
		{
			sums[i] = matrix[i][1][1] + matrix[i][1][0] + matrix[i][1][2] + matrix[i][0][0] +  matrix[i][0][1] + matrix[i][0][2];
			sums[i] = (sums[i] == 0) ? 1 : sums[i];
		}
		while( ! curIt.isDone() )
		{
			currentPixel = dstIt.value();
			dstIt++;
			memmove( pixels, pixels + 1, 8 * sizeof(KisPixelRO));
			pixels[ CONVOLUTION_PIXEL_RIGHTTOP ] = beforeIt.oldPixelValue();
			beforeIt++;
			pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixelValue();
			curIt++;
			for(int i = 0; i < depth; i++)
			{
				currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
										(   pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
										  + pixels[ CONVOLUTION_PIXEL_LEFTTOP ][i] * matrix[i][0][0]
										  + pixels[ CONVOLUTION_PIXEL_TOP ][i] * matrix[i][0][1]
										  + pixels[ CONVOLUTION_PIXEL_RIGHTTOP ][i] * matrix[i][0][2]
										  + pixels[ CONVOLUTION_PIXEL_LEFT ][i] * matrix[i][1][0]
										  + pixels[ CONVOLUTION_PIXEL_RIGHT ][i] * matrix[i][1][2] ) * matrix[i].sum()
										/ matrix[i].factor() / sums[i] + matrix[i].offset() ) );
			}
		}

	// Corner : right bottom
		currentPixel = dstIt.value();
		for(int i = 0; i < depth; i++)
		{
			int sum = matrix[i][1][1] + matrix[i][0][0] + matrix[i][0][1] + matrix[i][1][0];
			sum = (sum == 0) ? 1 : sum;
			currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
									(   pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
									  + pixels[ CONVOLUTION_PIXEL_LEFTTOP + 1 ][i] * matrix[i][0][0]
									  + pixels[ CONVOLUTION_PIXEL_TOP + 1 ][i] * matrix[i][0][1]
									  + pixels[ CONVOLUTION_PIXEL_LEFT + 1 ][i] * matrix[i][1][0] ) * matrix[i].sum()
									/ matrix[i].factor() / sum + matrix[i].offset() ) );
		}
	}
}

