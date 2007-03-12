/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <QRect>
#include <QMatrix>
#include <QImage>
#include <QDateTime>
#include <QApplication>
#include <QList>
#include <QTimer>
#include <QUndoCommand>

#include <klocale.h>
#include <kdebug.h>

#include "KoColorProfile.h"
#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoIntegerMaths.h"
#include <KoStore.h>

#include "kis_global.h"
#include "kis_paint_engine.h"
#include "kis_types.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_iterator.h"
#include "kis_iterators_pixel.h"
#include "kis_iteratorpixeltrait.h"
#include "kis_random_accessor.h"
#include "kis_random_sub_accessor.h"
#include "kis_selection.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_datamanager.h"
#include "kis_memento.h"
#include "kis_undo_adapter.h"

#include "kis_exif_info.h"


namespace {

    class KisPaintDeviceCommand : public QUndoCommand {
        typedef QUndoCommand super;

    public:
        KisPaintDeviceCommand(const QString& name, KisPaintDeviceSP paintDevice);
        virtual ~KisPaintDeviceCommand() {}

    protected:
        void setUndo(bool undo);

        KisPaintDeviceSP m_paintDevice;
    };

    KisPaintDeviceCommand::KisPaintDeviceCommand(const QString& name, KisPaintDeviceSP paintDevice) :
        super(name), m_paintDevice(paintDevice)
    {
    }

    void KisPaintDeviceCommand::setUndo(bool undo)
    {
        if (m_paintDevice->undoAdapter()) {
            m_paintDevice->undoAdapter()->setUndo(undo);
        }
    }

    class KisConvertLayerTypeCmd : public KisPaintDeviceCommand {
        typedef KisPaintDeviceCommand super;

    public:
        KisConvertLayerTypeCmd(KisPaintDeviceSP paintDevice,
                       KisDataManagerSP beforeData, KoColorSpace * beforeColorSpace,
                       KisDataManagerSP afterData, KoColorSpace * afterColorSpace
                ) : super(i18n("Convert Layer Type"), paintDevice)
            {
                m_beforeData = beforeData;
                m_beforeColorSpace = beforeColorSpace;
                m_afterData = afterData;
                m_afterColorSpace = afterColorSpace;
            }

        virtual ~KisConvertLayerTypeCmd()
            {
            }

    public:
        virtual void redo()
            {
                setUndo(false);
                m_paintDevice->setData(m_afterData, m_afterColorSpace);
                setUndo(true);
            }

        virtual void undo()
            {
                setUndo(false);
                m_paintDevice->setData(m_beforeData, m_beforeColorSpace);
                setUndo(true);
            }

    private:
        KisDataManagerSP m_beforeData;
        KoColorSpace * m_beforeColorSpace;

        KisDataManagerSP m_afterData;
        KoColorSpace * m_afterColorSpace;
    };

}

KisPaintDevice::KisPaintDevice(KoColorSpace * colorSpace, const QString& name) :
        QObject(0), m_exifInfo(0)
{
    setObjectName(name);
    if (colorSpace == 0) {
        kWarning(41001) << "Cannot create paint device without colorspace!\n";
        return;
    }
    m_longRunningFilterTimer = 0;

    m_x = 0;
    m_y = 0;

    m_pixelSize = colorSpace->pixelSize();
    m_nChannels = colorSpace->channelCount();

    quint8* defPixel = new quint8 [ m_pixelSize ];
    colorSpace->fromQColor(Qt::black, OPACITY_TRANSPARENT, defPixel);

    m_datamanager = new KisDataManager(m_pixelSize, defPixel);
    delete [] defPixel;

    Q_CHECK_PTR(m_datamanager);
    m_extentIsValid = true;

    m_parentLayer = 0;

    m_colorSpace = colorSpace;

    m_hasSelection = false;
    m_selectionDeselected = false;
    m_selection = 0;
    m_paintEngine = new KisPaintEngine();


}

