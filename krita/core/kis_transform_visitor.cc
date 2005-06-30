/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#include <qwmatrix.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_transform_visitor.h"
#include "kis_progress_display_interface.h"
#include "kis_iterators_pixel.h"

double KisSimpleFilterStrategy::valueAt(double t) const {
        /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
        if(t < 0.0) t = -t;
        if(t < 1.0) return((2.0 * t - 3.0) * t * t + 1.0);
        return(0.0);
}

Q_UINT32 KisSimpleFilterStrategy::intValueAt(Q_INT32 t) const {
        /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
        if(t < 0) t = -t;
        if(t < 256)
	{
	 t =(2 * t - 3*256) * t * t +(256<<16);
	 
	 //go from .24 fixed point to .8 fixedpoint (hack only works wit positve numbers)
	 t = (t+0x8000) >> 16;
	 
	 // go from .8 fixed point to 8bitscale. ie t = (t*255)/256;
	 if(t>=128) return t-1;
	 return t;
	}
        return(0);
}

double KisBoxFilterStrategy::valueAt(double t) const {
        if((t > -0.5) && (t <= 0.5)) return(1.0);
        return(0.0);
}

double KisTriangleFilterStrategy::valueAt(double t) const {
        if(t < 0.0) t = -t;
        if(t < 1.0) return(1.0 - t);
        return(0.0);
}

double KisBellFilterStrategy::valueAt(double t) const {
        if(t < 0) t = -t;
        if(t < .5) return(.75 - (t * t));
        if(t < 1.5) {
                t = (t - 1.5);
                return(.5 * (t * t));
        }
        return(0.0);
}

double KisBSplineFilterStrategy::valueAt(double t) const {
        double tt;

        if(t < 0) t = -t;
        if(t < 1) {
                tt = t * t;
                return((.5 * tt * t) - tt + (2.0 / 3.0));
        } else if(t < 2) {
                t = 2 - t;
                return((1.0 / 6.0) * (t * t * t));
        }
        return(0.0);
}

double KisLanczos3FilterStrategy::valueAt(double t) const {
        if(t < 0) t = -t;
        if(t < 3.0) return(sinc(t) * sinc(t/3.0));
        return(0.0);
}

double KisLanczos3FilterStrategy::sinc(double x) const {
        const double pi=3.1415926535897932385;
        x *= pi;
        if(x != 0) return(sin(x) / x);
        return(1.0);
}

double KisMitchellFilterStrategy::valueAt(double t) const {
        const double B=1.0/3.0;
        const double C=1.0/3.0;
        double tt;

        tt = t * t;
        if(t < 0) t = -t;
        if(t < 1.0) {
                t = (((12.0 - 9.0 * B - 6.0 * C) * (t * tt)) + ((-18.0 + 12.0 * B + 6.0 * C) * tt) + (6.0 - 2 * B));
                return(t / 6.0);
        } else if(t < 2.0) {
                t = (((-1.0 * B - 6.0 * C) * (t * tt)) + ((6.0 * B + 30.0 * C) * tt) + ((-12.0 * B - 48.0 * C) * t) + (8.0 * B + 24 * C));
                return(t / 6.0);
                }
        return(0.0);
}


