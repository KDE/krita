/* This file is part of the KDE project
   Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>

   ported from digikam, Copyright 2004 by Gilles Caulier,
   Original Oilpaint algorithm copyrighted 2004 by 
   Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>
#include <vector>

#include <qpoint.h>
#include <qspinbox.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_tile_command.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kistile.h>
#include <kistilemgr.h>

#include "kis_filter_configuration_widget.h"
#include "kis_oilpaint_filter_configuration_widget.h"
#include "kis_oilpaint_filter_configuration_base_widget.h"
#include "kis_oilpaint_filter.h"

KisOilPaintFilter::KisOilPaintFilter(KisView * view) : KisFilter(name(), view)
{
}

void KisOilPaintFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect, KisTileCommand* ktc)
{
	kdDebug() << "Oilpaintfilter 2 called!\n";
        
        Q_INT32 width = src->width();
        Q_INT32 height = src->height();
        
        // create a QUANTUM array that holds the data the filter works on
        
        QUANTUM * newData = new QUANTUM[width * height * src -> depth() * sizeof(QUANTUM)];
        
        // create iterators for the src and the dst image
        
        KisIteratorLinePixel lineIt = src->iteratorPixelSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
	KisIteratorLinePixel dstLineIt = dst->iteratorPixelSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
	KisIteratorLinePixel lastLine = src->iteratorPixelSelectionEnd(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);
	KisIteratorLinePixel dstLastLine = src->iteratorPixelSelectionEnd(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);
        
        Q_INT32 depth = src->depth();
        
        Q_UINT32 x=0;
        
        //read pixel data from image into QUANTUM array using iterators
        
        while( lineIt <= lastLine )
	{
		KisIteratorPixel quantumIt = lineIt.begin();
		KisIteratorPixel lastQuantum = lineIt.end();
		while( quantumIt <= lastQuantum )
		{
			for( int i = 0; i < depth; i++)
			{
			        newData[x*depth+i] = quantumIt.oldValue()[i];    
                        }
			++quantumIt;
		        ++x;
                }
		++lineIt;
        }
        
        //read the filter configuration values from the KisFilterConfiguration object
        
        Q_UINT32 brushSize = ((KisOilPaintFilterConfiguration*)configuration)->brushSize();
        Q_UINT32 smooth = ((KisOilPaintFilterConfiguration*)configuration)->smooth();
        
        kdDebug() << "brushSize:" << brushSize << " smooth:" << smooth << "\n";
        
        //the actual filter function from digikam. It needs a pointer to a QUANTUM array
        //with the actual pixel data.
        
        OilPaint(newData, width, height, brushSize, smooth, 0);
       
        x=0;
        
        //we set the iterator back to the first line
        
        lineIt = src->iteratorPixelSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
        
        // now we read the pixels from the QUANTUM array and use the iterators to write it back
        // to the actual image
        //
        // Fixme: this code uses the src and destination iterators. I think it would be enough
        // to iterate over the destination image only. I don't actually know if this speeds up
        // things, but it definitely should be tried
        
        while( lineIt <= lastLine )
	{
		KisIteratorPixel quantumIt = lineIt.begin();
		KisIteratorPixel dstQuantumIt = *dstLineIt;
		KisIteratorPixel lastQuantum = lineIt.end();
		while( quantumIt <= lastQuantum )
		{
			for( int i = 0; i < depth; i++)
			{
                                dstQuantumIt[i]=newData[x*depth+i];
			}
			++quantumIt;
			++dstQuantumIt;
		        ++x;
                }
		++lineIt;
		++dstLineIt;
	}
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the OilPaint effect.                
 *                                                                                    
 * data             => The image data in RGBA mode.                            
 * w                => Width of image.                          
 * h                => Height of image.                          
 * BrushSize        => Brush size.
 * Smoothness       => Smooth value.                                                
 *                                                                                  
 * Theory           => Using MostFrequentColor function we take the main color in  
 *                     a matrix and simply write at the original position.            
 */                                                                                 
    