KisPaintDevice::KisPaintDevice(KisLayer *parent, KoColorSpace * colorSpace, const QString& name) :
        QObject(0), m_exifInfo(0)
{
    setObjectName(name);
    Q_ASSERT( colorSpace );
    m_longRunningFilterTimer = 0;

    m_x = 0;
    m_y = 0;

    m_hasSelection = false;
    m_selectionDeselected = false;
    m_selection = 0;

    m_parentLayer = parent;

    if (colorSpace == 0 && parent != 0 && parent->image() != 0) {
        m_colorSpace = parent->image()->colorSpace();
    }
    else {
        m_colorSpace = colorSpace;
    }

    m_pixelSize = m_colorSpace->pixelSize();
    m_nChannels = m_colorSpace->channelCount();

    quint8* defPixel = new quint8[ m_pixelSize ];
    colorSpace->fromQColor(Qt::black, OPACITY_TRANSPARENT, defPixel);

    m_datamanager = new KisDataManager(m_pixelSize, defPixel);
    delete [] defPixel;
    Q_CHECK_PTR(m_datamanager);
    m_extentIsValid = true;
    m_paintEngine = new KisPaintEngine();

}


KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs) : QObject(), KisShared(rhs), QPaintDevice()
{
    if (this != &rhs) {
        m_longRunningFilterTimer = 0;
        m_parentLayer = 0;
        if (rhs.m_datamanager) {
            m_datamanager = new KisDataManager(*rhs.m_datamanager);
            Q_CHECK_PTR(m_datamanager);
        }
        else {
            kWarning() << "rhs " << rhs.objectName() << " has no datamanager\n";
        }
        m_extentIsValid = rhs.m_extentIsValid;
        m_x = rhs.m_x;
        m_y = rhs.m_y;
        m_colorSpace = rhs.m_colorSpace;
        m_hasSelection = false;
        m_selection = 0;
        m_pixelSize = rhs.m_pixelSize;
        m_nChannels = rhs.m_nChannels;
        if(rhs.m_exifInfo)
        {
            m_exifInfo = new KisExifInfo(*rhs.m_exifInfo);
        }
        else {
            m_exifInfo = 0;
        }
        m_paintEngine = rhs.m_paintEngine;
    }
}

KisPaintDevice::~KisPaintDevice()
{
    delete m_longRunningFilterTimer;
    QList<KisFilter*>::iterator it;
    QList<KisFilter*>::iterator end = m_longRunningFilters.end();
    for (it = m_longRunningFilters.begin(); it != end; ++it) {
        KisFilter * f = (*it);
        delete f;
    }
    m_longRunningFilters.clear();
    delete m_paintEngine;
    //delete m_exifInfo;
}

QPaintEngine * KisPaintDevice::paintEngine () const
{
    return m_paintEngine;
}

int KisPaintDevice::metric( PaintDeviceMetric metric ) const
{
    QRect rc = exactBounds();
    int depth = colorSpace()->pixelSize() - 1;

    switch (metric) {
    case PdmWidth:
        return rc.width();
        break;

    case PdmHeight:
        return rc.height();
        break;

    case PdmWidthMM:
        return qRound( rc.width() * ( 72 / 25.4 ) );
        break;

    case PdmHeightMM:
        return qRound( rc.height() * ( 72 / 25.4 ) );
        break;

    case PdmNumColors:
        return depth * depth;
        break;

    case PdmDepth:
        return qRound( colorSpace()->pixelSize() * 8 ); // in bits
        break;

    case PdmDpiX:
        return 72;
        break;

    case PdmDpiY:
        return 72;
        break;

    case PdmPhysicalDpiX:
        return 72;
        break;

    case PdmPhysicalDpiY:
        return 72;
        break;
    default:
        qWarning("QImage::metric(): Unhandled metric type %d", metric);
        break;
    }
    return 0;
}

void KisPaintDevice::startBackgroundFilters()
{
    m_longRunningFilters = m_colorSpace->createBackgroundFilters();
    if (!m_longRunningFilters.isEmpty()) {
        m_longRunningFilterTimer = new QTimer(this);
        connect(m_longRunningFilterTimer, SIGNAL(timeout()), this, SLOT(runBackgroundFilters()));
        m_longRunningFilterTimer->start(800);
    }
}

KisLayer *KisPaintDevice::parentLayer() const
{
    return m_parentLayer;
}

void KisPaintDevice::setParentLayer(KisLayer *parentLayer)
{
    m_parentLayer = parentLayer;
}

void KisPaintDevice::setDirty(const QRect & rc)
{
    if (m_parentLayer) m_parentLayer->setDirty(rc);
}

void KisPaintDevice::setDirty( const QRegion & region )
{
    if ( m_parentLayer ) m_parentLayer->setDirty( region );
}

void KisPaintDevice::setDirty()
{
    if (m_parentLayer) m_parentLayer->setDirty();
}