void KisTransformVisitor::transformx(KisPaintDevice *src, KisPaintDevice *dst, double floatscale, Q_INT32  shear, Q_INT32 dx, KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy)
{
	Q_INT32 lineNum,srcStart,firstLine,srcLen,numLines;
        Q_INT32 center, begin, end;	/* filter calculation variables */
	Q_UINT8 *data;
	Q_UINT8 pixelSize = src->pixelSize();
	KisSelectionSP dstSelection;
	KisStrategyColorSpaceSP cs = src->colorStrategy();
	Q_INT32 scale;
	Q_INT32 scaleDenom;
	
	if(src->hasSelection())
	{
		QRect r = src->selection()->selectedExactRect();
		r.rect(&srcStart, &firstLine, &srcLen, &numLines);
		dstSelection = dst->selection();
	}
	else
	{
		src->exactBounds(srcStart, firstLine, srcLen, numLines);
		dstSelection = new KisSelection(dst, "dummy"); // essentially a dummy to be deleted
	}
	scale = int(floatscale*srcLen);
	scaleDenom = srcLen;
		
	Q_UINT8 *weight = new Q_UINT8[100];
	const Q_UINT8 *colors[100];
	Q_INT32 support = int(256 * filterStrategy->support());
	Q_INT32  dstLen, dstStart;
	Q_INT32 invfscale = 256;
	
	// handle magnification/minification
	if(abs(scale) < scaleDenom)
	{
		support *= scaleDenom;
		support /= scale;
		
		invfscale *= scale;
		invfscale /= scaleDenom;
		if(scale < 0) // handle mirroring
		{
			support = -support;
			invfscale = -invfscale;
		}
	}
	
	// handle mirroring
	if(scale < 0)
		dstLen = -srcLen * scale / scaleDenom;
	else
		dstLen = srcLen * scale / scaleDenom;
	
	
	// Calculate extra length needed due to shear
	Q_INT32 extraLen = (support+256)>>8;
	
	Q_UINT8 *tmpLine = new Q_UINT8[(srcLen +2*extraLen)* pixelSize];
	Q_CHECK_PTR(tmpLine);

	Q_UINT8 *tmpSel = new Q_UINT8[srcLen+2*extraLen];
	Q_CHECK_PTR(tmpSel);
	
	kdDebug(DBG_AREA_CORE) << "srcLen=" << srcLen << " dstLen" << dstLen << " scale=" << scale << " sDenom=" <<scaleDenom << endl;
	kdDebug(DBG_AREA_CORE) << "srcStart="<< srcStart << ",dx=" << dx << endl;
	kdDebug(DBG_AREA_CORE) << "extraLen="<< extraLen << endl;
	
	for(lineNum = firstLine; lineNum < firstLine+numLines; lineNum++)
	{
		if(scale < 0)
			dstStart = srcStart * scale / scaleDenom - dstLen + dx;
		else
			dstStart = (srcStart) * scale / scaleDenom + dx;
		
		// Build a temporary line
		KisHLineIteratorPixel srcIt = src->createHLineIterator(srcStart - extraLen, lineNum, srcLen+2*extraLen, true);
		int i = 0;
		while(!srcIt.isDone())
		{
			Q_UINT8 *data;
			
			if(srcIt.isSelected())
			{
				data = srcIt.rawData();
				memcpy(&tmpLine[i*pixelSize], data, pixelSize);
				
				// XXX: Find a way to colorstrategy independently set alpha = alpha*(1-selectedness)
				// but for now this will do
				*(data+3) = 0;
				
				tmpSel[i] = 255;
			}
			else
				tmpSel[i] = 0;
			++srcIt;
			i++;
		}

		KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstStart, lineNum, dstLen, true);
		KisHLineIteratorPixel dstSelIt = dstSelection->createHLineIterator(dstStart, lineNum, dstLen, true);
		
		i=0;
		while(!dstIt.isDone())
		{
			if(scale < 0)
				center = (srcLen<<8) + (((i * scaleDenom))<<8) / scale;
			else
				center = ((i * scaleDenom)<<8) / scale;
			
			center += (extraLen<<8);
			
			// find contributing pixels
			begin = (256 + center - support)>>8; // takes ceiling by adding 256
			end = (center + support)>>8; // takes floor
						
			Q_UINT8 selectedness = tmpSel[center>>8];
			if(selectedness)
			{
				// calculate weights
				int num = 0;
				int sum = 0;
				//Q_INT32 t = ((center - (srcpos<<8)) * invfscale)>>8;
				//Q_INT32 dt = (256 * invfscale)>>8;
				for(int srcpos = begin; srcpos <= end; srcpos++)
				{
					Q_UINT32 tmpw = filterStrategy->intValueAt(
								((center - (srcpos<<8)) * invfscale)>>8) * invfscale;
					
					tmpw >>=8;
					weight[num] = tmpw;
					colors[num] = &tmpLine[begin*pixelSize];
					num++;
				}				
				data = dstIt.rawData();
				cs->mixColors(colors, weight, num, data);
				data = dstSelIt.rawData();
				*data = selectedness;
			}
			
			++dstSelIt;
			++dstIt;
			i++;
		}
		
		//progress info
		emit notifyProgress(this,((lineNum-firstLine) * 100) / numLines);
		if (m_cancelRequested) {
			break;
		}
	}
	delete [] tmpLine;
	delete [] tmpSel;
	delete [] weight;
}

