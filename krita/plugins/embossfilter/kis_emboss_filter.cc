/* This file is part of the KDE project
   Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>

   ported from digikam, Copyright 2004 by Gilles Caulier,
   Original Emboss algorithm copyrighted 2004 by 
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

#include "kis_filter_configuration_widget.h"
#include "kis_emboss_filter_configuration_widget.h"
#include "kis_emboss_filter_configuration_base_widget.h"
#include "kis_emboss_filter.h"

KisEmbossFilter::KisEmbossFilter(KisView * view) : KisFilter(name(), view)
{
}

void KisEmbossFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect, KisTileCommand* ktc)
{
	kdDebug() << "Embossfilter called!\n";

#if 0 // AUTO_LAYERS 
        Q_INT32 x, y, width, height;
	src.extent(x, y, width, height);
        
        // create a QUANTUM array that holds the data the filter works on
        
        QUANTUM * newData = new QUANTUM[width * height * src -> depth() * sizeof(QUANTUM)];
        
        // create iterators for the src and the dst image
        
        KisIteratorLinePixel lineIt = src->iteratorPixelSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
	KisIteratorLinePixel dstLineIt = dst->iteratorPixelSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
	KisIteratorLinePixel lastLine = src->iteratorPixelSelectionEnd(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);
	KisIteratorLinePixel dstLastLine = src->iteratorPixelSelectionEnd(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);
        
        Q_UINT32 depth = src->depth();
        
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
        
        Q_UINT32 embossdepth = ((KisEmbossFilterConfiguration*)configuration)->depth();

        kdDebug() << "depth:" << depth << "\n";
        
        //the actual filter function from digikam. It needs a pointer to a QUANTUM array
        //with the actual pixel data.
        
        Emboss(newData, width, height, embossdepth);
       
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
#endif // AUTO_LAYERS
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the Emboss effect                                             
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                          
 * d                => Emboss value                                                  
 *                                                                                
 * Theory           => This is an amazing effect. And the theory is very simple to 
 *                     understand. You get the diference between the colors and    
 *                     increase it. After this, get the gray tone            
 */

void KisEmbossFilter::Emboss(QUANTUM* data, int Width, int Height, int d)
{
    float Depth = d / 10.0;
    int LineWidth = Width * 4;
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    uchar *Bits = (uchar*) data;
    int    i = 0, j = 0;
    int    R = 0, G = 0, B = 0;
    uchar  Gray = 0;
    
    bool m_cancel = false; //make it compile...
    
    for (int h = 0 ; !m_cancel && (h < Height) ; ++h)
       {
       for (int w = 0 ; !m_cancel && (w < Width) ; ++w)
           {
           i = h * LineWidth + 4 * w;
           j = (h + Lim_Max (h, 1, Height)) * LineWidth + 4 * (w + Lim_Max (w, 1, Width));
               
           R = abs ((int)((Bits[i+2] - Bits[j+2]) * Depth + 128));
           G = abs ((int)((Bits[i+1] - Bits[j+1]) * Depth + 128));
           B = abs ((int)((Bits[ i ] - Bits[ j ]) * Depth + 128));

           Gray = LimitValues ((R + G + B) / 3);
           
           Bits[i+2] = Gray;
           Bits[i+1] = Gray;
           Bits[ i ] = Gray;
           }
       
       // Update de progress bar in dialog.
       //m_progressBar->setValue((int) (((double)h * 100.0) / Height));
       //kapp->processEvents(); 
       }
}
       
// This method have been ported from Pieter Z. Voloshyn algorithm code.   
    
/* This function limits the max and min values     
 * defined by the developer                                    
 *                                                                              
 * Now               => Original value                                      
 * Up                => Increments                                              
 * Max               => Maximum value                                          
 *                                                                                  
 * Theory            => This function is used in some functions to limit the        
 *                      "for step". E.g. I have a picture with 309 pixels (width), and  
 *                      my "for step" is 5. All the code go alright until reachs the  
 *                      w = 305, because in the next step w will go to 310, but we want  
 *                      to analize all the pixels. So, this function will reduce the 
 *                      "for step", when necessary, until reach the last possible value  
 */
 
int KisEmbossFilter::Lim_Max (int Now, int Up, int Max)
{
    --Max;
    while (Now > Max - Up)
        --Up;
    return (Up);
}    

// This method have been ported from Pieter Z. Voloshyn algorithm code.  
 
/* This function limits the RGB values                        
 *                                                                         
 * ColorValue        => Here, is an RGB value to be analized                   
 *                                                                             
 * Theory            => A color is represented in RGB value (e.g. 0xFFFFFF is     
 *                      white color). But R, G and B values has 256 values to be used   
 *                      so, this function analize the value and limits to this range   
 */   
                     
uchar KisEmbossFilter::LimitValues (int ColorValue)
{
    if (ColorValue > 255)        // MAX = 255
        ColorValue = 255;        
    if (ColorValue < 0)          // MIN = 0
        ColorValue = 0;
    return ((uchar) ColorValue);
}

KisFilterConfigurationWidget* KisEmbossFilter::createConfigurationWidget(QWidget* parent)
{
	KisEmbossFilterConfigurationWidget* kefcw = new KisEmbossFilterConfigurationWidget(this,parent, "");
	kdDebug() << kefcw << endl;
	return kefcw  ;
}

KisFilterConfiguration* KisEmbossFilter::configuration(KisFilterConfigurationWidget* nwidget)
{
	KisEmbossFilterConfigurationWidget* widget = (KisEmbossFilterConfigurationWidget*) nwidget;

	if( widget == 0 )
	{
		return new KisEmbossFilterConfiguration(30);
	} else {
                Q_UINT32 depth = widget -> baseWidget() -> depthSpinBox -> value();
                
                return new KisEmbossFilterConfiguration(depth);
        }
}
