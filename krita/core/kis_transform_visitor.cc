/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de> filters
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org> right angle rotators
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
#include <kdebug.h>
#include <klocale.h>

#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_transform_visitor.h"
#include "kis_progress_display_interface.h"
#include "kis_iterators_pixel.h"
#include "kis_filter_strategy.h"

void KisTransformVisitor::rotateRight90(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
	dst -> setX(src -> getX());
	dst -> setY(src -> getY());

	Q_INT32 pixelSize = src -> pixelSize();
	QRect r = src -> exactBounds();
	Q_INT32 x = 0;

	for (Q_INT32 y = r.bottom(); y >= r.top(); --y) {
		KisHLineIteratorPixel hit = src -> createHLineIterator(r.x(), y, r.width(), false);
		KisVLineIterator vit = dst -> createVLineIterator(r.x() + x, r.y(), r.width(), true);

			while (!hit.isDone()) {
			if (hit.isSelected())  {
				memcpy(vit.rawData(), hit.rawData(), pixelSize);
			}
			++hit;
			++vit;
		}
		++x;
	}
}

void KisTransformVisitor::rotateLeft90(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
	dst -> setX(src -> getX());
	dst -> setY(src -> getY());

	Q_INT32 pixelSize = src -> pixelSize();
	QRect r = src -> exactBounds();
	Q_INT32 x = 0;

	for (Q_INT32 y = r.top(); y <= r.bottom(); ++y) {
		// Read the horizontal line from back to front, write onto the vertical column
		KisHLineIteratorPixel hit = src -> createHLineIterator(r.x(), y, r.width(), false);
		KisVLineIterator vit = dst -> createVLineIterator(r.x() + x, r.y(), r.width(), true);

		hit += r.width() - 1;
		while (!vit.isDone()) {
			if (hit.isSelected()) {
				memcpy(vit.rawData(), hit.rawData(), pixelSize);
			}
			--hit;
			++vit;
		}
		++x;
	}
}

void KisTransformVisitor::rotate180(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
	dst -> setX(src -> getX());
	dst -> setY(src -> getY());

	Q_INT32 pixelSize = src -> pixelSize();
	QRect r = src -> exactBounds();

	for (Q_INT32 y = r.top(); y <= r.bottom(); ++y) {
		KisHLineIteratorPixel srcIt = src -> createHLineIterator(r.x(), y, r.width(), false);
		KisHLineIterator dstIt = dst -> createHLineIterator(r.x(), r.y() + r.bottom() - y, r.width(), true);

		srcIt += r.width() - 1;
		while (!dstIt.isDone()) {
			if (srcIt.isSelected())  {
				memcpy(dstIt.rawData(), srcIt.rawData(), pixelSize);
			}
			--srcIt;
			++dstIt;
		}
	}
}

template <class iter> iter createIterator(KisPaintDevice *dev, Q_INT32 start, Q_INT32 lineNum, Q_INT32 len);

template <> KisHLineIteratorPixel createIterator <KisHLineIteratorPixel>
(KisPaintDevice *dev, Q_INT32 start, Q_INT32 lineNum, Q_INT32 len)
{
	return dev->createHLineIterator(start, lineNum, len, true);
}

template <> KisVLineIteratorPixel createIterator <KisVLineIteratorPixel>
(KisPaintDevice *dev, Q_INT32 start, Q_INT32 lineNum, Q_INT32 len)
{
	return dev->createVLineIterator(lineNum, start, len, true);
}

template <class iter> void calcDimensions (KisPaintDevice *dev, Q_INT32 &srcStart, Q_INT32 &srcLen, Q_INT32 &firstLine, Q_INT32 &numLines);

template <> void calcDimensions <KisHLineIteratorPixel>
(KisPaintDevice *dev, Q_INT32 &srcStart, Q_INT32 &srcLen, Q_INT32 &firstLine, Q_INT32 &numLines)
{
	if(dev->hasSelection())
	{
		QRect r = dev->selection()->selectedExactRect();
		r.rect(&srcStart, &firstLine, &srcLen, &numLines);
	}
	else
		dev->exactBounds(srcStart, firstLine, srcLen, numLines);
}

template <> void calcDimensions <KisVLineIteratorPixel>
(KisPaintDevice *dev, Q_INT32 &srcStart, Q_INT32 &srcLen, Q_INT32 &firstLine, Q_INT32 &numLines)
{
	if(dev->hasSelection())
	{
		QRect r = dev->selection()->selectedExactRect();
		r.rect(&firstLine, &srcStart, &numLines, &srcLen);
	}
	else
		dev->exactBounds(firstLine, srcStart, numLines, srcLen);
}


template <class T> void KisTransformVisitor::transformPass(KisPaintDevice *src, KisPaintDevice *dst, double floatscale, Q_INT32  shear, Q_INT32 dx, KisFilterStrategy *filterStrategy)
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
		dstSelection = dst->selection();
	else
		dstSelection = new KisSelection(dst, "dummy"); // essentially a dummy to be deleted
		
	calcDimensions <T>(src, srcStart, srcLen, firstLine, numLines);
	
	m_progressTotalSteps += numLines*srcLen*floatscale;;
	
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
		T srcIt = createIterator <T>(src, srcStart - extraLen, lineNum, srcLen+2*extraLen);
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

		T dstIt = createIterator <T>(dst, dstStart, lineNum, dstLen);
		T dstSelIt = createIterator <T>(dstSelection, dstStart, lineNum, dstLen);
		
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
				//Q_INT32 t = ((center - (srcpos<<8)) * invfscale)>>8;
				//Q_INT32 dt = (256 * invfscale)>>8;
				for(int srcpos = begin; srcpos <= end; srcpos++)
				{
					Q_UINT32 tmpw = filterStrategy->intValueAt(
								((center - (srcpos<<8)) * invfscale)>>8) * invfscale;
					
					tmpw >>=8;
					weight[num] = tmpw;
					colors[num] = &tmpLine[srcpos*pixelSize];
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
		m_progressStep += dstLen;
		emit notifyProgress(this,(m_progressStep * 100) / m_progressTotalSteps);
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
				    KisProgressDisplayInterface *progress, enumFilterType filterType)
{
        double fwidth;

        KisFilterStrategy *filterStrategy = 0;
	filterType= HERMITE_FILTER;
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
                case HERMITE_FILTER:
                        filterStrategy = new KisHermiteFilterStrategy();
                        break;
                case LANCZOS3_FILTER:
                        filterStrategy = new KisLanczos3FilterStrategy();
                        break;
                case MITCHELL_FILTER:
                        filterStrategy = new KisMitchellFilterStrategy();
                        break;
        }
	
	fwidth = filterStrategy->support();

        //progress info
        m_cancelRequested = false;
        progress -> setSubject(this, true, true);
	m_progressTotalSteps = 0;
	m_progressStep = 0;
	
QTime time;
time.start();
	KisPaintDeviceSP tmpdev = new KisPaintDevice(m_dev->colorStrategy(),"temporary");
printf("time taken to create tmp dev %d\n",time.restart());
	transformPass <KisHLineIteratorPixel>(m_dev, tmpdev, xscale, xshear, xtranslate, filterStrategy);
printf("time taken first pass %d\n",time.restart());
	if(m_dev->hasSelection())
		m_dev->selection()->clear();
printf("time taken to clear selection %d\n",time.restart());
	transformPass <KisVLineIteratorPixel>(tmpdev, m_dev, yscale, yshear, ytranslate, filterStrategy);

printf("time taken second pass %d\n",time.elapsed());

	//progress info
        emit notifyProgressDone(this);

	delete filterStrategy;
        return;
}