KisImageSP KisPaintDevice::image() const
{
    if (m_parentLayer) {
        return m_parentLayer->image();
    } else {
        return 0;
    }
}


void KisPaintDevice::move(qint32 x, qint32 y)
{
    QRect dirtyRegion = extent();

    m_x = x;
    m_y = y;

    dirtyRegion |= extent();

    if(m_selection)
    {
        m_selection->setX(x);
        m_selection->setY(y);
    }

    setDirty(dirtyRegion);

    emit positionChanged(KisPaintDeviceSP(this));
}

void KisPaintDevice::move(const QPoint& pt)
{
    move(pt.x(), pt.y());
}

void KisPaintDevice::extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const
{
    m_datamanager->extent(x, y, w, h);
    x += m_x;
    y += m_y;
}

QRect KisPaintDevice::extent() const
{
    qint32 x, y, w, h;
    extent(x, y, w, h);
    return QRect(x, y, w, h);
}

QRegion KisPaintDevice::region() const
{
    return m_datamanager->region();
}

bool KisPaintDevice::extentIsValid() const
{
    return m_extentIsValid;
}

void KisPaintDevice::setExtentIsValid(bool isValid)
{
    m_extentIsValid = isValid;
}

void KisPaintDevice::exactBounds(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const
{
    QRect r = exactBounds();
    x = r.x();
    y = r.y();
    w = r.width();
    h = r.height();
}

QRect KisPaintDevice::exactBounds() const
{
    qint32 x, y, w, h, boundX, boundY, boundW, boundH;
    extent(x, y, w, h);

    extent(boundX, boundY, boundW, boundH);

    const quint8* defaultPixel = m_datamanager->defaultPixel();

    bool found = false;

    KisHLineConstIterator it = this->createHLineConstIterator(x, y, w);
    for (qint32 y2 = y; y2 < y + h ; ++y2) {
        while (!it.isDone() && found == false) {
            if (memcmp(it.rawData(), defaultPixel, m_pixelSize) != 0) {
                boundY = y2;
                found = true;
                break;
            }
            ++it;
        }
        it.nextRow();
        if (found) break;
    }

    found = false;

    for (qint32 y2 = y + h; y2 > y ; --y2) {
        KisHLineConstIterator it = this->createHLineConstIterator(x, y2, w);
        while (!it.isDone() && found == false) {
            if (memcmp(it.rawData(), defaultPixel, m_pixelSize) != 0) {
                boundH = y2 - boundY + 1;
                found = true;
                break;
            }
            ++it;
        }
        if (found) break;
    }
    found = false;

    KisVLineConstIterator vit = createVLineConstIterator(x, y, h);
    for (qint32 x2 = x; x2 < x + w ; ++x2) {
        while (!vit.isDone() && found == false) {
            if (memcmp(it.rawData(), defaultPixel, m_pixelSize) != 0) {
                boundX = x2;
                found = true;
                break;
            }
            ++vit;
        }
        vit.nextCol();
        if (found) break;
    }

    found = false;

    // Look for right edge )
    for (qint32 x2 = x + w; x2 > x ; --x2) {
        KisVLineConstIterator it = this->createVLineConstIterator(x2, y, h);
        while (!it.isDone() && found == false) {
            if (memcmp(it.rawData(), defaultPixel, m_pixelSize) != 0) {
                boundW = x2 - boundX + 1; // XXX: I commented this
                                          // +1 out, but why? It
                                          // should be correct, since
                                          // we've found the first
                                          // pixel that should be
                                          // included, and it should
                                          // be added to the width.
                found = true;
                break;
            }
            ++it;
        }
        if (found) break;
    }

    return QRect(boundX, boundY, boundW, boundH);
}

void KisPaintDevice::crop(qint32 x, qint32 y, qint32 w, qint32 h)
{
     m_datamanager->setExtent(x - m_x, y - m_y, w, h);
}


void KisPaintDevice::crop(QRect r)
{
    r.translate(-m_x, -m_y);
    m_datamanager->setExtent(r);
}

void KisPaintDevice::clear()
{
    m_datamanager->clear();
}

void KisPaintDevice::fill(qint32 x, qint32 y, qint32 w, qint32 h, const quint8 *fillPixel)
{
    m_datamanager->clear(x, y, w, h, fillPixel);
}

void KisPaintDevice::mirrorX()
{
    QRect r;
    if (hasSelection()) {
        r = selection()->selectedExactRect();
    }
    else {
        r = exactBounds();
    }

    KisHLineConstIteratorPixel srcIt = createHLineConstIterator(r.x(), r.top(), r.width());
    KisHLineIteratorPixel dstIt = createHLineIterator(r.x(), r.top(), r.width());

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {

        dstIt += r.width() - 1;

        while (!srcIt.isDone()) {
            if (srcIt.isSelected()) {
                memcpy(dstIt.rawData(), srcIt.oldRawData(), m_pixelSize);
            }
            ++srcIt;
            --dstIt;

        }
        srcIt.nextRow();
        dstIt.nextRow();
    }
    if (m_parentLayer) {
        m_parentLayer->setDirty(r);
    }
}

void KisPaintDevice::mirrorY()
{
    /* Read a line from bottom to top and and from top to bottom and write their values to each other */
    QRect r;
    if (hasSelection()) {
        r = selection()->selectedExactRect();
    }
    else {
        r = exactBounds();
    }


    qint32 y1, y2;
    for (y1 = r.top(), y2 = r.bottom(); y1 <= r.bottom(); ++y1, --y2) {
        KisHLineIteratorPixel itTop = createHLineIterator(r.x(), y1, r.width());
        KisHLineConstIteratorPixel itBottom = createHLineConstIterator(r.x(), y2, r.width());
        while (!itTop.isDone() && !itBottom.isDone()) {
            if (itBottom.isSelected()) {
                memcpy(itTop.rawData(), itBottom.oldRawData(), m_pixelSize);
            }
            ++itBottom;
            ++itTop;
        }
    }

    if (m_parentLayer) {
        m_parentLayer->setDirty(r);
    }
}

KisMementoSP KisPaintDevice::getMemento()
{
    return m_datamanager->getMemento();
}

void KisPaintDevice::rollback(KisMementoSP memento) { m_datamanager->rollback(memento); }

void KisPaintDevice::rollforward(KisMementoSP memento) { m_datamanager->rollforward(memento); }

bool KisPaintDevice::write(KoStore *store)
{
    bool retval = m_datamanager->write(store);
    emit ioProgress(100);

        return retval;
}

bool KisPaintDevice::read(KoStore *store)
{
    bool retval = m_datamanager->read(store);
    emit ioProgress(100);

        return retval;
}

void KisPaintDevice::convertTo(KoColorSpace * dstColorSpace, qint32 renderingIntent)
{
    if ( (colorSpace()->id() == dstColorSpace->id()) )
    {
        return;
    }

    KisPaintDevice dst(dstColorSpace);
    dst.setX(getX());
    dst.setY(getY());

    qint32 x, y, w, h;
    extent(x, y, w, h);

    for (qint32 row = y; row < y + h; ++row) {

        qint32 column = x;
        qint32 columnsRemaining = w;

        while (columnsRemaining > 0) {

            qint32 numContiguousDstColumns = dst.numContiguousColumns(column, row, row);
            qint32 numContiguousSrcColumns = numContiguousColumns(column, row, row);

            qint32 columns = qMin(numContiguousDstColumns, numContiguousSrcColumns);
            columns = qMin(columns, columnsRemaining);

            KisHLineConstIteratorPixel srcIt = createHLineConstIterator(column, row, columns);
            KisHLineIteratorPixel dstIt = dst.createHLineIterator(column, row, columns);

            const quint8 *srcData = srcIt.rawData();
            quint8 *dstData = dstIt.rawData();

            m_colorSpace->convertPixelsTo(srcData, dstData, dstColorSpace, columns, renderingIntent);

            column += columns;
            columnsRemaining -= columns;
        }
    }

    KisDataManagerSP oldData = m_datamanager;
    KoColorSpace *oldColorSpace = m_colorSpace;

    setData(dst.m_datamanager, dstColorSpace);

    if (undoAdapter() && undoAdapter()->undo()) {
        undoAdapter()->addCommand(new KisConvertLayerTypeCmd(KisPaintDeviceSP(this), oldData, oldColorSpace, m_datamanager, m_colorSpace));
    }
    // XXX: emit colorSpaceChanged(dstColorSpace);
}

void KisPaintDevice::setProfile(KoColorProfile * profile)
{
    if (profile == 0) return;

    KoColorSpace * dstSpace =
            KoColorSpaceRegistry::instance()->colorSpace( colorSpace()->id(),
                                                                      profile);
    if (dstSpace)
        m_colorSpace = dstSpace;
    emit profileChanged(profile);
}

void KisPaintDevice::setData(KisDataManagerSP data, KoColorSpace * colorSpace)
{
    m_datamanager = data;
    m_colorSpace = colorSpace;
    m_pixelSize = m_colorSpace->pixelSize();
    m_nChannels = m_colorSpace->channelCount();

    if (m_parentLayer) {
        m_parentLayer->setDirty(extent());
        m_parentLayer->notifyPropertyChanged();
    }
}

KisUndoAdapter *KisPaintDevice::undoAdapter() const
{
    if (m_parentLayer && m_parentLayer->image()) {
        return m_parentLayer->image()->undoAdapter();
    }
    return 0;
}

void KisPaintDevice::convertFromQImage(const QImage& image, const QString &srcProfileName,
                                       qint32 offsetX, qint32 offsetY)
{
    Q_UNUSED( srcProfileName );

    QImage img = image;

    if (img.format() != QImage::Format_ARGB32) {
        img = img.convertToFormat(QImage::Format_ARGB32);
    }
#ifdef __GNUC__
#warning "KisPaintDevice::convertFromQImage. re-enable use of srcProfileName"
#endif
#if 0

    // XXX: Apply import profile
    if (colorSpace() == KoColorSpaceRegistry::instance() ->colorSpace("RGBA",0)) {
        writeBytes(img.bits(), 0, 0, img.width(), img.height());
    }
    else {
#endif
#if 0
        quint8 * dstData = new quint8[img.width() * img.height() * pixelSize()];
        KoColorSpaceRegistry::instance()
                ->colorSpace("RGBA", srcProfileName)->
                        convertPixelsTo(img.bits(), dstData, colorSpace(), img.width() * img.height());
        writeBytes(dstData, offsetX, offsetY, img.width(), img.height());
#endif
        KisPainter p( this );
        p.bitBlt(offsetX, offsetY, colorSpace()->compositeOp( COMPOSITE_OVER ),
                 &image, OPACITY_OPAQUE,
                 0, 0, image.width(), image.height());
        p.end();

//    }
}

QImage KisPaintDevice::convertToQImage(KoColorProfile *  dstProfile, float exposure)
{
    qint32 x1;
    qint32 y1;
    qint32 w;
    qint32 h;

    x1 = - getX();
    y1 = - getY();

    if (image()) {
        w = image()->width();
        h = image()->height();
    }
    else {
        extent(x1, y1, w, h);
    }

    return convertToQImage(dstProfile, x1, y1, w, h, exposure);
}

// XXX: is this faster than building the QImage ourselves? It makes
QImage KisPaintDevice::convertToQImage(KoColorProfile *  dstProfile, qint32 x1, qint32 y1, qint32 w, qint32 h, float exposure)
{
    if (w < 0)
        return QImage();

    if (h < 0)
        return QImage();

    quint8 * data;
    try {
        data = new quint8 [w * h * m_pixelSize];
    } catch(std::bad_alloc)
    {
        kWarning() << "KisPaintDevice::convertToQImage std::bad_alloc for " << w << " * " << h << " * " << m_pixelSize << endl;
        //delete[] data; // data is not allocated, so don't free it
        return QImage();
    }
    Q_CHECK_PTR(data);

    // XXX: Is this really faster than converting line by line and building the QImage directly?
    //      This copies potentially a lot of data.
    readBytes(data, x1, y1, w, h);
    QImage image = colorSpace()->convertToQImage(data, w, h, dstProfile, INTENT_PERCEPTUAL, exposure);
    delete[] data;

    return image;
}

KisPaintDeviceSP KisPaintDevice::createThumbnailDevice(qint32 w, qint32 h) const
{
    KisPaintDeviceSP thumbnail = KisPaintDeviceSP(new KisPaintDevice(colorSpace(), "thumbnail"));
    thumbnail->clear();

    int srcw, srch;
    if( image() )
    {
        srcw = image()->width();
        srch = image()->height();
    }
    else
    {
        const QRect e = exactBounds();
        srcw = e.width();
        srch = e.height();
    }

    if (w > srcw)
    {
        w = srcw;
        h = qint32(double(srcw) / w * h);
    }
    if (h > srch)
    {
        h = srch;
        w = qint32(double(srch) / h * w);
    }

    if (srcw > srch)
        h = qint32(double(srch) / srcw * w);
    else if (srch > srcw)
        w = qint32(double(srcw) / srch * h);
#ifdef __GNUC__
#warning FIXME: use KisRandomAccessor instead !
#endif
#if 0
    for (qint32 y=0; y < h; ++y) {
        qint32 iY = (y * srch ) / h;
        for (qint32 x=0; x < w; ++x) {
            qint32 iX = (x * srcw ) / w;

             thumbnail->setPixel(x, y, colorAt(iX, iY));
        }
    }
#endif
    return thumbnail;

}


QImage KisPaintDevice::createThumbnail(qint32 w, qint32 h)
{
    int srcw, srch;
    if( image() )
    {
        srcw = image()->width();
        srch = image()->height();
    }
    else
    {
        const QRect e = extent();
        srcw = e.width();
        srch = e.height();
    }

    if (w > srcw)
    {
        w = srcw;
        h = qint32(double(srcw) / w * h);
    }
    if (h > srch)
    {
        h = srch;
        w = qint32(double(srch) / h * w);
    }

    if (srcw > srch)
        h = qint32(double(srch) / srcw * w);
    else if (srch > srcw)
        w = qint32(double(srcw) / srch * h);

    QColor c;
    quint8 opacity;
    QImage img(w, h, QImage::Format_ARGB32);

    for (qint32 y=0; y < h; ++y) {
        qint32 iY = (y * srch ) / h;
        for (qint32 x=0; x < w; ++x) {
            qint32 iX = (x * srcw ) / w;
            pixel(iX, iY, &c, &opacity);
            const QRgb rgb = c.rgb();
            img.setPixel(x, y, qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), opacity));
        }
    }

    return img;
}

