/*
 * This file is part of Krita
 *
 * Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_gmic.h"
#include <limits.h>

#include <stdlib.h>
#include <vector>

#include <QColor>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <knuminput.h>

#include <KoColorModelStandardIds.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoChannelInfo.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>

#include <kis_transaction.h>
#include <kis_undo_adapter.h>
#include <kis_global.h>
#include <kis_types.h>

#include <kis_view2.h>
#include <kis_paint_device.h>
#include <kis_convolution_painter.h>
#include <commands/kis_node_commands.h>
#include <kis_node_manager.h>
#include <kis_node_commands_adapter.h>

#include <KoColorSpaceTraits.h>
#include <KoColorProfile.h>

//#include <gmic.h>

using namespace cimg_library;

KisGmic::KisGmic(KisView2 * view)
    : m_view(view)
{
}

QImage KisGmic::convertToQImage(gmic_image<float>& gmicImage)
{

    QImage image = QImage(gmicImage._width, gmicImage._height, QImage::Format_ARGB32);

    qDebug() << image.format() <<"first pixel:"<< gmicImage._data[0] << gmicImage._width << gmicImage._height << gmicImage._spectrum;

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int pos = 0;

    for (int y = 0; y < gmicImage._height; y++)
    {
        QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < gmicImage._width; x++)
        {
            pos = y * gmicImage._width + x;
            float r = gmicImage._data[pos];
            float g = gmicImage._data[pos + greenOffset];
            float b = gmicImage._data[pos + blueOffset];
            pixel[x] = qRgb(int(r),int(g), int(b));
        }
    }
    return image;
}


void KisGmic::convertFromQImage(const QImage& image, CImg< float >& gmicImage)
{
    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    int pos = 0;

    Q_ASSERT(image.width() == gmicImage._width);
    Q_ASSERT(image.height() == gmicImage._height);
    Q_ASSERT(image.format() == QImage::Format_ARGB32);
    Q_ASSERT(gmicImage._spectrum == 4);

    for (int y = 0; y < image.height(); y++)
    {
        const QRgb *pixel = reinterpret_cast<const QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); x++)
        {
            pos = y * gmicImage._width + x;
            gmicImage._data[pos]                = qRed(pixel[x]);
            gmicImage._data[pos + greenOffset]  = qGreen(pixel[x]);
            gmicImage._data[pos + blueOffset]   = qBlue(pixel[x]);
            gmicImage._data[pos + alphaOffset]   = qAlpha(pixel[x]);
        }
    }

}


KisPaintDeviceSP KisGmic::convertFromGmicImage(CImg< float >& gmicImage)
{
    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    KisPaintDeviceSP dev = new KisPaintDevice(rgbaFloat32bitcolorSpace);
    // TODO: let's reuse planes from readPlanarBytes ;)
    int channelBytes = gmicImage._width * gmicImage._height * sizeof(float);

    QVector< quint8 * > planes;
    planes.resize(rgbaFloat32bitcolorSpace->channelCount());
    for (int i=0; i<rgbaFloat32bitcolorSpace->channelCount();i++)
    {
        planes[i] = new quint8[channelBytes];
    }

    // copy red channel
    float * gmicPixelData = gmicImage._data;

    quint8 * redChannelBytes = planes[0];
    quint8 * greenChannelBytes = planes[1];
    quint8 * blueChannelBytes = planes[2];
    quint8 * alphaChannelBytes = planes[3];

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    int pos = 0;

    float r,g,b,a;
    for (int y = 0; y < gmicImage._height; y++)
    {
        for (int x = 0; x < gmicImage._width; x++){
            pos = y * gmicImage._width + x;

            // gmic assumes 0.0 - 255.0, Krita stores 0.0 - 1.0
            r = gmicImage._data[pos]                / 255.0;
            g = gmicImage._data[pos + greenOffset]  / 255.0;
            b = gmicImage._data[pos + blueOffset]   / 255.0;
            a = gmicImage._data[pos + alphaOffset]  / 255.0;

            memcpy(redChannelBytes,     &r, 4); redChannelBytes     += 4;
            memcpy(greenChannelBytes,   &g, 4); greenChannelBytes   += 4;
            memcpy(blueChannelBytes,    &b, 4); blueChannelBytes    += 4;
            memcpy(alphaChannelBytes,   &a, 4); alphaChannelBytes   += 4;
        }
    }

    dev->writePlanarBytes(planes, 0, 0, gmicImage._width, gmicImage._height);
    return dev;
}


void KisGmic::convertToGmicImage(KisPaintDeviceSP dev, CImg< float >& gmicImage)
{

    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);
    dev->convertTo(rgbaFloat32bitcolorSpace);
    QRect rc = dev->exactBounds();

    QVector<quint8 *> planarBytes = dev->readPlanarBytes(rc.x(), rc.y(), rc.width(), rc.height());

    float * gmicPixelData = gmicImage._data;

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    int pos = 0;

    quint8 * redChannelBytes = planarBytes.at(KoRgbF32Traits::red_pos);
    quint8 * greenChannelBytes = planarBytes.at(KoRgbF32Traits::green_pos);
    quint8 * blueChannelBytes = planarBytes.at(KoRgbF32Traits::blue_pos);
    quint8 * alphaChannelBytes = planarBytes.at(KoRgbF32Traits::alpha_pos);

    float r,g,b,a;
    for (int y = 0; y < gmicImage._height; y++)
    {
        for (int x = 0; x < gmicImage._width; x++){
            pos = y * gmicImage._width + x;

            memcpy(&r, redChannelBytes, 4); redChannelBytes     += 4;
            memcpy(&g, greenChannelBytes, 4); greenChannelBytes += 4;
            memcpy(&b, blueChannelBytes, 4); blueChannelBytes   += 4;
            memcpy(&a, alphaChannelBytes, 4); alphaChannelBytes += 4;

            // gmic assumes 0.0 - 255.0, Krita stores 0.0 - 1.0
            gmicImage._data[pos]                = r * 255.0;
            gmicImage._data[pos + greenOffset]  = g * 255.0;
            gmicImage._data[pos + blueOffset]   = b * 255.0;
            gmicImage._data[pos + alphaOffset]  = a * 255.0;
        }
    }

    qDeleteAll(planarBytes);
    planarBytes.clear();

}

void KisGmic::convertToGmicImageOpti(KisPaintDeviceSP dev, CImg< float >& gmicImage)
{
    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);
    dev->convertTo(rgbaFloat32bitcolorSpace);

    QRect rc = dev->exactBounds();
    QVector<quint8 *> planarBytes = dev->readPlanarBytes(rc.x(), rc.y(), rc.width(), rc.height());

    float * gmicPixelData = gmicImage._data;

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    quint8 * redChannelBytes = planarBytes.at(KoRgbF32Traits::red_pos);
    quint8 * greenChannelBytes = planarBytes.at(KoRgbF32Traits::green_pos);
    quint8 * blueChannelBytes = planarBytes.at(KoRgbF32Traits::blue_pos);
    quint8 * alphaChannelBytes = planarBytes.at(KoRgbF32Traits::alpha_pos);

    unsigned int channelSize = sizeof(float);

    memcpy(gmicImage._data                  ,redChannelBytes    ,gmicImage._width * gmicImage._height * channelSize);
    memcpy(gmicImage._data + greenOffset    ,greenChannelBytes  ,gmicImage._width * gmicImage._height * channelSize);
    memcpy(gmicImage._data + blueOffset     ,blueChannelBytes   ,gmicImage._width * gmicImage._height * channelSize);
    memcpy(gmicImage._data + alphaOffset    ,alphaChannelBytes  ,gmicImage._width * gmicImage._height * channelSize);

    qDeleteAll(planarBytes);
    planarBytes.clear();
}



void KisGmic::processGmic(KoUpdater * progressUpdater, const QString &gmicString)
{
    if(!m_view) return;

    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP src = m_view->activeLayer();
    if (!src) return;

    bool conversionNeeded = (src->colorSpace()->colorModelId() != RGBAColorModelID || src->colorSpace()->colorDepthId() != Integer8BitsColorDepthID);

    /*if (conversionNeeded) {
        if (KMessageBox::warningContinueCancel(m_view,
                                               i18n("Applying a G'Mic action will convert your layer to 8 bit RGBA and back. You will lose color information."),
                                               i18n("Krita"))
                == KMessageBox::Cancel) {
            return;
        }
    }*/


    KisPaintDeviceSP dev = src->projection();
    if (!dev) return;
    QRect rc = dev->exactBounds();

    KisTransaction transaction(i18n("Execute G'Mic action"), dev);

    gmic_list<float> images;                            // List of images, will contain all images pixel data.
    gmic_list<char> images_names;                       // List of images names. Can be left empty if no names are associated to images.
    images.assign(1);                                   // Assign list to contain 5 images.
    gmic_image<float>& gimg = images._data[0];
    gimg.assign(rc.width(),rc.height(),1 ,4);

