/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from digikam, copyrighted 2004 by Gilles Caulier,
 * Original RainDrops algorithm copyrighted 2004 by
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

#include "kis_raindrops_filter.h"

#include <stdlib.h>
#include <vector>
#include <math.h>

#include <QDateTime>
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
#include <KoUpdater.h>

#include <kis_iterators_pixel.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter.h>
#include <kis_global.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <kis_paint_device.h>
#include <KoProgressUpdater.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "widgets/kis_multi_integer_filter_widget.h"


KisRainDropsFilter::KisRainDropsFilter() : KisFilter(id(), KisFilter::categoryArtistic(), i18n("&Raindrops..."))
{
    setSupportsPainting(false);
    setSupportsThreading(false);
    setSupportsPreview(true);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the RainDrops effect (inspired from Jason Waltman code)
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * DropSize         => Raindrop size
 * number           => Maximum number of raindrops
 * fishEyes            => FishEye coefficient
 *
 * Theory           => This functions does several math's functions and the engine
 *                     is simple to undestand, but a little hard to implement. A
 *                     control will indicate if there is or not a raindrop in that
 *                     area, if not, a fisheye effect with a random size (max=DropSize)
 *                     will be applied, after this, a shadow will be applied too.
 *                     and after this, a blur function will finish the effect.
 */


void KisRainDropsFilter::process(KisConstProcessingInformation srcInfo,
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
    Q_UNUSED(config);
    Q_ASSERT(!src.isNull());
    Q_ASSERT(!dst.isNull());

    //read the filter configuration values from the KisFilterConfiguration object
    quint32 DropSize = config->getInt("dropSize", 80);
    quint32 number = config->getInt("number", 80);
    quint32 fishEyes = config->getInt("fishEyes", 30);

    if (progressUpdater) {
        progressUpdater->setRange(0, size.width() * size.height());
    }
    int count = 0;

    if (fishEyes <= 0) fishEyes = 1;

    if (fishEyes > 100) fishEyes = 100;

    int Width = size.width();
    int Height = size.height();

    bool** BoolMatrix = CreateBoolArray(Width, Height);

    int       i, j, k, l, m, n;                 // loop variables
    int       Bright;                           // Bright value for shadows and highlights
    int       x, y;                             // center coordinates
    int       Counter = 0;                      // Counter (duh !)
    int       NewSize;                          // Size of current raindrop
    int       halfSize;                         // Half of the current raindrop
    int       Radius;                           // Maximum radius for raindrop
    int       BlurRadius;                       // Blur Radius
    int       BlurPixels;

    double    r, a;                             // polar coordinates
    double    OldRadius;                        // Radius before processing
    double    NewfishEyes = (double)fishEyes * 0.01;  // FishEye fishEyesicients
    double    s;
    double    R, G, B;

    bool      FindAnother = false;              // To search for good coordinates

    const KoColorSpace * cs = src->colorSpace();


    // XXX: move the seed to the config, so the filter can be used as
    // and adjustment filter (boud).
    // And use a thread-safe random number generator
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000(QDate(2000, 1, 1), QTime(0, 0, 0));

    srand((uint) dt.secsTo(Y2000));

    // Init booleen Matrix.

    for (i = 0 ; (i < Width) && !(progressUpdater && progressUpdater->interrupted()) ; ++i) {
        for (j = 0 ; (j < Height) && !(progressUpdater && progressUpdater->interrupted()); ++j) {
            BoolMatrix[i][j] = false;
        }
    }

    for (uint NumBlurs = 0; (NumBlurs <= number) && !(progressUpdater && progressUpdater->interrupted()); ++NumBlurs) {
        NewSize = (int)(rand() * ((double)(DropSize - 5) / RAND_MAX) + 5);
        halfSize = NewSize / 2;
        Radius = halfSize;
        s = Radius / log(NewfishEyes * Radius + 1);

        Counter = 0;

        do {
            FindAnother = false;
            y = (int)(rand() * ((double)(Width - 1) / RAND_MAX));
            x = (int)(rand() * ((double)(Height - 1) / RAND_MAX));

            if (BoolMatrix[y][x])
                FindAnother = true;
            else
                for (i = x - halfSize ; (i <= x + halfSize) && !(progressUpdater && progressUpdater->interrupted()); i++)
                    for (j = y - halfSize ; (j <= y + halfSize) && !(progressUpdater && progressUpdater->interrupted()); j++)
                        if ((i >= 0) && (i < Height) && (j >= 0) && (j < Width))
                            if (BoolMatrix[j][i])
                                FindAnother = true;

            Counter++;
        } while ((FindAnother && (Counter < 10000) && !(progressUpdater && progressUpdater->interrupted())));

        if (Counter >= 10000) {
            NumBlurs = number;
            break;
        }

        for (i = -1 * halfSize ; (i < NewSize - halfSize) && !(progressUpdater && progressUpdater->interrupted()); i++) {
            for (j = -1 * halfSize ; (j < NewSize - halfSize) && !(progressUpdater && progressUpdater->interrupted()); j++) {
                r = sqrt(i * i + j * j);
                a = atan2(static_cast<double>(i), static_cast<double>(j));

                if (r <= Radius) {
                    OldRadius = r;
                    r = (exp(r / s) - 1) / NewfishEyes;

                    k = x + (int)(r * sin(a));
                    l = y + (int)(r * cos(a));

                    m = x + i;
                    n = y + j;

                    if ((k >= 0) && (k < Height) && (l >= 0) && (l < Width)) {
                        if ((m >= 0) && (m < Height) && (n >= 0) && (n < Width)) {
                            Bright = 0;

                            if (OldRadius >= 0.9 * Radius) {
                                if ((a <= 0) && (a > -2.25))
                                    Bright = -80;
                                else if ((a <= -2.25) && (a > -2.5))
                                    Bright = -40;
                                else if ((a <= 0.25) && (a > 0))
                                    Bright = -40;
                            }

                            else if (OldRadius >= 0.8 * Radius) {
                                if ((a <= -0.75) && (a > -1.50))
                                    Bright = -40;
                                else if ((a <= 0.10) && (a > -0.75))
                                    Bright = -30;
                                else if ((a <= -1.50) && (a > -2.35))
                                    Bright = -30;
                            }

                            else if (OldRadius >= 0.7 * Radius) {
                                if ((a <= -0.10) && (a > -2.0))
                                    Bright = -20;
                                else if ((a <= 2.50) && (a > 1.90))
                                    Bright = 60;
                            }

                            else if (OldRadius >= 0.6 * Radius) {
                                if ((a <= -0.50) && (a > -1.75))
                                    Bright = -20;
                                else if ((a <= 0) && (a > -0.25))
                                    Bright = 20;
                                else if ((a <= -2.0) && (a > -2.25))
                                    Bright = 20;
                            }

                            else if (OldRadius >= 0.5 * Radius) {
                                if ((a <= -0.25) && (a > -0.50))
                                    Bright = 30;
                                else if ((a <= -1.75) && (a > -2.0))
                                    Bright = 30;
                            }

                            else if (OldRadius >= 0.4 * Radius) {
                                if ((a <= -0.5) && (a > -1.75))
                                    Bright = 40;
                            }

                            else if (OldRadius >= 0.3 * Radius) {
                                if ((a <= 0) && (a > -2.25))
                                    Bright = 30;
                            }

                            else if (OldRadius >= 0.2 * Radius) {
                                if ((a <= -0.5) && (a > -1.75))
                                    Bright = 20;
                            }

                            BoolMatrix[n][m] = true;

                            QColor originalColor;

                            KisHLineConstIterator oldIt = src->createHLineConstIterator(srcTopLeft.x() + l, srcTopLeft.y() + k, 1, srcInfo.selection());
                            cs->toQColor(oldIt.oldRawData(), &originalColor);

                            int newRed = CLAMP(originalColor.red() + Bright, 0, quint8_MAX);
                            int newGreen = CLAMP(originalColor.green() + Bright, 0, quint8_MAX);
                            int newBlue = CLAMP(originalColor.blue() + Bright, 0, quint8_MAX);

                            QColor newColor;
                            newColor.setRgb(newRed, newGreen, newBlue);

                            KisHLineIterator dstIt = dst->createHLineIterator(dstTopLeft.x() + n, dstTopLeft.y() + m, 1, dstInfo.selection());
                            cs->fromQColor(newColor, dstIt.rawData());
                        }
                    }
                }
            }
        }

        BlurRadius = NewSize / 25 + 1;

        for (i = -1 * halfSize - BlurRadius ; (i < NewSize - halfSize + BlurRadius) && !(progressUpdater && progressUpdater->interrupted()) ; i++) {
            for (j = -1 * halfSize - BlurRadius;
                    ((j < NewSize - halfSize + BlurRadius) && !(progressUpdater && progressUpdater->interrupted()));
                    ++j) {
                r = sqrt(i * i + j * j);

                if (r <= Radius * 1.1) {
                    R = G = B = 0;
                    BlurPixels = 0;

                    for (k = -1 * BlurRadius; k < BlurRadius + 1; k++)
                        for (l = -1 * BlurRadius; l < BlurRadius + 1; l++) {
                            m = x + i + k;
                            n = y + j + l;

                            if ((m >= 0) && (m < Height) && (n >= 0) && (n < Width)) {
                                QColor color;
                                KisHLineConstIterator dstIt = dst->createHLineConstIterator(dstTopLeft.x() + n, dstTopLeft.y() + m, 1);
                                cs->toQColor(dstIt.rawData(), &color);

                                R += color.red();
                                G += color.green();
                                B += color.blue();
                                BlurPixels++;
                            }
                        }

                    m = x + i;
                    n = y + j;

                    if ((m >= 0) && (m < Height) && (n >= 0) && (n < Width)) {
                        QColor color;

                        color.setRgb((int)(R / BlurPixels), (int)(G / BlurPixels), (int)(B / BlurPixels));
                        KisHLineIterator dstIt = dst->createHLineIterator(dstTopLeft.x() + n, dstTopLeft.y() + m, 1);
                        cs->fromQColor(color, dstIt.rawData());
                    }
                }
            }
        }

        if (progressUpdater) progressUpdater->setValue(++count);
    }

    FreeBoolArray(BoolMatrix, Width);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to free a dinamic boolean array
 *
 * lpbArray          => Dynamic boolean array
 * Columns           => The array bidimension value
 *
 * Theory            => An easy to undestand 'for' statement
 */
void KisRainDropsFilter::FreeBoolArray(bool** lpbArray, uint Columns) const
{
    for (uint i = 0; i < Columns; ++i)
        free(lpbArray[i]);

    free(lpbArray);
}

/* Function to create a bidimentional dinamic boolean array
 *
 * Columns           => Number of columns
 * Rows              => Number of rows
 *
 * Theory            => Using 'for' statement, we can alloc multiple dinamic arrays
 *                      To create more dimentions, just add some 'for's, ok?
 */
bool** KisRainDropsFilter::CreateBoolArray(uint Columns, uint Rows) const
{
    bool** lpbArray = NULL;
    lpbArray = (bool**) malloc(Columns * sizeof(bool*));

    if (lpbArray == NULL)
        return (NULL);

    for (uint i = 0; i < Columns; ++i) {
        lpbArray[i] = (bool*) malloc(Rows * sizeof(bool));
        if (lpbArray[i] == NULL) {
            FreeBoolArray(lpbArray, Columns);
            return (NULL);
        }
    }

    return (lpbArray);
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

uchar KisRainDropsFilter::LimitValues(int ColorValue) const
{
    if (ColorValue > 255)        // MAX = 255
        ColorValue = 255;
    if (ColorValue < 0)          // MIN = 0
        ColorValue = 0;
    return ((uchar) ColorValue);
}

KisConfigWidget * KisRainDropsFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageWSP) const
{
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(1, 200, 80, i18n("Drop size"), "dropsize"));
    param.push_back(KisIntegerWidgetParam(1, 500, 80, i18n("Number"), "number"));
    param.push_back(KisIntegerWidgetParam(1, 100, 30, i18n("Fish eyes"), "fishEyes"));
    return new KisMultiIntegerFilterWidget(id().id(), parent, id().id(), param);
}

KisFilterConfiguration* KisRainDropsFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("raindrops", 1);
    config->setProperty("dropsize", 80);
    config->setProperty("number", 80);
    config->setProperty("fishEyes", 30);
    return config;
}