KisRectIteratorPixel KisPaintDevice::createRectIterator(qint32 left, qint32 top, qint32 w, qint32 h)
{
    if(hasSelection())
        return KisRectIteratorPixel( m_datamanager.data(), m_selection->m_datamanager.data(), left, top, w, h, m_x, m_y);
    else
        return KisRectIteratorPixel( m_datamanager.data(), NULL, left, top, w, h, m_x, m_y);
}

KisRectConstIteratorPixel KisPaintDevice::createRectConstIterator(qint32 left, qint32 top, qint32 w, qint32 h) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    if(hasSelection())
    {
        KisDataManager* sm = const_cast< KisDataManager*>(m_selection->m_datamanager.data());
        return KisRectConstIteratorPixel( dm, sm, left, top, w, h, m_x, m_y);
    }
    else
        return KisRectConstIteratorPixel( dm, NULL, left, top, w, h, m_x, m_y);
}

KisHLineIteratorPixel  KisPaintDevice::createHLineIterator(qint32 x, qint32 y, qint32 w)
{
    if(hasSelection())
        return KisHLineIteratorPixel( m_datamanager.data(), m_selection->m_datamanager.data(), x, y, w, m_x, m_y);
    else
        return KisHLineIteratorPixel( m_datamanager.data(), NULL, x, y, w, m_x, m_y);
}

