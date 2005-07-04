/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from digikam, Copyright 2004 by Gilles Caulier,
 * Original Oilpaint algorithm copyrighted 2004 by
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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
#include <kis_types.h>
#include <kis_view.h>
#include <kis_progress_display_interface.h>

#include "kis_multi_integer_filter_widget.h"
#include "kis_pixelize_filter.h"

KisPixelizeFilter::KisPixelizeFilter(KisView * view) : KisFilter(id(), view)
{
}

void KisPixelizeFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect)
{

	Q_UNUSED(dst);

	Q_INT32 x = rect.x(), y = rect.y();
	Q_INT32 width = rect.width();
	Q_INT32 height = rect.height();

	//read the filter configuration values from the KisFilterConfiguration object
	Q_UINT32 pixelWidth = ((KisPixelizeFilterConfiguration*)configuration)->pixelWidth();
	Q_UINT32 pixelHeight = ((KisPixelizeFilterConfiguration*)configuration)->pixelHeight();

	kdDebug() << "pixelwidth: " << pixelWidth << endl;
	kdDebug() << "pixelheight: " << pixelHeight << endl;
	pixelize(src, x, y, width, height, pixelWidth, pixelHeight);
}

void KisPixelizeFilter::pixelize(KisPaintDeviceSP src, int startx, int starty, int width, int height, int pixelWidth, int pixelHeight)
{
        Q_INT32 pixelSize = src -> pixelSize();
        Q_INT32 average[ pixelSize ];
        Q_UINT8* bufRow; 
        Q_UINT8* buf;
        Q_INT32 count;                
        bool hasAlpha = src -> alpha();
        Q_INT32 rowstride; 
        
        setProgressTotalSteps(height);
        setProgressStage(i18n("Applying pixelize filter..."),0);

        for (Q_INT32 y = starty; y < starty + height; y += pixelHeight - (y % pixelHeight))
        {
                Q_INT32 h = pixelHeight - (y % pixelHeight);
                //h = MIN (h, starty + height - y);

                for (Q_INT32 x = startx; x < startx + width; x += pixelWidth - (x % pixelWidth))
                {
                        Q_INT32 w = pixelWidth - (x % pixelWidth);
                        //w = MIN (w, startx + width - x);
                        
                        for (Q_INT32 i = 0; i < pixelSize; i++)
                        {
                                average[i] = 0;
                        }
                        count = 0;

                        /* Read */
                        //bufRow = area.data + (y-area.y)*rowstride + (x-area.x)*bpp;

                        for (Q_INT32 row = 0; row < h; row++)
                        {
                                buf = bufRow;
                                if (hasAlpha)
                                {
                                        for (Q_INT32 col = 0; col < w; col++)
                                        {
                                                Q_INT32 alpha = buf[pixelSize-1];
        
                                                average[pixelSize-1] += alpha;
                                                for (Q_INT32 i = 0; i < pixelSize-1; i++)
                                                {        
                                                        average[i] += buf[i] * alpha;
                                                }
                                                buf += pixelSize;
                                        }
                                }
                                else
                                {
                                        for (Q_INT32 col = 0; col < w; col++)
                                        {
                                                for (Q_INT32 i = 0; i < pixelSize; i++)
                                                {        
                                                        average[i] += buf[i];
                                                }
                                                buf += pixelSize;
                                        }
                                }
                                bufRow += rowstride;
                        }

                        count += w*h;

                        /* Average */
                        if (count > 0)
                        {
                                if (hasAlpha)
                                        {
                                        Q_INT32 alpha = average[pixelSize-1];
                        
                                        if ((average[pixelSize-1] = alpha / count))
                                        {
                                        for (Q_INT32 i = 0; i < pixelSize-1; i++)
                                                average[i] /= alpha;
                                        }
                                }
                                else
                                {
                                        for (Q_INT32 i = 0; i < pixelSize; i++)
                                        average[i] /= count;
                                }
                        }

                        /* Write */
                        //bufRow = area.data + (y-area.y)*rowstride + (x-area.x)*bpp;
                
                        for (Q_INT32 row = 0; row < h; row++)
                        {
                                buf = bufRow;
                                for (Q_INT32 col = 0; col < w; col++)
                                        {
                                        for (Q_INT32 i = 0; i < pixelSize; i++)
                                        buf[i] = average[i];
                        
                                        count++;
                                        buf += pixelSize;
                                        }
                                bufRow += rowstride;
                        }
                }
        }
		
	setProgressDone();
}

QWidget* KisPixelizeFilter::createConfigurationWidget(QWidget* parent)
{
	vKisIntegerWidgetParam param;
	param.push_back( KisIntegerWidgetParam( 2, 40, 10, i18n("Pixelwidth") ) );
	param.push_back( KisIntegerWidgetParam( 2, 40, 10, i18n("Pixelheight") ) );
	return new KisMultiIntegerFilterWidget(this, parent, id().id().ascii(), id().id().ascii(), param );
}

KisFilterConfiguration* KisPixelizeFilter::configuration(QWidget* nwidget)
{
	KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
	if( widget == 0 )
	{
		return new KisPixelizeFilterConfiguration( 10, 10);
	} else {
		return new KisPixelizeFilterConfiguration( widget->valueAt( 0 ), widget->valueAt( 1 ) );
	}
}
