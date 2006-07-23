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

#include <kcommand.h>
#include <klocale.h>
#include <kdebug.h>

#include <KoStore.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_undo_adapter.h"
#include "kis_iterator.h"
#include "kis_iterators_pixel.h"
#include "kis_iteratorpixeltrait.h"
#include "KoColorProfile.h"
#include "KoColor.h"
#include "KoIntegerMaths.h"
#include "KoColorSpaceRegistry.h"
#include "kis_selection.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_datamanager.h"
#include "kis_memento.h"

#include "kis_exif_info.h"

namespace {

    class KisPaintDeviceCommand : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisPaintDeviceCommand(const QString& name, KisPaintDeviceSP paintDevice);
        virtual ~KisPaintDeviceCommand() {}

        virtual void execute() = 0;
        virtual void unexecute() = 0;

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

    class MoveCommand : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        MoveCommand(KisPaintDeviceSP device, const QPoint& oldpos, const QPoint& newpos);
        virtual ~MoveCommand();

        virtual void execute();
        virtual void unexecute();

    private:
        void moveTo(const QPoint& pos);
        void undoOff();
        void undoOn();

    private:
        KisPaintDeviceSP m_device;
        QPoint m_oldPos;
        QPoint m_newPos;
    };

    MoveCommand::MoveCommand(KisPaintDeviceSP device, const QPoint& oldpos, const QPoint& newpos) :
        super(i18n("Move Layer"))
    {
        m_device = device;
        m_oldPos = oldpos;
        m_newPos = newpos;
    }

    MoveCommand::~MoveCommand()
    {
    }

    void MoveCommand::undoOff()
    {
        if (m_device->undoAdapter()) {
            m_device->undoAdapter()->setUndo(false);
        }
    }

    void MoveCommand::undoOn()
    {
        if (m_device->undoAdapter()) {
            m_device->undoAdapter()->setUndo(true);
        }
    }

    void MoveCommand::execute()
    {
        undoOff();
        moveTo(m_newPos);
        undoOn();
    }

    void MoveCommand::unexecute()
    {
        undoOff();
        moveTo(m_oldPos);
        undoOn();
    }

    void MoveCommand::moveTo(const QPoint& pos)
    {
        m_device->move(pos.x(), pos.y());
    }

    class KisConvertLayerTypeCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisConvertLayerTypeCmd(KisUndoAdapter *adapter, KisPaintDeviceSP paintDevice,
                       KisDataManagerSP beforeData, KoColorSpace * beforeColorSpace,
                       KisDataManagerSP afterData, KoColorSpace * afterColorSpace
                ) : super(i18n("Convert Layer Type"))
            {
                m_adapter = adapter;
                m_paintDevice = paintDevice;
                m_beforeData = beforeData;
                m_beforeColorSpace = beforeColorSpace;
                m_afterData = afterData;
                m_afterColorSpace = afterColorSpace;
            }

        virtual ~KisConvertLayerTypeCmd()
            {
            }

    public:
        virtual void execute()
            {
                m_adapter->setUndo(false);
                m_paintDevice->setData(m_afterData, m_afterColorSpace);
                m_adapter->setUndo(true);
            }

        virtual void unexecute()
            {
                m_adapter->setUndo(false);
                m_paintDevice->setData(m_beforeData, m_beforeColorSpace);
                m_adapter->setUndo(true);
            }

    private:
        KisUndoAdapter *m_adapter;

        KisPaintDeviceSP m_paintDevice;

        KisDataManagerSP m_beforeData;
        KoColorSpace * m_beforeColorSpace;

        KisDataManagerSP m_afterData;
        KoColorSpace * m_afterColorSpace;
    };

}

KisPaintDevice::KisPaintDevice(KoColorSpace * colorSpace, QString name) :
        QObject(0), KShared(), m_exifInfo(0)
{
    setObjectName(name);
    if (colorSpace == 0) {
        kWarning(41001) << "Cannot create paint device without colorstrategy!\n";
        return;
    }
    m_longRunningFilterTimer = 0;
    
    m_x = 0;
    m_y = 0;

    m_pixelSize = colorSpace->pixelSize();
    m_nChannels = colorSpace->nChannels();

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

}