void KisOilPaintFilter::OilPaint(QUANTUM* data, int w, int h, int BrushSize, int Smoothness, KisProgressDisplayInterface *m_progress)
{
        bool m_cancel=false; //to make it compile
    
        //Progress info
        //m_cancelRequested = false;
        //m_progress -> setSubject(this, true, true);
        //emit notifyProgressStage(this,i18n("Applying oilpaint filter..."),0);
    
        int LineWidth = w * 4;
        if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
      
        uchar* newBits = (uchar*)data;
        int i = 0;
        uint color;
    
        for (int h2 = 0; !m_cancel && (h2 < h); ++h2)
        {
                for (int w2 = 0; !m_cancel && (w2 < w); ++w2)
                {
                        i = h2 * LineWidth + 4*w2;
                        color = MostFrequentColor ((uchar*)data, w, h, w2, h2, BrushSize, Smoothness);
                
                        newBits[i+3] = qAlpha(color);
                        newBits[i+2] = qBlue(color);
                        newBits[i+1] = qGreen(color);
                        newBits[ i ] = qRed(color);
                }
       
       // Update de progress bar in dialog.
       //emit notifyProgress(this, (int) (((double)h2 * 100.0) / h));
       //m_progressBar->setValue((int) (((double)h2 * 100.0) / h));
       //kapp->processEvents();          
       }
//emit notifyProgressDone(this);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to determine the most frequent color in a matrix                        
 *                                                                                
 * Bits             => Bits array                                                    
 * Width            => Image width                                                   
 * Height           => Image height                                                 
 * X                => Position horizontal                                           
 * Y                => Position vertical                                            
 * Radius           => Is the radius of the matrix to be analized                  
 * Intensity        => Intensity to calcule                                         
 *                                                                                  
 * Theory           => This function creates a matrix with the analized pixel in   
 *                     the center of this matrix and find the most frequenty color   
 */
 
uint KisOilPaintFilter::MostFrequentColor (uchar* Bits, int Width, int Height, int X, 
                                              int Y, int Radius, int Intensity)
{
        int i, w, h, I;
        uint color;
        
        double Scale = Intensity / 255.0;
        int LineWidth = 4 * Width;
        
        if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);   // Don't take off this step
                
        // Alloc some arrays to be used
        uchar *IntensityCount = new uchar[(Intensity + 1) * sizeof (uchar)];
        uint  *AverageColorR  = new uint[(Intensity + 1)  * sizeof (uint)];
        uint  *AverageColorG  = new uint[(Intensity + 1)  * sizeof (uint)];
        uint  *AverageColorB  = new uint[(Intensity + 1)  * sizeof (uint)];
        
        // Erase the array
        memset(IntensityCount, 0, (Intensity + 1) * sizeof (uchar));

        /*for (i = 0; i <= Intensity; ++i)
                IntensityCount[i] = 0;*/
        
        for (w = X - Radius; w <= X + Radius; ++w)
                {
                for (h = Y - Radius; h <= Y + Radius; ++h)
                {
                // This condition helps to identify when a point doesn't exist
                
                if ((w >= 0) && (w < Width) && (h >= 0) && (h < Height))
                        {
                        // You'll see a lot of times this formula
                        i = h * LineWidth + 4 * w;
                        I = (uint)(GetIntensity (Bits[i], Bits[i+1], Bits[i+2]) * Scale);
                        IntensityCount[I]++;
        
                        if (IntensityCount[I] == 1)
                        {
                        AverageColorR[I] = Bits[ i ];
                        AverageColorG[I] = Bits[i+1];
                        AverageColorB[I] = Bits[i+2];
                        }
                        else
                        {
                        AverageColorR[I] += Bits[ i ];
                        AverageColorG[I] += Bits[i+1];
                        AverageColorB[I] += Bits[i+2];
                        }
                        }
                }
                }
        
        I = 0;
        int MaxInstance = 0;
        
        for (i = 0 ; i <= Intensity ; ++i)
        {
        if (IntensityCount[i] > MaxInstance)
                {
                I = i;
                MaxInstance = IntensityCount[i];
                }
        }
        
        int R, G, B;
        R = AverageColorR[I] / MaxInstance;
        G = AverageColorG[I] / MaxInstance;
        B = AverageColorB[I] / MaxInstance;
        color = qRgb (R, G, B);
        
        delete [] IntensityCount;        // free all the arrays
        delete [] AverageColorR;
        delete [] AverageColorG;
        delete [] AverageColorB;
        
        return (color);                    // return the most frequenty color
}


KisFilterConfigurationWidget* KisOilPaintFilter::createConfigurationWidget(QWidget* parent)
{
	KisOilPaintFilterConfigurationWidget* kopfcw = new KisOilPaintFilterConfigurationWidget(this,parent, "");
	kdDebug() << kopfcw << endl;
	return kopfcw  ;
}

KisFilterConfiguration* KisOilPaintFilter::configuration(KisFilterConfigurationWidget* nwidget)
{
	KisOilPaintFilterConfigurationWidget* widget = (KisOilPaintFilterConfigurationWidget*) nwidget;

	if( widget == 0 )
	{
		return new KisOilPaintFilterConfiguration(1,30);
	} else {
                Q_UINT32 brushSize = widget -> baseWidget() -> brushSizeSpinBox -> value();
                Q_UINT32 smooth = widget -> baseWidget() -> smoothSpinBox -> value();
                
                return new KisOilPaintFilterConfiguration(brushSize, smooth);
        }
}
