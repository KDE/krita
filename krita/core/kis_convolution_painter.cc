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

void KisConvolutionPainter::applyMatrix(KisMatrix3x3* matrix, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	applyMatrix(matrix, m_device, x, y, w, h);
}

void KisConvolutionPainter::applyMatrix(KisKernel * kernel, KisPaintDeviceSP src, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, KisConvolutionBorderOp borderOp)
{
	// Don't try to convolve on an area smaller than the kernel
	if (w < kernel[0].width || h < kernel[0].height) return;

	m_cancelRequested = false;
	
	int lastProgressPercent = 0;
	emit notifyProgress(this, 0);

	Q_INT32 depth = src -> colorStrategy() -> nChannels();

	
	
}

void KisConvolutionPainter::applyMatrix(KisMatrix3x3 * matrix, KisPaintDeviceSP src, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{


	
	// XXX: Add checking of selections
	// kdDebug() << "Convolving on x: " << x << ", y: " << y << ", w: " << w << ", h: " << h << "\n";
	if (w < 3 || h < 3) {
		// kdDebug() << "Refusing to convolve on too small an area.\n";
		return;
	}
	
	m_cancelRequested = false;
	int lastProgressPercent = 0;
	emit notifyProgress(this, 0);

	Q_INT32 depth = src -> colorStrategy() -> nChannels();

	Q_INT32 top, left;

	left = x;
	top = y;

	Q_INT32 above=top;
	Q_INT32 below=top+1;
	Q_INT32 dstY=top;
	
	KisPixelRO pixels[9];
	{
		KisHLineIteratorPixel curIt = src->createHLineIterator(left, y, w, false);
		KisHLineIteratorPixel dstIt = src->createHLineIterator(left, dstY, w, true);
		KisHLineIteratorPixel afterIt = src->createHLineIterator(left, below, w, false);


		// Corner : left top
		KisPixel currentPixel = dstIt.pixel();
		++dstIt;
		pixels[ CONVOLUTION_PIXEL_CUR ] = curIt.oldPixel();
		++curIt;
		pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixel();
		++curIt;
		pixels[ CONVOLUTION_PIXEL_BOTTOM ] = afterIt.oldPixel();
		++afterIt;
		pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ] = afterIt.oldPixel();
		++afterIt;

		if (dstIt.isSelected())
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
			currentPixel = dstIt.pixel();
			++dstIt;
			pixels[ CONVOLUTION_PIXEL_LEFT ] = pixels[ CONVOLUTION_PIXEL_CUR ];
			pixels[ CONVOLUTION_PIXEL_CUR ] = pixels[ CONVOLUTION_PIXEL_RIGHT ];
			pixels[ CONVOLUTION_PIXEL_LEFTBOTTOM ] = pixels[ CONVOLUTION_PIXEL_BOTTOM ];
			pixels[ CONVOLUTION_PIXEL_BOTTOM ] = pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ];

			pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixel();
			++curIt;
			pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ] = afterIt.oldPixel();
			++afterIt;
			// XXX: do something useful with the selectedness
			if (dstIt.isSelected()) {
				for(int i = 0; i < depth; i++)
				{
					currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
								(	pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
									+ pixels[ CONVOLUTION_PIXEL_LEFT ][i] * matrix[i][1][0]
									+ pixels[ CONVOLUTION_PIXEL_RIGHT ][i] * matrix[i][1][2]
									+ pixels[ CONVOLUTION_PIXEL_LEFTBOTTOM ][i] * matrix[i][2][0]
									+ pixels[ CONVOLUTION_PIXEL_BOTTOM ][i] * matrix[i][2][1]
									+ pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ][i] * matrix[i][2][2]
								) * matrix[i].sum() / matrix[i].factor() / sums[i] + matrix[i].offset() ) );
				}
			}
		}
		// Corner : right top
		currentPixel = dstIt.pixel();
		if (dstIt.isSelected() ) {
			for(int i = 0; i < depth; i++)
			{
				int sum = matrix[i][1][1] + matrix[i][1][0] + matrix[i][2][0] + matrix[i][2][1];
				sum = (sum == 0) ? 1 : sum;
				// XXX: do something useful with the selectedness
	
				currentPixel[ i ] = QMAX( 0, QMIN( QUANTUM_MAX,
									(     pixels[ CONVOLUTION_PIXEL_CUR ][ i ] * matrix[i][1][1]
										+ pixels[ CONVOLUTION_PIXEL_RIGHT - 1 ][i] * matrix[i][1][0]
										+ pixels[ CONVOLUTION_PIXEL_BOTTOM - 1 ][i] * matrix[i][2][0]
										+ pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM - 1 ][i] * matrix[i][2][1] )
										* matrix[i].sum() / matrix[i].factor() / sum + matrix[i].offset() ) );
			}
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
		KisPixel currentPixel = dstIt.pixel();
		++dstIt;
		pixels[ CONVOLUTION_PIXEL_TOP ] = beforeIt.oldPixel();
		++beforeIt;
		pixels[ CONVOLUTION_PIXEL_RIGHTTOP ] = beforeIt.oldPixel();
		++beforeIt;
		pixels[ CONVOLUTION_PIXEL_CUR ] = curIt.oldPixel();
		++curIt;
		pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixel();
		++curIt;
		pixels[ CONVOLUTION_PIXEL_BOTTOM ] = afterIt.oldPixel();
		++afterIt;
		pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ] = afterIt.oldPixel();
		++afterIt;
		
		// XXX: do something useful with the selectedness
		if (dstIt.isSelected() ) {
	
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
		}
		// Body : body
		while( ! curIt.isDone() )
		{
			currentPixel = dstIt.pixel();
			++dstIt;
			pixels[ CONVOLUTION_PIXEL_LEFTTOP ] = pixels[ CONVOLUTION_PIXEL_TOP ];
			pixels[ CONVOLUTION_PIXEL_TOP ] = pixels[ CONVOLUTION_PIXEL_RIGHTTOP ];
			pixels[ CONVOLUTION_PIXEL_LEFT ] = pixels[ CONVOLUTION_PIXEL_CUR ];
			pixels[ CONVOLUTION_PIXEL_CUR ] = pixels[ CONVOLUTION_PIXEL_RIGHT ];
			pixels[ CONVOLUTION_PIXEL_LEFTBOTTOM ] = pixels[ CONVOLUTION_PIXEL_BOTTOM ];
			pixels[ CONVOLUTION_PIXEL_BOTTOM ] = pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ];
			pixels[ CONVOLUTION_PIXEL_RIGHTTOP ] = beforeIt.oldPixel();
			++beforeIt;
			pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixel();
			++curIt;
			pixels[ CONVOLUTION_PIXEL_RIGHTBOTTOM ] = afterIt.oldPixel();
			++afterIt;
			
			// XXX: do something useful with the selectedness
			if (dstIt.isSelected() ) {

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
		}

		// Body : right
		currentPixel = dstIt.pixel();
		
		// XXX: do something useful with the selectedness
		if (dstIt.isSelected() ) {


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
		
		}
		above++;
		y++;
		dstY++;
		below++;

		int progressPercent = ((y - top) * 100) / h;

		if (progressPercent > lastProgressPercent) {
			emit notifyProgress(this, progressPercent);
			lastProgressPercent = progressPercent;

			if (m_cancelRequested) {
				return;
			}
		}
	}

