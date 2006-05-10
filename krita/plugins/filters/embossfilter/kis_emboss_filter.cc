/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from digikam, Copyright 2004 by Gilles Caulier,
 * Original Emboss algorithm copyrighted 2004 by
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>

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

#include "kis_multi_integer_filter_widget.h"
#include "kis_emboss_filter.h"

KisEmbossFilter::KisEmbossFilter() : KisFilter(id(), "emboss", i18n("&Emboss with Variable Depth..."))
{
}

void KisEmbossFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect)
{
    Q_UNUSED(dst);


    //read the filter configuration values from the KisFilterConfiguration object
    quint32 embossdepth = ((KisEmbossFilterConfiguration*)configuration)->depth();

    //the actual filter function from digikam. It needs a pointer to a quint8 array
    //with the actual pixel data.

    Emboss(src, dst, rect, embossdepth);
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

void KisEmbossFilter::Emboss(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect& rect, int d)
{
    float Depth = d / 10.0;
    int    R = 0, G = 0, B = 0;
    uchar  Gray = 0;
    int Width = rect.width();
    int Height = rect.height();

    setProgressTotalSteps(Height);
    setProgressStage(i18n("Applying emboss filter..."),0);

        for (int y = 0 ; !cancelRequested() && (y < Height) ; ++y)
        {
        KisHLineIteratorPixel it = src->createHLineIterator(rect.x(), rect.y() + y, rect.width(), false);
        KisHLineIteratorPixel dstIt = dst->createHLineIterator(rect.x(), rect.y() + y, rect.width(), true);

        for (int x = 0 ; !cancelRequested() && (x < Width) ; ++x, ++it, ++dstIt)
        {
            if (it.isSelected()) {

// XXX: COLORSPACE_INDEPENDENCE

                QColor color1;
                src->colorSpace()->toQColor(it.rawData(), &color1);

                QColor color2;
                quint8 opacity2;
                src->pixel(rect.x() + x + Lim_Max(x, 1, Width), rect.y() + y + Lim_Max(y, 1, Height), &color2, &opacity2);

                R = abs((int)((color1.red() - color2.red()) * Depth + (quint8_MAX / 2)));
                G = abs((int)((color1.green() - color2.green()) * Depth + (quint8_MAX / 2)));
                B = abs((int)((color1.blue() - color2.blue()) * Depth + (quint8_MAX / 2)));

                Gray = CLAMP((R + G + B) / 3, 0, quint8_MAX);

                dst->colorSpace()->fromQColor(QColor(Gray, Gray, Gray), dstIt.rawData());
            }
        }

        setProgress(y);
        }

    setProgressDone();
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

KisFilterConfigWidget * KisEmbossFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP)
{
    vKisIntegerWidgetParam param;
    param.push_back( KisIntegerWidgetParam( 10, 300, 30, i18n("Depth"), "depth" ) );
    KisFilterConfigWidget * w = new KisMultiIntegerFilterWidget(parent, id().id().toAscii(), id().id(), param );
    Q_CHECK_PTR(w);
    return w;
}

KisFilterConfiguration* KisEmbossFilter::configuration(QWidget* nwidget)
{
    KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
    if( widget == 0 )
    {
        return new KisEmbossFilterConfiguration( 30 );
    } else {
        return new KisEmbossFilterConfiguration( widget->valueAt( 0 ) );
    }
}