KisHLineConstIteratorPixel  KisPaintDevice::createHLineConstIterator(qint32 x, qint32 y, qint32 w) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    if(hasSelection())
    {
        KisDataManager* sm = const_cast< KisDataManager*>(m_selection->m_datamanager.data());
        return KisHLineConstIteratorPixel( dm, sm, x, y, w, m_x, m_y);
    }
    else
        return KisHLineConstIteratorPixel( dm, NULL, x, y, w, m_x, m_y);
}

KisVLineIteratorPixel  KisPaintDevice::createVLineIterator(qint32 x, qint32 y, qint32 h)
{
    if(hasSelection())
        return KisVLineIteratorPixel( m_datamanager.data(), m_selection->m_datamanager.data(), x, y, h, m_x, m_y);
    else
        return KisVLineIteratorPixel( m_datamanager.data(), NULL, x, y, h, m_x, m_y);

}

KisVLineConstIteratorPixel  KisPaintDevice::createVLineConstIterator(qint32 x, qint32 y, qint32 h) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    if(hasSelection())
    {
        KisDataManager* sm = const_cast< KisDataManager*>(m_selection->m_datamanager.data());
        return KisVLineConstIteratorPixel( dm, sm, x, y, h, m_x, m_y);
    }
    else
        return KisVLineConstIteratorPixel( dm, NULL, x, y, h, m_x, m_y);

}