KisPaintDevice::KisPaintDevice(KisLayer *parent, KoColorSpace * colorSpace, QString name) :
        QObject(0), KShared(), m_exifInfo(0)
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
    m_nChannels = m_colorSpace->nChannels();

    quint8* defPixel = new quint8[ m_pixelSize ];
    colorSpace->fromQColor(Qt::black, OPACITY_TRANSPARENT, defPixel);

    m_datamanager = new KisDataManager(m_pixelSize, defPixel);
    delete [] defPixel;
    Q_CHECK_PTR(m_datamanager);
    m_extentIsValid = true;

}

 
KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs) : QObject(), KShared(rhs)
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
    //delete m_exifInfo;
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

void KisPaintDevice::setDirty()
{
    if (m_parentLayer) m_parentLayer->setDirty();
}

KisImage *KisPaintDevice::image() const
{
    if (m_parentLayer) {
        return m_parentLayer->image();
    } else {
        return 0;
    }
}


void KisPaintDevice::move(qint32 x, qint32 y)
{
    QRect dirtyRect = extent();

    m_x = x;
    m_y = y;

    dirtyRect |= extent();

    if(m_selection)
    {
        m_selection->setX(x);
        m_selection->setY(y);
    }

    setDirty(dirtyRect);

    emit positionChanged(KisPaintDeviceSP(this));
}

void KisPaintDevice::move(const QPoint& pt)
{
    move(pt.x(), pt.y());
}