void KisTransformVisitor::transformy(KisPaintDevice *src, KisPaintDevice *dst, double floatscale, Q_INT32  shear, Q_INT32 dx, KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy)
{
	Q_INT32 lineNum,srcStart,firstLine,srcLen,numLines;
        Q_INT32 center, begin, end;	/* filter calculation variables */
	Q_UINT8 *data;
	Q_UINT8 pixelSize = src->pixelSize();
	KisSelectionSP dstSelection;
	KisStrategyColorSpaceSP cs = src->colorStrategy();
	Q_INT32 scale;
	Q_INT32 scaleDenom;
	
	if(src->hasSelection())
	{
		QRect r = src->selection()->selectedExactRect();
		r.rect(&firstLine, &srcStart, &numLines, &srcLen);
		dstSelection = dst->selection();
	}
	else
	{
		src->exactBounds(firstLine, srcStart, numLines, srcLen);
		dstSelection = new KisSelection(dst, "dummy"); // essentially a dummy to be deleted
	}
	scale = int(floatscale*srcLen);
	scaleDenom = srcLen;
		
	Q_UINT8 *weight = new Q_UINT8[100];
	const Q_UINT8 *colors[100];
	Q_INT32 support = int(256 * filterStrategy->support());
	Q_INT32  dstLen, dstStart;
	Q_INT32 invfscale = 256;
	
	// handle magnification/minification
	if(abs(scale) < scaleDenom)
	{
		support *= scaleDenom;
		support /= scale;
		
		invfscale *= scale;
		invfscale /= scaleDenom;
		if(scale < 0) // handle mirroring
		{
			support = -support;
			invfscale = -invfscale;
		}
	}
	
	// handle mirroring
	if(scale < 0)
		dstLen = -srcLen * scale / scaleDenom;
	else
		dstLen = srcLen * scale / scaleDenom;
	
	
	// Calculate extra length needed due to shear
	Q_INT32 extraLen = (support+256)>>8;
	
	Q_UINT8 *tmpLine = new Q_UINT8[(srcLen +2*extraLen)* pixelSize];
	Q_CHECK_PTR(tmpLine);

	Q_UINT8 *tmpSel = new Q_UINT8[srcLen+2*extraLen];
	Q_CHECK_PTR(tmpSel);
	
	kdDebug(DBG_AREA_CORE) << "srcLen=" << srcLen << " dstLen" << dstLen << " scale=" << scale << " sDenom=" <<scaleDenom << endl;
	kdDebug(DBG_AREA_CORE) << "srcStart="<< srcStart << ",dx=" << dx << endl;
	kdDebug(DBG_AREA_CORE) << "extraLen="<< extraLen << endl;
	
	for(lineNum = firstLine; lineNum < firstLine+numLines; lineNum++)
	{
		if(scale < 0)
			dstStart = srcStart * scale / scaleDenom - dstLen + dx;
		else
			dstStart = (srcStart) * scale / scaleDenom + dx;
		
		// Build a temporary line
		KisVLineIteratorPixel srcIt = src->createVLineIterator(lineNum, srcStart - extraLen,  srcLen+2*extraLen, true);
		int i = 0;
		while(!srcIt.isDone())
		{
			Q_UINT8 *data;
			
			if(srcIt.isSelected())
			{
				data = srcIt.rawData();
				memcpy(&tmpLine[i*pixelSize], data, pixelSize);
				
				// XXX: Find a way to colorstrategy independently set alpha = alpha*(1-selectedness)
				// but for now this will do
				*(data+3) = 0;
				
				tmpSel[i] = 255;
			}
			else
				tmpSel[i] = 0;
			++srcIt;
			i++;
		}

		KisVLineIteratorPixel dstIt = dst->createVLineIterator(lineNum, dstStart, dstLen, true);
		KisVLineIteratorPixel dstSelIt = dstSelection->createVLineIterator(lineNum, dstStart, dstLen, true);
		
		i=0;
		while(!dstIt.isDone())
		{
			if(scale < 0)
				center = (srcLen<<8) + (((i * scaleDenom))<<8) / scale;
			else
				center = ((i * scaleDenom)<<8) / scale;
			
			center += (extraLen<<8);
			
			// find contributing pixels
			begin = (256 + center - support)>>8; // takes ceiling by adding 256
			end = (center + support)>>8; // takes floor
						
			Q_UINT8 selectedness = tmpSel[center>>8];
			if(selectedness)
			{
				// calculate weights
				int num = 0;
				int sum = 0;
				//Q_INT32 t = ((center - (srcpos<<8)) * invfscale)>>8;
				//Q_INT32 dt = (256 * invfscale)>>8;
				for(int srcpos = begin; srcpos <= end; srcpos++)
				{
					Q_UINT32 tmpw = filterStrategy->intValueAt(
								((center - (srcpos<<8)) * invfscale)>>8) * invfscale;
					
					tmpw >>=8;
					weight[num] = tmpw;
					colors[num] = &tmpLine[begin*pixelSize];
					num++;
				}				
				data = dstIt.rawData();
				cs->mixColors(colors, weight, num, data);
				data = dstSelIt.rawData();
				*data = selectedness;
			}
			
			++dstSelIt;
			++dstIt;
			i++;
		}
		
		//progress info
		emit notifyProgress(this,((lineNum-firstLine) * 100) / numLines);
		if (m_cancelRequested) {
			break;
		}
	}
	delete [] tmpLine;
	delete [] tmpSel;
	delete [] weight;
}