KisRandomAccessorPixel KisPaintDevice::createRandomAccessor(Q_INT32 x, Q_INT32 y) {
    if(hasSelection())
        return KisRandomAccessorPixel(m_datamanager.data(), m_selection->m_datamanager.data(), x, y, m_x, m_y);
    else
        return KisRandomAccessorPixel(m_datamanager.data(), NULL, x, y, m_x, m_y);
}

KisRandomConstAccessorPixel KisPaintDevice::createRandomConstAccessor(Q_INT32 x, Q_INT32 y) const {
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    if(hasSelection())
    {
        KisDataManager* sm = const_cast< KisDataManager*>(m_selection->m_datamanager.data());
        return KisRandomConstAccessorPixel(dm, sm, x, y, m_x, m_y);
    }
    else
        return KisRandomConstAccessorPixel(dm, NULL, x, y, m_x, m_y);
}

KisRandomSubAccessorPixel KisPaintDevice::createRandomSubAccessor() const
{
    KisPaintDevice* pd = const_cast<KisPaintDevice*>(this);
    return KisRandomSubAccessorPixel(pd);
}

void KisPaintDevice::emitSelectionChanged()
{
    if (m_parentLayer && m_parentLayer->image()) {
        m_parentLayer->image()->slotSelectionChanged();
    }
}

