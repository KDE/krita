/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * The gaussian blur algoithm is ported from gimo
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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
#include <limits.h>

#include <stdlib.h>
#include <vector>

#include <QColor>

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
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include "kis_meta_registry.h"
#include <kis_transaction.h>
#include <kis_undo_adapter.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_progress_subject.h>
#include <kis_progress_display_interface.h>
#include <kis_colorspace.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_view.h>
#include <kis_paint_device.h>
#include <kis_channelinfo.h>
#include <kis_convolution_painter.h>

#include "kis_dropshadow.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

KisDropshadow::KisDropshadow(KisView * view)
    : m_view(view)
{
}

void KisDropshadow::dropshadow(KisProgressDisplayInterface * progress, qint32 xoffset, qint32 yoffset, qint32 blurradius, QColor color, quint8 opacity, bool allowResize)
{
    KisImageSP image = m_view->canvasSubject()->currentImg();
    if (!image) return;

    KisLayerSP src = image->activeLayer();
    if (!src) return;

    KisPaintDeviceSP dev = image->activeDevice();
    if (!dev) return;

    m_cancelRequested = false;
    if ( progress )
        progress->setSubject(this, true, true);
    emit notifyProgressStage(i18n("Add drop shadow..."), 0);

    if (image->undo()) {
        image->undoAdapter()->beginMacro(i18n("Add Drop Shadow"));
    }

    KisPaintDeviceSP shadowDev = KisPaintDeviceSP(new KisPaintDevice( KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA",""),"" ), "Shadow"));
    KisPaintDeviceSP bShadowDev;
    KisColorSpace *rgb8cs = shadowDev->colorSpace();

    QRect rect = dev->exactBounds();

    for (qint32 row = 0; row < rect.height(); ++row)
    {
        KisHLineIteratorPixel srcIt = dev->createHLineIterator(rect.x(), rect.y() + row, rect.width(), false);
        KisHLineIteratorPixel dstIt = shadowDev->createHLineIterator(rect.x(), rect.y() + row, rect.width(), true);
        while( ! srcIt.isDone() )
        {
            if (srcIt.isSelected())
            {
                //set the shadow color
                quint8 alpha = dev->colorSpace()->getAlpha(srcIt.rawData());
                rgb8cs->fromQColor(color, alpha, dstIt.rawData());
            }
            ++srcIt;
            ++dstIt;
        }
        emit notifyProgress((row * 100) / rect.height() );
    }

    if( blurradius > 0 )
    {
        bShadowDev = new KisPaintDevice( KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA",""),"" ), "bShadow");
        gaussianblur(shadowDev, bShadowDev, rect, blurradius, blurradius, BLUR_RLE, progress);
        shadowDev = bShadowDev;
    }

    if (!m_cancelRequested) {
        shadowDev->move (xoffset,yoffset);

        KisGroupLayerSP parent = image->rootLayer();
        if (image->activeLayer())
            parent = image->activeLayer()->parent().data();

        KisPaintLayerSP l = KisPaintLayerSP(new KisPaintLayer(image.data(), i18n("Drop Shadow"), opacity, shadowDev));
        image->addLayer( KisLayerSP(l.data()), parent, src->siblingBelow() );

        if (allowResize)
        {
            QRect shadowBounds = shadowDev->exactBounds();

            if (!image->bounds().contains(shadowBounds)) {

                QRect newImageSize = image->bounds() | shadowBounds;
                image->resize(newImageSize.width(), newImageSize.height());

                if (shadowBounds.left() < 0 || shadowBounds.top() < 0) {

                    qint32 newRootX = image->rootLayer()->x();
                    qint32 newRootY = image->rootLayer()->y();

                    if (shadowBounds.left() < 0) {
                        newRootX += -shadowBounds.left();
                    }
                    if (shadowBounds.top() < 0) {
                        newRootY += -shadowBounds.top();
                    }

                    KCommand *moveCommand = image->rootLayer()->moveCommand(QPoint(image->rootLayer()->x(), image->rootLayer()->y()),
                                                                            QPoint(newRootX, newRootY));
                    Q_ASSERT(moveCommand != 0);

                    if (moveCommand) {
                        moveCommand->execute();
                        if (image->undo()) {
                            image->undoAdapter()->addCommand(moveCommand);
                        } else {
                            delete moveCommand;
                        }
                    }
                }
            }
        }
        m_view->canvasSubject()->document()->setModified(true);
    }

    if (image->undo()) {
        image->undoAdapter()->endMacro();
    }

    emit notifyProgressDone();
}

void KisDropshadow::gaussianblur (KisPaintDeviceSP srcDev, KisPaintDeviceSP dstDev, QRect& rect, double horz, double vert, BlurMethod method, KisProgressDisplayInterface *)
{
    qint32 width, height;
    qint32 bytes;
    quint8 *dest, *dp;
    quint8 *src, *sp, *sp_p, *sp_m;
    qint32 *buf = NULL;
    qint32 *bb;
    double n_p[5], n_m[5];
    double d_p[5], d_m[5];
    double bd_p[5], bd_m[5];
    double *val_p = NULL;
    double *val_m = NULL;
    double *vp, *vm;
    qint32 x1, y1, x2, y2;
    qint32 i, j;
    qint32 row, col, b;
    qint32 terms;
    double progress, max_progress;
    qint32 initial_p[4];
    qint32 initial_m[4];
    double std_dev;
    qint32 pixels;
    qint32 total = 1;
    qint32 start, end;
    qint32 *curve;
    qint32 *sum = NULL;
    qint32 val;
    qint32 length;
    qint32 initial_pp, initial_mm;

    x1 = (qint32)(rect.x() - horz);
    y1 = (qint32)(rect.y() - vert);
    width = (qint32)(rect.width() + 2 * horz);
    height = (qint32)(rect.height() + 2 * vert);
    x2 = x1 + width;
    y2 = y1 + height;

    if (width < 1 || height < 1) return;

    emit notifyProgressStage(i18n("Blur..."), 0);

    bytes = srcDev->pixelSize();

    switch (method)
    {
    case BLUR_IIR:
        val_p = new double[MAX (width, height) * bytes];
        val_m = new double[MAX (width, height) * bytes];
        break;

    case BLUR_RLE:
        buf = new qint32[MAX (width, height) * 2];
        break;
    }

    src =  new quint8[MAX (width, height) * bytes];
    dest = new quint8[MAX (width, height) * bytes];

    progress = 0.0;
    max_progress  = (horz <= 0.0 ) ? 0 : width * height * horz;
    max_progress += (vert <= 0.0 ) ? 0 : width * height * vert;


    /*  First the vertical pass  */
    if (vert > 0.0)
    {
        vert = fabs (vert) + 1.0;
        std_dev = sqrt (-(vert * vert) / (2 * log (1.0 / 255.0)));

        switch (method)
        {
        case BLUR_IIR:
            /*  derive the constants for calculating the gaussian
             *  from the std dev
             */
            find_constants (n_p, n_m, d_p, d_m, bd_p, bd_m, std_dev);
            break;

        case BLUR_RLE:
            curve = make_curve (std_dev, &length);
            sum = new qint32[2 * length + 1];

            sum[0] = 0;

            for (i = 1; i <= length*2; i++)
                sum[i] = curve[i-length-1] + sum[i-1];
            sum += length;

            total = sum[length] - sum[-length];
            break;
        }

        for (col = 0; col < width; col++)
        {
            switch (method)
            {
            case BLUR_IIR:
                memset (val_p, 0, height * bytes * sizeof (double));
                memset (val_m, 0, height * bytes * sizeof (double));
                break;

            case BLUR_RLE:
                break;
            }

            //gimp_pixel_rgn_get_col (&src_rgn, src, col + x1, y1, height);
            srcDev->readBytes(src, col+x1, y1, 1, height);

            multiply_alpha (src, height, bytes);

            switch (method)
            {
            case BLUR_IIR:
                sp_p = src;
                sp_m = src + (height - 1) * bytes;
                vp = val_p;
                vm = val_m + (height - 1) * bytes;

                /*  Set up the first vals  */
                for (i = 0; i < bytes; i++)
                {
                    initial_p[i] = sp_p[i];
                    initial_m[i] = sp_m[i];
                }

                for (row = 0; row < height; row++)
                {
                    double *vpptr, *vmptr;
                    terms = (row < 4) ? row : 4;

                    for (b = 0; b < bytes; b++)
                    {
                        vpptr = vp + b; vmptr = vm + b;
                        for (i = 0; i <= terms; i++)
                        {
                            *vpptr += n_p[i] * sp_p[(-i * bytes) + b] -
                                      d_p[i] * vp[(-i * bytes) + b];
                            *vmptr += n_m[i] * sp_m[(i * bytes) + b] -
                                      d_m[i] * vm[(i * bytes) + b];
                        }
                        for (j = i; j <= 4; j++)
                        {
                            *vpptr += (n_p[j] - bd_p[j]) * initial_p[b];
                            *vmptr += (n_m[j] - bd_m[j]) * initial_m[b];
                        }
                    }

                    sp_p += bytes;
                    sp_m -= bytes;
                    vp += bytes;
                    vm -= bytes;
                }

                transfer_pixels (val_p, val_m, dest, bytes, height);
                break;

            case BLUR_RLE:
                sp = src;
                dp = dest;

                for (b = 0; b < bytes; b++)
                {
                    initial_pp = sp[b];
                    initial_mm = sp[(height-1) * bytes + b];

                    /*  Determine a run-length encoded version of the row  */
                    run_length_encode (sp + b, buf, bytes, height);

                    for (row = 0; row < height; row++)
                    {
                        start = (row < length) ? -row : -length;
                        end = (height <= (row + length) ?
                               (height - row - 1) : length);

                        val = 0;
                        i = start;
                        bb = buf + (row + i) * 2;

                        if (start != -length)
                            val += initial_pp * (sum[start] - sum[-length]);

                        while (i < end)
                        {
                            pixels = bb[0];
                            i += pixels;
                            if (i > end)
                                i = end;
                            val += bb[1] * (sum[i] - sum[start]);
                            bb += (pixels * 2);
                            start = i;
                        }

                        if (end != length)
                            val += initial_mm * (sum[length] - sum[end]);

                        dp[row * bytes + b] = val / total;
                    }
                }
                break;
            }

            separate_alpha (src, height, bytes);

            dstDev->writeBytes(dest, col + x1, y1, 1, height);

            progress += height * vert;
            if ((col % 5) == 0) emit notifyProgress( (quint32)((progress * 100) / max_progress));
        }
    }

    /*  Now the horizontal pass  */
    if (horz > 0.0)
    {
        horz = fabs (horz) + 1.0;

        if (horz != vert)
        {
            std_dev = sqrt (-(horz * horz) / (2 * log (1.0 / 255.0)));

            switch (method)
            {
            case BLUR_IIR:
                /*  derive the constants for calculating the gaussian
                 *  from the std dev
                 */
                find_constants (n_p, n_m, d_p, d_m, bd_p, bd_m, std_dev);
                break;

            case BLUR_RLE:
                curve = make_curve (std_dev, &length);
                sum = new qint32[2 * length + 1];

                sum[0] = 0;

                for (i = 1; i <= length*2; i++)
                    sum[i] = curve[i-length-1] + sum[i-1];
                sum += length;

                total = sum[length] - sum[-length];
                break;
            }
        }

        for (row = 0; row < height; row++)
        {
            switch (method)
            {
            case BLUR_IIR:
                memset (val_p, 0, width * bytes * sizeof (double));
                memset (val_m, 0, width * bytes * sizeof (double));
                break;

            case BLUR_RLE:
                break;
            }


            //gimp_pixel_rgn_get_row (&src_rgn, src, x1, row + y1, width);
            dstDev->readBytes(src, x1, row + y1, width, 1);

            multiply_alpha (dest, width, bytes);

            switch (method)
            {
            case BLUR_IIR:
                sp_p = src;
                sp_m = src + (width - 1) * bytes;
                vp = val_p;
                vm = val_m + (width - 1) * bytes;

                /*  Set up the first vals  */
                for (i = 0; i < bytes; i++)
                {
                    initial_p[i] = sp_p[i];
                    initial_m[i] = sp_m[i];
                }

                for (col = 0; col < width; col++)
                {
                    double *vpptr, *vmptr;
                    terms = (col < 4) ? col : 4;

                    for (b = 0; b < bytes; b++)
                    {
                        vpptr = vp + b; vmptr = vm + b;
                        for (i = 0; i <= terms; i++)
                        {
                            *vpptr += n_p[i] * sp_p[(-i * bytes) + b] -
                                      d_p[i] * vp[(-i * bytes) + b];
                            *vmptr += n_m[i] * sp_m[(i * bytes) + b] -
                                      d_m[i] * vm[(i * bytes) + b];
                        }
                        for (j = i; j <= 4; j++)
                        {
                            *vpptr += (n_p[j] - bd_p[j]) * initial_p[b];
                            *vmptr += (n_m[j] - bd_m[j]) * initial_m[b];
                        }
                    }

                    sp_p += bytes;
                    sp_m -= bytes;
                    vp += bytes;
                    vm -= bytes;
                }

                transfer_pixels (val_p, val_m, dest, bytes, width);
                break;

            case BLUR_RLE:
                sp = src;
                dp = dest;

                for (b = 0; b < bytes; b++)
                {
                    initial_pp = sp[b];
                    initial_mm = sp[(width-1) * bytes + b];

                    /*  Determine a run-length encoded version of the row  */
                    run_length_encode (sp + b, buf, bytes, width);

                    for (col = 0; col < width; col++)
                    {
                        start = (col < length) ? -col : -length;
                        end = (width <= (col + length)) ? (width - col - 1) : length;

                        val = 0;
                        i = start;
                        bb = buf + (col + i) * 2;

                        if (start != -length)
                            val += initial_pp * (sum[start] - sum[-length]);

                        while (i < end)
                        {
                            pixels = bb[0];
                            i += pixels;
                            if (i > end)
                                i = end;
                            val += bb[1] * (sum[i] - sum[start]);
                            bb += (pixels * 2);
                            start = i;
                        }

                        if (end != length)
                            val += initial_mm * (sum[length] - sum[end]);

                        dp[col * bytes + b] = val / total;
                    }
                }
                break;
            }

            separate_alpha (dest, width, bytes);

            //gimp_pixel_rgn_set_row (&dest_rgn, dest, x1, row + y1, width);
            dstDev->writeBytes(dest, x1, row + y1, width, 1);

            progress += width * horz;
            //if ((row % 5) == 0) gimp_progress_update (progress / max_progress);
            if ((row % 5) == 0) emit notifyProgress( (quint32)((progress * 100) / max_progress ));
        }
    }

    /*  free up buffers  */
    switch (method)
    {
    case BLUR_IIR:
        delete[] val_p;
        delete[] val_m;
        break;

    case BLUR_RLE:
        delete[] buf;
        break;
    }

    delete[] src;
    delete[] dest;
}

void KisDropshadow::find_constants (double n_p[], double n_m[], double d_p[], double d_m[], double bd_p[], double bd_m[], double std_dev)
{
    qint32    i;
    double constants [8];
    double div;

    /*  The constants used in the implemenation of a casual sequence
     *  using a 4th order approximation of the gaussian operator
     */

    div = sqrt(2 * M_PI) * std_dev;
    constants [0] = -1.783 / std_dev;
    constants [1] = -1.723 / std_dev;
    constants [2] = 0.6318 / std_dev;
    constants [3] = 1.997  / std_dev;
    constants [4] = 1.6803 / div;
    constants [5] = 3.735 / div;
    constants [6] = -0.6803 / div;
    constants [7] = -0.2598 / div;

    n_p [0] = constants[4] + constants[6];
    n_p [1] = exp (constants[1]) *
              (constants[7] * sin (constants[3]) -
               (constants[6] + 2 * constants[4]) * cos (constants[3])) +
              exp (constants[0]) *
              (constants[5] * sin (constants[2]) -
               (2 * constants[6] + constants[4]) * cos (constants[2]));
    n_p [2] = 2 * exp (constants[0] + constants[1]) *
              ((constants[4] + constants[6]) * cos (constants[3]) * cos (constants[2]) -
               constants[5] * cos (constants[3]) * sin (constants[2]) -
               constants[7] * cos (constants[2]) * sin (constants[3])) +
              constants[6] * exp (2 * constants[0]) +
              constants[4] * exp (2 * constants[1]);
    n_p [3] = exp (constants[1] + 2 * constants[0]) *
              (constants[7] * sin (constants[3]) - constants[6] * cos (constants[3])) +
              exp (constants[0] + 2 * constants[1]) *
              (constants[5] * sin (constants[2]) - constants[4] * cos (constants[2]));
    n_p [4] = 0.0;

    d_p [0] = 0.0;
    d_p [1] = -2 * exp (constants[1]) * cos (constants[3]) -
              2 * exp (constants[0]) * cos (constants[2]);
    d_p [2] = 4 * cos (constants[3]) * cos (constants[2]) * exp (constants[0] + constants[1]) +
              exp (2 * constants[1]) + exp (2 * constants[0]);
    d_p [3] = -2 * cos (constants[2]) * exp (constants[0] + 2 * constants[1]) -
              2 * cos (constants[3]) * exp (constants[1] + 2 * constants[0]);
    d_p [4] = exp (2 * constants[0] + 2 * constants[1]);

    for (i = 0; i <= 4; i++)
        d_m [i] = d_p [i];

    n_m[0] = 0.0;
    for (i = 1; i <= 4; i++)
        n_m [i] = n_p[i] - d_p[i] * n_p[0];

    {
        double sum_n_p, sum_n_m, sum_d;
        double a, b;

        sum_n_p = 0.0;
        sum_n_m = 0.0;
        sum_d = 0.0;
        for (i = 0; i <= 4; i++)
        {
            sum_n_p += n_p[i];
            sum_n_m += n_m[i];
            sum_d += d_p[i];
        }

        a = sum_n_p / (1.0 + sum_d);
        b = sum_n_m / (1.0 + sum_d);

        for (i = 0; i <= 4; i++)
        {
            bd_p[i] = d_p[i] * a;
            bd_m[i] = d_m[i] * b;
        }
    }
}


void KisDropshadow::transfer_pixels (double *src1, double *src2, quint8 *dest, qint32 bytes, qint32 width)
{
    qint32    b;
    qint32    bend = bytes * width;
    double sum;

    for(b = 0; b < bend; b++)
    {
        sum = *src1++ + *src2++;
        if (sum > 255) sum = 255;
        else if(sum < 0) sum = 0;

        *dest++ = (quint8) sum;
    }
}

//The equations: g(r) = exp (- r^2 / (2 * sigma^2)), r = sqrt (x^2 + y ^2)
qint32 * KisDropshadow::make_curve(double sigma, qint32 *length)
{
    int    *curve;
    double  sigma2;
    double  l;
    int     temp;
    int     i, n;

    sigma2 = 2 * sigma * sigma;
    l = sqrt (-sigma2 * log (1.0 / 255.0));

    n = (int)(ceil (l) * 2);
    if ((n % 2) == 0)
        n += 1;

    curve = new qint32[n];

    *length = n / 2;
    curve += *length;
    curve[0] = 255;

    for (i = 1; i <= *length; i++)
    {
        temp = (qint32) (exp (- (i * i) / sigma2) * 255);
        curve[-i] = temp;
        curve[i] = temp;
    }

    return curve;
}

void KisDropshadow::run_length_encode (quint8 *src, qint32 *dest, qint32 bytes, qint32 width)
{
    qint32   start;
    qint32   i;
    qint32   j;
    quint8 last;

    last = *src;
    src += bytes;
    start = 0;

    for (i = 1; i < width; i++)
    {
        if (*src != last)
        {
            for (j = start; j < i; j++)
            {
                *dest++ = (i - j);
                *dest++ = last;
            }
            start = i;
            last = *src;
        }
        src += bytes;
    }

    for (j = start; j < i; j++)
    {
        *dest++ = (i - j);
        *dest++ = last;
    }
}

void KisDropshadow::multiply_alpha (quint8 *buf, qint32 width, qint32 bytes)
{
    qint32    i, j;
    double alpha;

    for (i = 0; i < width * bytes; i += bytes)
    {
        alpha = buf[i + bytes - 1] * (1.0 / 255.0);
        for (j = 0; j < bytes - 1; j++) {
            double a = (double)(buf[i + j]) * alpha;
            buf[i + j] = (quint8)a;
        }
    }
}

void KisDropshadow::separate_alpha (quint8 *buf, qint32 width, qint32 bytes)
{
    qint32   i, j;
    quint8 alpha;
    double recip_alpha;
    quint32    new_val;

    for (i = 0; i < width * bytes; i += bytes)
    {
        alpha = buf[i + bytes - 1];
        if (alpha != 0 && alpha != 255)
        {
            recip_alpha = 255.0 / alpha;
            for (j = 0; j < bytes - 1; j++)
            {
                new_val = (quint32)(buf[i + j] * recip_alpha);
                buf[i + j] = MIN (255, new_val);
            }
        }
    }
}

#include "kis_dropshadow.moc"