#if 0 // No convolution on the bottom row	
	{
		KisHLineIteratorPixel beforeIt = src->createHLineIterator(left, above, w, false);
		KisHLineIteratorPixel curIt = src->createHLineIterator(left, y, w, false);
		KisHLineIteratorPixel dstIt = src->createHLineIterator(left, dstY, w, true);

		// Corner : left bottom
		pixels[ CONVOLUTION_PIXEL_TOP ] = beforeIt.oldPixel();
		++beforeIt;
		pixels[ CONVOLUTION_PIXEL_RIGHTTOP ] = beforeIt.oldPixel();
		++beforeIt;
		KisPixel currentPixel = dstIt.pixel();
		++dstIt;
		pixels[ CONVOLUTION_PIXEL_CUR ] = curIt.oldPixel();
		++curIt;
		pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixel();
		++curIt;

		// XXX: do something useful with the selectedness
		if (dstIt.isSelected() ) {
	
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
			currentPixel = dstIt.pixel();
			++dstIt;
			pixels[ CONVOLUTION_PIXEL_LEFTTOP ] = pixels[ CONVOLUTION_PIXEL_TOP ];
			pixels[ CONVOLUTION_PIXEL_TOP ] = pixels[ CONVOLUTION_PIXEL_RIGHTTOP ];
			pixels[ CONVOLUTION_PIXEL_LEFT ] = pixels[ CONVOLUTION_PIXEL_CUR ];
			pixels[ CONVOLUTION_PIXEL_CUR ] = pixels[ CONVOLUTION_PIXEL_RIGHT ];
			pixels[ CONVOLUTION_PIXEL_RIGHTTOP ] = beforeIt.oldPixel();
			++beforeIt;
			pixels[ CONVOLUTION_PIXEL_RIGHT ] = curIt.oldPixel();
			++curIt;

			// XXX: do something useful with the selectedness
			if (dstIt.isSelected() ) {
	
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
		}
		// Corner : right bottom
		currentPixel = dstIt.pixel();
		// XXX: do something useful with the selectedness
		if (dstIt.isSelected() ) {

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
#endif
	emit notifyProgressDone(this);
}