void KisPaintDevice::emitSelectionChanged(const QRect& r)
{
    if (m_parentLayer && m_parentLayer->image()) {
        m_parentLayer->image()->slotSelectionChanged(r);
    }
}

KisSelectionSP KisPaintDevice::selection()
{
    if ( m_selectionDeselected && m_selection ) {
        m_selectionDeselected = false;
    }
    else if (!m_selection) {
        m_selection = KisSelectionSP(new KisSelection(KisPaintDeviceSP(this)));
        Q_CHECK_PTR(m_selection);
        m_selection->setX(m_x);
        m_selection->setY(m_y);
    }
    m_hasSelection = true;

    return m_selection;
}

const KisSelectionSP KisPaintDevice::selection() const
{
    if ( m_selectionDeselected && m_selection ) {
        m_selectionDeselected = false;
    }
    else if (!m_selection) {
        m_selection = KisSelectionSP(new KisSelection(const_cast<KisPaintDevice*>(this)));
        Q_CHECK_PTR(m_selection);
        m_selection->setX(m_x);
        m_selection->setY(m_y);
    }
    m_hasSelection = true;

    return m_selection;
}


bool KisPaintDevice::hasSelection() const
{
    return m_hasSelection;
}

bool KisPaintDevice::selectionDeselected()
{
    return m_selectionDeselected;
}


void KisPaintDevice::deselect()
{
    if (m_selection && m_hasSelection) {
        m_hasSelection = false;
        m_selectionDeselected = true;
    }
}

void KisPaintDevice::reselect()
{
    m_hasSelection = true;
    m_selectionDeselected = false;
}

void KisPaintDevice::addSelection(KisSelectionSP selection) {

    KisPainter painter(this->selection());
    QRect r = selection->selectedExactRect();
    painter.bitBlt(r.x(), r.y(), COMPOSITE_OVER, KisPaintDeviceSP(selection.data()), r.x(), r.y(), r.width(), r.height());
    painter.end();
}

void KisPaintDevice::subtractSelection(KisSelectionSP selection) {
    KisPainter painter(KisPaintDeviceSP(this->selection().data()));
    selection->invert();

    QRect r = selection->selectedExactRect();
    painter.bitBlt(r.x(), r.y(), COMPOSITE_ERASE, KisPaintDeviceSP(selection.data()), r.x(), r.y(), r.width(), r.height());

    selection->invert();
    painter.end();
}

void KisPaintDevice::clearSelection()
{
    if (!hasSelection()) return;

    QRect r = m_selection->selectedExactRect();

    if (r.isValid()) {

        KisHLineIterator devIt = createHLineIterator(r.x(), r.y(), r.width());
        KisHLineConstIterator selectionIt = m_selection->createHLineIterator(r.x(), r.y(), r.width());

        for (qint32 y = 0; y < r.height(); y++) {

            while (!devIt.isDone()) {
                // XXX: Optimize by using stretches

                m_colorSpace->applyInverseAlphaU8Mask( devIt.rawData(), selectionIt.rawData(), 1);

                ++devIt;
                ++selectionIt;
            }
            devIt.nextRow();
            selectionIt.nextRow();
        }

        if (m_parentLayer) {
            m_parentLayer->setDirty(r);
        }
    }
}