void KisTransformVisitor::transform(double  xscale, double  yscale, 
				    Q_INT32  xshear, Q_INT32  yshear,
				    Q_INT32  xtranslate, Q_INT32  ytranslate,
				    KisProgressDisplayInterface *m_progress, enumFilterType filterType)
{
        double fwidth;

        KisFilterStrategy *filterStrategy = 0;
filterType=FILTER;
        switch(filterType){
                case BOX_FILTER:
                        filterStrategy = new KisBoxFilterStrategy();
                        break;
                case TRIANGLE_FILTER:
                        filterStrategy = new KisTriangleFilterStrategy();
                        break;
                case BELL_FILTER:
                        filterStrategy = new KisBellFilterStrategy();
                        break;
                case B_SPLINE_FILTER:
                        filterStrategy = new KisBSplineFilterStrategy();
                        break;
                case FILTER:
                        filterStrategy = new KisSimpleFilterStrategy();
                        break;
                case LANCZOS3_FILTER:
                        filterStrategy = new KisLanczos3FilterStrategy();
                        break;
                case MITCHELL_FILTER:
                        filterStrategy = new KisMitchellFilterStrategy();
                        break;
        }
	
	fwidth = filterStrategy->support();

	m_cancelRequested = false;
	
	KisPaintDeviceSP tmpdev = new KisPaintDevice(m_dev->colorStrategy(),"temporary");
	transformx(m_dev, tmpdev, xscale, xshear, xtranslate, m_progress, filterStrategy);
	if(m_dev->hasSelection())
		m_dev->selection()->clear();
	transformy(tmpdev, m_dev, yscale, yshear, ytranslate, m_progress, filterStrategy);
	
	//progress info
        emit notifyProgressDone(this);

        //return nRet;
        return;
}
