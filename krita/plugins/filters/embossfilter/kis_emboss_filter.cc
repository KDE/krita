/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from digikam, Copyrighted 2004 Gilles Caulier,
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

#include "kis_emboss_filter.h"


#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include "KoIntegerMaths.h"
#include "KoProgressUpdater.h"
#include <KoUpdater.h>

#include <kis_iterators_pixel.h>
#include <kis_random_accessor.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include "widgets/kis_multi_integer_filter_widget.h"

KisEmbossFilter::KisEmbossFilter() : KisFilter(id(), categoryEmboss(), i18n("&Emboss with Variable Depth..."))
{
    setSupportsPainting(false);
    setSupportsPreview(true);
    setColorSpaceIndependence(TO_RGBA8);
}

KisFilterConfiguration* KisEmbossFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 0);
    config->setProperty("depth", 30);
    return config;
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
void KisEmbossFilter::process(KisConstProcessingInformation srcInfo,
                              KisProcessingInformation dstInfo,
                              const QSize& size,
                              const KisFilterConfiguration* config,
                              KoUpdater* progressUpdater
                             ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    //read the filter configuration values from the KisFilterConfiguration object
    quint32 embossdepth = config ?  config->getInt("depth", 30) : 30;

    //the actual filter function from digikam. It needs a pointer to a quint8 array
    //with the actual pixel data.

    float Depth = embossdepth / 10.0;
    int    R = 0, G = 0, B = 0;
    uchar  Gray = 0;
    int Width = size.width();
    int Height = size.height();

    if (progressUpdater) {
        progressUpdater->setRange(0, Height);
    }

    KisHLineConstIteratorPixel it = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), srcInfo.selection());
    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), dstInfo.selection());
    QColor color1;
    QColor color2;
    for (int y = 0; !(progressUpdater && progressUpdater->interrupted()) && (y < Height); ++y) {
        KisRandomConstAccessorPixel acc = src->createRandomConstAccessor(srcTopLeft.x(), srcTopLeft.y());

        for (int x = 0; !(progressUpdater && progressUpdater->interrupted()) && (x < Width); ++x, ++it, ++dstIt) {
            if (dstIt.isSelected()) {

// XXX: COLORSPACE_INDEPENDENCE or at least work IN RGB16A


                src->colorSpace()->toQColor(it.oldRawData(), &color1);
                acc.moveTo(srcTopLeft.x() + x + Lim_Max(x, 1, Width), srcTopLeft.y() + y + Lim_Max(y, 1, Height));

                src->colorSpace()->toQColor(acc.oldRawData(), &color2);

                R = abs((int)((color1.red() - color2.red()) * Depth + (quint8_MAX / 2)));
                G = abs((int)((color1.green() - color2.green()) * Depth + (quint8_MAX / 2)));
                B = abs((int)((color1.blue() - color2.blue()) * Depth + (quint8_MAX / 2)));

                Gray = CLAMP((R + G + B) / 3, 0, quint8_MAX);

                dst->colorSpace()->fromQColor(QColor(Gray, Gray, Gray, color1.alpha()), dstIt.rawData());
            }
        }
        it.nextRow();
        dstIt.nextRow();
        if (progressUpdater) progressUpdater->setValue(y);
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

int KisEmbossFilter::Lim_Max(int Now, int Up, int Max) const
{
    --Max;
    while (Now > Max - Up)
        --Up;
    return (Up);
}

KisConfigWidget * KisEmbossFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageSP image) const
{
    Q_UNUSED(dev);
    Q_UNUSED(image);

    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(10, 300, 30, i18n("Depth"), "depth"));
    KisConfigWidget * w = new KisMultiIntegerFilterWidget(id().id(), parent, id().id(), param);
    Q_CHECK_PTR(w);
    return w;
}