#if 0
    QImage layerImage = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    convertFromQImage(layerImage, gimg);
#endif

    QElapsedTimer timer;
    qint64 nanoSec;
    timer.start();
    //
    convertToGmicImageOpti(dev, gimg);
    //something happens here
    nanoSec = timer.nsecsElapsed();
    qDebug() << "convert to gmic" << nanoSec;
    //something else happening here
    timer.restart();

    // Second step : Call G'MIC API to process input images.
    //------------------------------------------------------
    std::fprintf(stderr,"\n- 2st step : Call G'MIC interpreter.\n");

    try {

        // Here you can call any G'MIC command you want !
        // (here, create a deformed average of the input images, and save it as a BMP file).
        //gmic("-+ -n 0,255 -flower 8 -sharpen 100 -o foo.bmp",images,images_names);
        QString gmicCmd = "-+ -n 0,255 ";
        gmicCmd.append(gmicString);
        gmic(gmicCmd.toLocal8Bit().constData(),images,images_names);
        //gmic("-+ -n 0,255 -o foo.bmp",images,images_names);

    ;


    } catch (gmic_exception &e) { // Catch exception, if an error occured in the interpreter.
        qDebug() << "\n- Error encountered when calling G'MIC : '%s'\n" << e.what();
        return;
    }

    // Third step : get back modified image data.
    //-------------------------------------------
    std::fprintf(stderr,"\n- 3st step : Returned %u output images.\n",images._width);
    for (unsigned int i = 0; i<images._width; ++i) {
        std::fprintf(stderr,"   Output image %u = %ux%ux%ux%u, buffer : %p\n",i,
                     images._data[i]._width,
                     images._data[i]._height,
                     images._data[i]._depth,
                     images._data[i]._spectrum,
                     images._data[i]._data);
    }



    nanoSec = timer.nsecsElapsed();
    qDebug() << "apply gmic" << nanoSec;
    timer.restart();


    // write the bytes to a paint device
    //KisPaintDeviceSP dstDev = new KisPaintDevice(dev->colorSpace());

