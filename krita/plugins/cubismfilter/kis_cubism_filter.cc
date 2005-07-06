/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from Gimp, Copyright (C) 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 * original pixelize.c for GIMP 0.54 by Tracy Scott
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
#include <time.h>
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
#include "kis_cubism_filter.h"

#define RANDOMNESS       5

KisCubismFilter::KisCubismFilter(KisView * view) : KisFilter(id(), view)
{
}

void KisCubismFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect)
{
        Q_INT32 x = rect.x(), y = rect.y();
        Q_INT32 width = rect.width();
        Q_INT32 height = rect.height();
        
        //read the filter configuration values from the KisFilterConfiguration object
        Q_UINT32 tileSize = ((KisCubismFilterConfiguration*)configuration)->tileSize();
        Q_UINT32 tileSaturation = ((KisCubismFilterConfiguration*)configuration)->tileSaturation();
        
        //pixelize(src, dst, x, y, width, height, pixelWidth, pixelHeight);
}

void KisCubismFilter::randomize_indices (Q_INT32 count, Q_INT32* indices)
{
        Q_INT32 index1, index2;
        Q_INT32 tmp;
        
        //initialize random number generator with time
        srand(static_cast<unsigned int>(time(0)));
        
        for (Q_INT32 i = 0; i < count * RANDOMNESS; i++)
        {
                index1 = randomIntRange(0, count);
                index2 = randomIntRange(0, count);
                tmp = indices[index1];
                indices[index1] = indices[index2];
                indices[index2] = tmp;
        }
}

Q_INT32 KisCubismFilter::randomIntRange(Q_INT32 lowestNumber, Q_INT32 highestNumber)
{
        if(lowestNumber > highestNumber)
        {
                Q_INT32 temp = lowestNumber;
                lowestNumber = highestNumber;
                highestNumber = temp;
        }

        Q_INT32 range = highestNumber - lowestNumber + 1;
        return lowestNumber + static_cast<Q_INT32>(range *rand()/(RAND_MAX+1.0));
}

QWidget* KisCubismFilter::createConfigurationWidget(QWidget* parent)
{
	vKisIntegerWidgetParam param;
	param.push_back( KisIntegerWidgetParam( 2, 40, 10, i18n("Tile size") ) );
	param.push_back( KisIntegerWidgetParam( 2, 40, 10, i18n("Tile saturation") ) );
	return new KisMultiIntegerFilterWidget(this, parent, id().id().ascii(), id().id().ascii(), param );
}

KisFilterConfiguration* KisCubismFilter::configuration(QWidget* nwidget)
{
	KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
	if( widget == 0 )
	{
		return new KisCubismFilterConfiguration( 10, 10);
	} else {
		return new KisCubismFilterConfiguration( widget->valueAt( 0 ), widget->valueAt( 1 ) );
	}
}
