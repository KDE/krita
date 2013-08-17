/*
 *  Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <QImage>

#include <kis_gmic_processing_visitor.h>

#include <klocale.h>

#include "commands_new/kis_node_move_command2.h"

#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "generator/kis_generator_layer.h"

#include "kis_transparency_mask.h"
#include "kis_filter_mask.h"
#include "kis_selection_mask.h"

#include "kis_external_layer_iface.h"

#include "kis_paint_device.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include <kis_painter.h>
#include <KoCompositeOpRegistry.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

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



using namespace cimg_library;

KisGmicProcessingVisitor::KisGmicProcessingVisitor(QString gmicCommandStr, KisView2 * view):
    m_gmicCommandStr(gmicCommandStr),
    m_view(view)
{
}

void KisGmicProcessingVisitor::visit(KisNode *node, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(node);
    Q_UNUSED(undoAdapter);
}

void KisGmicProcessingVisitor::visit(KisCloneLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisGmicProcessingVisitor::visit(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisGmicProcessingVisitor::visit(KisPaintLayer *layer, KisUndoAdapter *undoAdapter)
{
    process(layer, undoAdapter);
}

void KisGmicProcessingVisitor::visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(undoAdapter);

    layer->resetCache();
}

void KisGmicProcessingVisitor::visit(KisAdjustmentLayer *layer, KisUndoAdapter *undoAdapter)
{
    process(layer, undoAdapter);
    layer->resetCache();
}

void KisGmicProcessingVisitor::visit(KisGeneratorLayer *layer, KisUndoAdapter *undoAdapter)
{
    process(layer, undoAdapter);
    layer->resetCache();
}

void KisGmicProcessingVisitor::visit(KisFilterMask *mask, KisUndoAdapter *undoAdapter)
{
    process(mask, undoAdapter);
}

void KisGmicProcessingVisitor::visit(KisTransparencyMask *mask, KisUndoAdapter *undoAdapter)
{
    process(mask, undoAdapter);
}

void KisGmicProcessingVisitor::visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter)
{
    process(mask, undoAdapter);
}

void KisGmicProcessingVisitor::process(KisNode *node, KisUndoAdapter *undoAdapter)
{
    KisPaintDeviceSP device = node->paintDevice();
    KisTransaction transaction(i18n("Gmic filter"), device);

    applyGmicToDevice(device, m_gmicCommandStr);

    transaction.commit(undoAdapter);
}


void KisGmicProcessingVisitor::applyGmicToDevice(KisPaintDeviceSP src, QString gmicCommandStr)
{
    if (!src)
    {
        return;
    }

    gmic_list<float> images;
    gmic_list<char> images_names;
    images.assign(1);

    gmic_image<float>& gimg = images._data[0];

    QRect rc = src->exactBounds();
    quint32 x = rc.width();
    quint32 y = rc.height();
    quint32 z = 1;
    quint32 colorChannelCount = 4; // RGBA
    gimg.assign(x,y,z,colorChannelCount);

    QElapsedTimer timer;
    double milisec;
    timer.start();

    convertToGmicImageOpti(src, gimg);
    //something happens here
    milisec = timer.elapsed() * 0.001;
    qDebug() << "convert to gmic; time: " << milisec << "sec";
    //something else happening here
    timer.restart();

    // Second step : Call G'MIC API to process input images.
    //------------------------------------------------------
    std::fprintf(stderr,"\n- 2st step : Call G'MIC interpreter.\n");


    try
    {
        QString gmicCmd = "-+ -n 0,255 ";
        gmicCmd.append(gmicCommandStr);
        qDebug() << gmicCommandStr;
        gmic(gmicCmd.toLocal8Bit().constData(),images,images_names);
    }
    // Catch exception, if an error occured in the interpreter.
    catch (gmic_exception &e)
    {
        qDebug() << "\n- Error encountered when calling G'MIC : '%s'\n" << e.what();
        return;
    }

    // Third step : get back modified image data.
    //-------------------------------------------
    std::fprintf(stderr,"\n- 3st step : Returned %u output images.\n",images._width);
    for (unsigned int i = 0; i<images._width; ++i)
    {
        std::fprintf(stderr,"   Output image %u = %ux%ux%ux%u, buffer : %p\n",i,
                     images._data[i]._width,
                     images._data[i]._height,
                     images._data[i]._depth,
                     images._data[i]._spectrum,
                     images._data[i]._data);
    }

    milisec = timer.elapsed();
    qDebug() << "apply gmic; time: " << milisec *  0.001 << "sec";
    timer.restart();

    KisPaintDeviceSP dstDev = convertFromGmicImage(images._data[0]);

    milisec = timer.elapsed();
    qDebug() << "convert from gmic; time: " << milisec * 0.001 << "sec";
    timer.restart();


    // Fourth step : Free image resources.
    //-------------------------------------
    unsigned int clearValue = 0;
    images.assign(clearValue);


    // to actual layer colorspace
    dstDev->convertTo(src->colorSpace());
    // bitBlt back -- we don't write the pixel data back directly, but bitBlt so the
    // unselected pixels are not overwritten.
    KisPainter gc(src);
    gc.setCompositeOp(COMPOSITE_COPY);
    gc.bitBlt(rc.topLeft(), dstDev, rc);
    gc.end();

    milisec = timer.elapsed();
    qDebug() << "bitblit; time: " << milisec * 0.001 << "sec";
}


QImage KisGmicProcessingVisitor::convertToQImage(gmic_image<float>& gmicImage)
{

    QImage image = QImage(gmicImage._width, gmicImage._height, QImage::Format_ARGB32);

    qDebug() << image.format() <<"first pixel:"<< gmicImage._data[0] << gmicImage._width << gmicImage._height << gmicImage._spectrum;

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int pos = 0;

    for (unsigned int y = 0; y < gmicImage._height; y++)
    {
        QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (unsigned int x = 0; x < gmicImage._width; x++)
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


void KisGmicProcessingVisitor::convertFromQImage(const QImage& image, CImg< float >& gmicImage)
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


KisPaintDeviceSP KisGmicProcessingVisitor::convertFromGmicImage(CImg< float >& gmicImage)
{
    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    KisPaintDeviceSP dev = new KisPaintDevice(rgbaFloat32bitcolorSpace);
    // TODO: let's reuse planes from readPlanarBytes ;)
    int channelBytes = gmicImage._width * gmicImage._height * sizeof(float);

    QVector< quint8 * > planes;
    planes.resize(rgbaFloat32bitcolorSpace->channelCount());
    for (quint32 i=0; i<rgbaFloat32bitcolorSpace->channelCount();i++)
    {
        planes[i] = new quint8[channelBytes];
    }

    quint8 * redChannelBytes = planes[0];
    quint8 * greenChannelBytes = planes[1];
    quint8 * blueChannelBytes = planes[2];
    quint8 * alphaChannelBytes = planes[3];

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    int pos = 0;

    float r,g,b,a;
    for (unsigned int y = 0; y < gmicImage._height; y++)
    {
        for (unsigned int x = 0; x < gmicImage._width; x++){
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


void KisGmicProcessingVisitor::convertToGmicImage(KisPaintDeviceSP dev, CImg< float >& gmicImage)
{

    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);
    dev->convertTo(rgbaFloat32bitcolorSpace);
    QRect rc = dev->exactBounds();

    QVector<quint8 *> planarBytes = dev->readPlanarBytes(rc.x(), rc.y(), rc.width(), rc.height());

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    int pos = 0;

    quint8 * redChannelBytes = planarBytes.at(KoRgbF32Traits::red_pos);
    quint8 * greenChannelBytes = planarBytes.at(KoRgbF32Traits::green_pos);
    quint8 * blueChannelBytes = planarBytes.at(KoRgbF32Traits::blue_pos);
    quint8 * alphaChannelBytes = planarBytes.at(KoRgbF32Traits::alpha_pos);

    float r,g,b,a;
    for (unsigned int y = 0; y < gmicImage._height; y++)
    {
        for (unsigned int x = 0; x < gmicImage._width; x++){
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

void KisGmicProcessingVisitor::convertToGmicImageOpti(KisPaintDeviceSP dev, CImg< float >& gmicImage)
{
    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);
    dev->convertTo(rgbaFloat32bitcolorSpace);

    QRect rc = dev->exactBounds();
    QVector<quint8 *> planarBytes = dev->readPlanarBytes(rc.x(), rc.y(), rc.width(), rc.height());

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