void KisPaintDevice::applySelectionMask(KisSelectionSP mask)
{
    QRect r = mask->selectedExactRect();
    crop(r);

    KisHLineIterator pixelIt = createHLineIterator(r.x(), r.top(), r.width());
    KisHLineConstIterator maskIt = mask->createHLineIterator(r.x(), r.top(), r.width());

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {

        while (!pixelIt.isDone()) {
            // XXX: Optimize by using stretches

            m_colorSpace->applyAlphaU8Mask( pixelIt.rawData(), maskIt.rawData(), 1);

            ++pixelIt;
            ++maskIt;
        }
        pixelIt.nextRow();
        maskIt.nextRow();
    }
}

KisSelectionSP KisPaintDevice::setSelection( KisSelectionSP selection)
{
    if (selection) {
        KisSelectionSP oldSelection = m_selection;
        m_selection = selection;
        m_hasSelection = true;
        return oldSelection;
    }
    else return KisSelectionSP(0);
}

bool KisPaintDevice::pixel(qint32 x, qint32 y, QColor *c, quint8 *opacity)
{
    KisHLineConstIteratorPixel iter = createHLineIterator(x, y, 1);

    const quint8 *pix = iter.rawData();

    if (!pix) return false;

    colorSpace()->toQColor(pix, c, opacity);

    return true;
}


bool KisPaintDevice::pixel(qint32 x, qint32 y, KoColor * kc)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1);

    quint8 *pix = iter.rawData();

    if (!pix) return false;

    kc->setColor(pix, m_colorSpace);

    return true;
}

KoColor KisPaintDevice::colorAt(qint32 x, qint32 y)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1);
    return KoColor(iter.rawData(), m_colorSpace);
}

bool KisPaintDevice::setPixel(qint32 x, qint32 y, const QColor& c, quint8  opacity)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1);

    colorSpace()->fromQColor(c, opacity, iter.rawData());

    return true;
}

bool KisPaintDevice::setPixel(qint32 x, qint32 y, const KoColor& kc)
{
    quint8 * pix;
    if (kc.colorSpace() != m_colorSpace) {
        KoColor kc2 (kc, m_colorSpace);
        pix = kc2.data();
    }
    else {
        pix = kc.data();
    }

    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1);
    memcpy(iter.rawData(), pix, m_colorSpace->pixelSize());

    return true;
}


qint32 KisPaintDevice::numContiguousColumns(qint32 x, qint32 minY, qint32 maxY) const
{
    return m_datamanager->numContiguousColumns(x - m_x, minY - m_y, maxY - m_y);
}

qint32 KisPaintDevice::numContiguousRows(qint32 y, qint32 minX, qint32 maxX) const
{
    return m_datamanager->numContiguousRows(y - m_y, minX - m_x, maxX - m_x);
}

qint32 KisPaintDevice::rowStride(qint32 x, qint32 y) const
{
    return m_datamanager->rowStride(x - m_x, y - m_y);
}

void KisPaintDevice::setX(qint32 x)
{
    m_x = x;
    if(m_selection && m_selection.data() != this)
        m_selection->setX(x);
}

void KisPaintDevice::setY(qint32 y)
{
    m_y = y;
    if(m_selection && m_selection.data() != this)
        m_selection->setY(y);
}


void KisPaintDevice::readBytes(quint8 * data, qint32 x, qint32 y, qint32 w, qint32 h)
{
    m_datamanager->readBytes(data, x - m_x, y - m_y, w, h);
}

void KisPaintDevice::writeBytes(const quint8 * data, qint32 x, qint32 y, qint32 w, qint32 h)
{
    m_datamanager->writeBytes( data, x - m_x, y - m_y, w, h);
}


KisDataManagerSP KisPaintDevice::dataManager() const
{
    return m_datamanager;
}

KisExifInfo* KisPaintDevice::exifInfo()
{
    if(!m_exifInfo)
        m_exifInfo = new KisExifInfo();
    return m_exifInfo;
}

void KisPaintDevice::runBackgroundFilters()
{
    QRect rc = exactBounds();
    if (!m_longRunningFilters.isEmpty()) {
        QList<KisFilter*>::iterator it;
        QList<KisFilter*>::iterator end = m_longRunningFilters.end();
        for (it = m_longRunningFilters.begin(); it != end; ++it) {
            (*it)->process(this, rc, 0);
        }
    }
    if (m_parentLayer) m_parentLayer->setDirty(rc);
}

#include "kis_paint_device.moc"