#if 0
    // from QImage
    QImage qimg = convertToQImage(images._data[0]);
    dstDev->convertFromQImage(qimg, 0, dev->x(), dev->y());
#endif
    progressUpdater->setProgress(50);

    KisPaintDeviceSP dstDev = convertFromGmicImage(images._data[0]);
    // to actual layer colorspace
    dstDev->convertTo(src->paintDevice()->colorSpace());

    qDebug() << "colorspace" << src->paintDevice()->colorSpace()->name();

    nanoSec = timer.nsecsElapsed();
    qDebug() << "convert from gmic" << nanoSec;
    timer.restart();


    // Fourth step : Free image resources.
    //-------------------------------------
    unsigned int clearValue = 0;
    images.assign(clearValue);



    // bitBlt back -- we don't write the pixel data back directly, but bitBlt so the
    // unselected pixels are not overwritten.
    KisPainter gc(src->paintDevice());
    gc.setCompositeOp(COMPOSITE_COPY);
    gc.setSelection(m_view->selection());
    gc.bitBlt(rc.topLeft(), dstDev, rc);
    gc.end();

    nanoSec = timer.nsecsElapsed();
    qDebug() << "bitblit" << nanoSec;

    progressUpdater->setProgress(100);
    transaction.commit(image->undoAdapter());
    src->setDirty(rc);
}


#include "kis_gmic.moc"