KNamedCommand * KisPaintDevice::moveCommand(qint32 x, qint32 y)
{
    KNamedCommand * cmd = new MoveCommand(KisPaintDeviceSP(this), QPoint(m_x, m_y), QPoint(x, y));
    Q_CHECK_PTR(cmd);
    cmd->execute();
    return cmd;
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

    for (qint32 y2 = y; y2 < y + h ; ++y2) {
        KisHLineIterator it = const_cast<KisPaintDevice *>(this)->createHLineIterator(x, y2, w, false);
        while (!it.isDone() && found == false) {
            if (memcmp(it.rawData(), defaultPixel, m_pixelSize) != 0) {
                boundY = y2;
                found = true;
                break;
            }
            ++it;
        }
        if (found) break;
    }

    found = false;

    for (qint32 y2 = y + h; y2 > y ; --y2) {
        KisHLineIterator it = const_cast<KisPaintDevice *>(this)->createHLineIterator(x, y2, w, false);
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

    for (qint32 x2 = x; x2 < x + w ; ++x2) {
        KisVLineIterator it = const_cast<KisPaintDevice *>(this)->createVLineIterator(x2, y, h, false);
        while (!it.isDone() && found == false) {
            if (memcmp(it.rawData(), defaultPixel, m_pixelSize) != 0) {
                boundX = x2;
                found = true;
                break;
            }
            ++it;
        }
        if (found) break;
    }

    found = false;

    // Look for right edge )
    for (qint32 x2 = x + w; x2 > x ; --x2) {
        KisVLineIterator it = const_cast<KisPaintDevice *>(this)->createVLineIterator(x2, y, h, false);
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
        r = selection()->exactBounds();
    }
    else {
        r = exactBounds();
    }

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {
        KisHLineIteratorPixel srcIt = createHLineIterator(r.x(), y, r.width(), false);
        KisHLineIteratorPixel dstIt = createHLineIterator(r.x(), y, r.width(), true);

        dstIt += r.width() - 1;

        while (!srcIt.isDone()) {
            if (srcIt.isSelected()) {
                memcpy(dstIt.rawData(), srcIt.oldRawData(), m_pixelSize);
            }
            ++srcIt;
            --dstIt;

        }
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
        r = selection()->exactBounds();
    }
    else {
        r = exactBounds();
    }


    qint32 y1, y2;
    for (y1 = r.top(), y2 = r.bottom(); y1 <= r.bottom(); ++y1, --y2) {
        KisHLineIteratorPixel itTop = createHLineIterator(r.x(), y1, r.width(), true);
        KisHLineIteratorPixel itBottom = createHLineIterator(r.x(), y2, r.width(), false);
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
    kDebug(41004) << "Converting " << objectName() << " to " << dstColorSpace->id() << " from "
              << m_colorSpace->id() << "\n";
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

            KisHLineIteratorPixel srcIt = createHLineIterator(column, row, columns, false);
            KisHLineIteratorPixel dstIt = dst.createHLineIterator(column, row, columns, true);

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
        undoAdapter()->addCommand(new KisConvertLayerTypeCmd(undoAdapter(), KisPaintDeviceSP(this), oldData, oldColorSpace, m_datamanager, m_colorSpace));
    }
    emit colorSpaceChanged(dstColorSpace);
}

void KisPaintDevice::setProfile(KoColorProfile * profile)
{
    if (profile == 0) return;

    KoColorSpace * dstSpace =
            KisMetaRegistry::instance()->csRegistry()->colorSpace( colorSpace()->id(),
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
    m_nChannels = m_colorSpace->nChannels();

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
    QImage img = image;

    if (img.format() != QImage::Format_ARGB32) {
        img = img.convertToFormat(QImage::Format_ARGB32);
    }
#if 0
    // XXX: Apply import profile
    if (colorSpace() == KisMetaRegistry::instance()->csRegistry() ->colorSpace("RGBA",0)) {
        writeBytes(img.bits(), 0, 0, img.width(), img.height());
    }
    else {
#endif
        quint8 * dstData = new quint8[img.width() * img.height() * pixelSize()];
        KisMetaRegistry::instance()->csRegistry()
                ->colorSpace("RGBA",srcProfileName)->
                        convertPixelsTo(img.bits(), dstData, colorSpace(), img.width() * img.height());
        writeBytes(dstData, offsetX, offsetY, img.width(), img.height());
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

    quint8 * data = new quint8 [w * h * m_pixelSize];
    Q_CHECK_PTR(data);

    // XXX: Is this really faster than converting line by line and building the QImage directly?
    //      This copies potentially a lot of data.
    readBytes(data, x1, y1, w, h);
    QImage image = colorSpace()->convertToQImage(data, w, h, dstProfile, INTENT_PERCEPTUAL, exposure);
    delete[] data;

    return image;
}

KisPaintDeviceSP KisPaintDevice::createThumbnailDevice(qint32 w, qint32 h)
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

    for (qint32 y=0; y < h; ++y) {
        qint32 iY = (y * srch ) / h;
        for (qint32 x=0; x < w; ++x) {
            qint32 iX = (x * srcw ) / w;
            thumbnail->setPixel(x, y, colorAt(iX, iY));
        }
    }

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

KisRectIteratorPixel KisPaintDevice::createRectIterator(qint32 left, qint32 top, qint32 w, qint32 h, bool writable)
{
    if(hasSelection())
        return KisRectIteratorPixel(this, m_datamanager.data(), m_selection->m_datamanager.data(), left, top, w, h, m_x, m_y, writable);
    else
        return KisRectIteratorPixel(this, m_datamanager.data(), NULL, left, top, w, h, m_x, m_y, writable);
}

KisHLineIteratorPixel  KisPaintDevice::createHLineIterator(qint32 x, qint32 y, qint32 w, bool writable)
{
    if(hasSelection())
        return KisHLineIteratorPixel(this, m_datamanager.data(), m_selection->m_datamanager.data(), x, y, w, m_x, m_y, writable);
    else
        return KisHLineIteratorPixel(this, m_datamanager.data(), NULL, x, y, w, m_x, m_y, writable);
}

KisVLineIteratorPixel  KisPaintDevice::createVLineIterator(qint32 x, qint32 y, qint32 h, bool writable)
{
    if(hasSelection())
        return KisVLineIteratorPixel(this, m_datamanager.data(), m_selection->m_datamanager.data(), x, y, h, m_x, m_y, writable);
    else
        return KisVLineIteratorPixel(this, m_datamanager.data(), NULL, x, y, h, m_x, m_y, writable);

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


bool KisPaintDevice::hasSelection()
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

    KisPainter painter(KisPaintDeviceSP(this->selection().data()));
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

        for (qint32 y = 0; y < r.height(); y++) {

            KisHLineIterator devIt = createHLineIterator(r.x(), r.y() + y, r.width(), true);
            KisHLineIterator selectionIt = m_selection->createHLineIterator(r.x(), r.y() + y, r.width(), false);

            while (!devIt.isDone()) {
                // XXX: Optimize by using stretches

                m_colorSpace->applyInverseAlphaU8Mask( devIt.rawData(), selectionIt.rawData(), 1);

                ++devIt;
                ++selectionIt;
            }
        }

        if (m_parentLayer) {
            m_parentLayer->setDirty(r);
        }
    }
}

void KisPaintDevice::applySelectionMask(KisSelectionSP mask)
{
    QRect r = mask->extent();
    crop(r);

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {

        KisHLineIterator pixelIt = createHLineIterator(r.x(), y, r.width(), true);
        KisHLineIterator maskIt = mask->createHLineIterator(r.x(), y, r.width(), false);

        while (!pixelIt.isDone()) {
            // XXX: Optimize by using stretches

            m_colorSpace->applyAlphaU8Mask( pixelIt.rawData(), maskIt.rawData(), 1);

            ++pixelIt;
            ++maskIt;
        }
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
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, false);

    quint8 *pix = iter.rawData();

    if (!pix) return false;

    colorSpace()->toQColor(pix, c, opacity);

    return true;
}


bool KisPaintDevice::pixel(qint32 x, qint32 y, KoColor * kc)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, false);

    quint8 *pix = iter.rawData();

    if (!pix) return false;

    kc->setColor(pix, m_colorSpace);

    return true;
}

