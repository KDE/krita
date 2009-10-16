/*
 *  Copyright (c) 2005,2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_paint_device.h"

#include "krs_image.h"

#include <QBuffer>
#include <klocale.h>

#include <KoColorSpaceRegistry.h>
#include <kis_doc2.h>
#include <kis_layer.h>
#include <kis_iterators_pixel.h>
#include <kis_transaction.h>
#include <kis_math_toolbox.h>
#include <kis_undo_adapter.h>
#include <kis_image.h>

#include "krs_iterator.h"
#include "krs_histogram.h"
#include "krs_painter.h"
#include "krs_wavelet.h"

using namespace Scripting;

PaintDevice::PaintDevice(KisPaintDeviceSP device, KisDoc2* doc)
        : ConstPaintDevice(device, doc)
        , m_device(device)
        , m_cmd(0)
{
    setObjectName("KritaLayer");
}


PaintDevice::~PaintDevice()
{
}

bool PaintDevice::convertToColorSpace(const QString& colorspacename)
{
    const KoColorSpace * dstCS = KoColorSpaceRegistry::instance()->colorSpace(colorspacename, 0);
    if (!dstCS) {
        warnScript << QString("ColorSpace %1 is not available, please check your installation.").arg(colorspacename) << endl;
        return false;
    }
    QUndoCommand* cmd = paintDevice()->convertTo(dstCS);
    delete cmd;
    return true;
}

QObject* PaintDevice::createRectIterator(uint x, uint y, uint width, uint height)
{
    return new Iterator<KisRectIteratorPixel>(this,
            paintDevice()->createRectIterator(x, y, width, height));
}

QObject* PaintDevice::createHLineIterator(uint x, uint y, uint width)
{
    return new Iterator<KisHLineIteratorPixel>(this,
            paintDevice()->createHLineIterator(x, y, width));
}

QObject* PaintDevice::createVLineIterator(uint x, uint y, uint height)
{
    return new Iterator<KisVLineIteratorPixel>(this,
            paintDevice()->createVLineIterator(x, y, height));
}

QObject* PaintDevice::createPainter()
{
    return new Painter(this);
}

void PaintDevice::beginPainting(const QString& name)
{
    if (m_cmd != 0) {
        delete m_cmd;
    }
    m_cmd = new KisTransaction(name, paintDevice());
    Q_CHECK_PTR(m_cmd);
}

void PaintDevice::endPainting()
{
    if (doc() != 0) {
        doc()->setModified(true);
        m_device->setDirty();
    }
    if (m_cmd != 0) {
        doc()->image()->undoAdapter()->addCommand(m_cmd);
    }
}

bool PaintDevice::fastWaveletUntransformation(QObject* wavelet)
{
    Wavelet* wav = dynamic_cast< Wavelet* >(wavelet);
    if (! wav) {
        warnScript << "The passed argument is not a valid Wavelet-object." << endl;
        return false;
    }
    KisMathToolbox* mathToolbox = KisMathToolboxRegistry::instance()->value(paintDevice()->colorSpace()->mathToolboxId().id());
    QRect rect = paintDevice()->exactBounds();
    mathToolbox->fastWaveletUntransformation(paintDevice(), rect, wav->wavelet());
    return true;
}

#if 0
QByteArray PaintDevice::bytes()
{
    qint32 pixelsize = paintDevice()->colorSpace()->pixelSize();
    const int w = width();
    const int h = height();
    const int size = w * h * pixelsize;
    Q_ASSERT(size >= 0);

    QByteArray bytearray;
    QBuffer buffer(&bytearray);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);

    quint8* data = new quint8[size];
    Q_CHECK_PTR(data);
    paintDevice()->readBytes(data, 0, 0, w, h);
    for (int i = 0; i < size; ++i)
        out << data[i];
    delete [] data;

    return bytearray;
}

bool PaintDevice::setBytes(const QByteArray& bytearray)
{
    qint32 pixelsize = paintDevice()->colorSpace()->pixelSize();
    const int w = width();
    const int h = height();
    const int size = w * h * pixelsize;

    if (size < 0 || bytearray.size() < size)
        return false;

    QBuffer buffer(&bytearray);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);

    quint8* data = new quint8[size];
    Q_CHECK_PTR(data);
    for (int i = 0; i < size; ++i)
        in >> data[i];
    paintDevice()->writeBytes(data, 0, 0, w, h);
    delete [] data;

    return true;
}
#endif

#include "krs_paint_device.moc"
