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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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


void KisConvolutionPainter::applyMatrix(KisMatrix3x3 * matrix, KisPaintDeviceSP src, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{

	KisKernel * kernel = new KisKernel();
	kernel -> width = 3;
	kernel -> height = 3;
	kernel -> factor = matrix[0].factor();
	kernel -> offset = matrix[0].offset();
	for (int row = 0; row < 3; ++row) {
		for (int col = 0; col < 3; ++col) {
			kernel -> data.push_back(matrix[0][col][row]);
			
		}
	}
	applyMatrix(kernel, src, x, y, w, h);
}


void KisConvolutionPainter::applyMatrix(KisKernel * kernel, KisPaintDeviceSP src, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h,
	KisConvolutionBorderOp borderOp,
	KisConvolutionChannelFlags channelFlags )
{
	// Make the area we cover as small as possible
	if (src -> hasSelection()) {

		if (src -> selection() -> isTotallyUnselected(QRect(x, y, w, h))) {
			return;
		}

		QRect r = src -> selection() -> extent().intersect(QRect(x, y, w, h));
		x = r.x();
		y = r.y();
		w = r.width();
		h = r.height();

	}

	// Determine the kernel's extent from the center pixel
	Q_INT32 kw, kh, kd;
	kw = kernel[0].width;
	kh = kernel[0].height;
	kd = (kw - 1) / 2;
	
	// Don't try to convolve on an area smaller than the kernel, or with a kernel that is not square or has no center pixel.
	if (w < kw || h < kh || kw != kh || kw&1 == 0 ) return;
	
	
	m_cancelRequested = false;
	int lastProgressPercent = 0;
	emit notifyProgress(this, 0);

	// The total number of channels in a pixel
	Q_INT32 depth = src -> colorStrategy() -> nChannels();
	
	// Determine which of the channels we are going to convolve.
	bool channels[depth];
	
 	vKisChannelInfoSP channelInfos = src -> colorStrategy() -> channels();
	vKisChannelInfoSP_cit it;
	
 	for (it = channelInfos.begin(); it != channelInfos.end(); ++it){

		if ((*it) -> channelType() == COLOR && channelFlags & CONVOLVE_COLOR)
			channels[(*it) -> pos()] = true;
		else if ((*it) -> channelType() == ALPHA && channelFlags & CONVOLVE_ALPHA)
			channels[(*it) -> pos()] = true;
		else if ((*it) -> channelType() == SUBSTANCE && channelFlags & CONVOLVE_SUBSTANCE)
			channels[(*it) -> pos()] = true;
		else if ((*it) -> channelType() == SUBSTRATE && channelFlags & CONVOLVE_SUBSTRATE)
			channels[(*it) -> pos()] = true;
		else
			channels[(*it) -> pos()] = false;
	}

	// Determine whether we convolve border pixels, or not.
	switch (borderOp) {
		case BORDER_DEFAULT_FILL :
			break;
		case BORDER_WRAP:
		case BORDER_REPEAT:
		case BORDER_AVOID:
		default :
			x += (kw - 1) / 2;
			y += (kh - 1) / 2;
			w -= kw - 1;
			h -= kh - 1;
	}
		
	// Iterate over all pixels in our rect.
	
	// XXX: Cache already seen pixels for the kernel iterator.?
	// XXX: Is it faster to use src->pixel(x,y, false)?

	// row == the y position of the pixel we want to change in the paint device
	for (int row = y; row < y + h; ++row) {

		// col = the x position of the pixel we want to change
		int col = x; 
		
		KisHLineIteratorPixel hit = src -> createHLineIterator(x, row, w, true);
		
		while (!hit.isDone()) {
			if (hit.isSelected()) {
				
				KisPixel curPixel = hit.pixel();
				
				int sums[depth];
				memset(&sums, 0, depth * sizeof(int));

				// Iterate over all contributing pixels that are covered by the kernel
				// krow = the y position in the kernel matrix
				for (Q_INT32 krow = 0; krow <  kh; ++krow) {
	
					// kx = the x position in the kernel matrix
					int kx = 0;
	
					// col - kd = the left starting point of the kernel as centered on our pixel
					// krow - kd = the offset for the top of the kernel as centered on our pixel
					// kw = the kernel of a matrix line
					KisHLineIteratorPixel kit = src -> createHLineIterator(col - kd, (row - kd) + krow, kw, false);
					while (!kit.isDone()) {
						KisPixelRO p = kit.oldPixel();
						Q_INT32 kval = kernel[0].data[(kw * krow) + kx];
						// Calculate for each channel of the current pixel the sum of all matrix pixels
						if (kval != 0) {
							for (int i = 0; i < depth;  ++i) {
								if (channels[i]) {
									sums[i] = sums[i] + (p[i] * kval);
								}
							}
						}
						++kx;
						++kit;
					}
				}
				
				// We got the total value of the channels of the pixels surrounding our pixel now, including the value of our own pixel,
				// multiplied with the values of kernel. Compute the weighted value for every channel.
				for (int i = 0; i < depth; ++i) {
					if (channels[i]) {
						curPixel[i] = CLAMP(((sums[i] / kernel[0].factor) + kernel[0].offset),
											0, QUANTUM_MAX );
					}
				}
			}
			++col;
			++hit;

		}

		int progressPercent = 100 - ((((y + h) - row) * 100) / h);
		
		if (progressPercent > lastProgressPercent) {
			emit notifyProgress(this, progressPercent);
			lastProgressPercent = progressPercent;

			if (m_cancelRequested) {
				return;
			}
		}		
	
	}
	
	emit notifyProgressDone(this);
}