KoColor KisPaintDevice::colorAt(qint32 x, qint32 y)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, true);
    return KoColor(iter.rawData(), m_colorSpace);
}

bool KisPaintDevice::setPixel(qint32 x, qint32 y, const QColor& c, quint8  opacity)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, true);

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

    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, true);
    memcpy(iter.rawData(), pix, m_colorSpace->pixelSize());

    return true;
}


qint32 KisPaintDevice::numContiguousColumns(qint32 x, qint32 minY, qint32 maxY)
{
    return m_datamanager->numContiguousColumns(x - m_x, minY - m_y, maxY - m_y);
}

qint32 KisPaintDevice::numContiguousRows(qint32 y, qint32 minX, qint32 maxX)
{
    return m_datamanager->numContiguousRows(y - m_y, minX - m_x, maxX - m_x);
}

qint32 KisPaintDevice::rowStride(qint32 x, qint32 y)
{
    return m_datamanager->rowStride(x - m_x, y - m_y);
}

const quint8* KisPaintDevice::pixel(qint32 x, qint32 y)
{
    return m_datamanager->pixel(x - m_x, y - m_y);
}

quint8* KisPaintDevice::writablePixel(qint32 x, qint32 y)
{
    return m_datamanager->writablePixel(x - m_x, y - m_y);
}

void KisPaintDevice::setX(qint32 x)
{
    m_x = x;
    if(m_selection)
        m_selection->setX(x);
}

void KisPaintDevice::setY(qint32 y)
{
    m_y = y;
    if(m_selection)
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
            (*it)->process(KisPaintDeviceSP(this), KisPaintDeviceSP(this), 0, rc);
        }
    }
    if (m_parentLayer) m_parentLayer->setDirty(rc);
}

#include "kis_paint_device.moc"
