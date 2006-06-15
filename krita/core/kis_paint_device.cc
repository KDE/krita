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
#include <qrect.h>
#include <qwmatrix.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <qvaluelist.h>
#include <qtimer.h>

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
#include "kis_random_accessor.h"
#include "kis_random_sub_accessor.h"
#include "kis_profile.h"
#include "kis_color.h"
#include "kis_integer_maths.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_selection.h"
#include "kis_layer.h"
#include "kis_paint_device_iface.h"
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
                       KisDataManagerSP beforeData, KisColorSpace * beforeColorSpace,
                       KisDataManagerSP afterData, KisColorSpace * afterColorSpace
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
        KisColorSpace * m_beforeColorSpace;

        KisDataManagerSP m_afterData;
        KisColorSpace * m_afterColorSpace;
    };

}

KisPaintDevice::KisPaintDevice(KisColorSpace * colorSpace, const char * name) :
        QObject(0, name), KShared(), m_exifInfo(0)
{
    if (colorSpace == 0) {
        kdWarning(41001) << "Cannot create paint device without colorstrategy!\n";
        return;
    }
    m_longRunningFilterTimer = 0;
    m_dcop = 0;
    
    m_x = 0;
    m_y = 0;

    m_pixelSize = colorSpace->pixelSize();
    m_nChannels = colorSpace->nChannels();

    Q_UINT8* defPixel = new Q_UINT8 [ m_pixelSize ];
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

    m_longRunningFilters = colorSpace->createBackgroundFilters();
    if (!m_longRunningFilters.isEmpty()) {
        m_longRunningFilterTimer = new QTimer(this);
        connect(m_longRunningFilterTimer, SIGNAL(timeout()), this, SLOT(runBackgroundFilters()));
        m_longRunningFilterTimer->start(800);
    }
}

KisPaintDevice::KisPaintDevice(KisLayer *parent, KisColorSpace * colorSpace, const char * name) :
        QObject(0, name), KShared(), m_exifInfo(0)
{
    
    m_longRunningFilterTimer = 0;
    m_dcop = 0;

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

    Q_ASSERT( m_colorSpace );
    
    m_pixelSize = m_colorSpace->pixelSize();
    m_nChannels = m_colorSpace->nChannels();

    Q_UINT8* defPixel = new Q_UINT8[ m_pixelSize ];
    m_colorSpace->fromQColor(Qt::black, OPACITY_TRANSPARENT, defPixel);

    m_datamanager = new KisDataManager(m_pixelSize, defPixel);
    delete [] defPixel;
    Q_CHECK_PTR(m_datamanager);
    m_extentIsValid = true;

    
    m_longRunningFilters = m_colorSpace->createBackgroundFilters();
    if (!m_longRunningFilters.isEmpty()) {
        m_longRunningFilterTimer = new QTimer(this);
        connect(m_longRunningFilterTimer, SIGNAL(timeout()), this, SLOT(runBackgroundFilters()));
        m_longRunningFilterTimer->start(800);
    }
}

 
KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs) : QObject(), KShared(rhs)
{
    if (this != &rhs) {
        m_longRunningFilterTimer = 0;
        m_parentLayer = 0;
        m_dcop = rhs.m_dcop;
        if (rhs.m_datamanager) {
            m_datamanager = new KisDataManager(*rhs.m_datamanager);
            Q_CHECK_PTR(m_datamanager);
        }
        else {
            kdWarning() << "rhs " << rhs.name() << " has no datamanager\n";
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
    delete m_dcop;
    delete m_longRunningFilterTimer;
    QValueList<KisFilter*>::iterator it;
    QValueList<KisFilter*>::iterator end = m_longRunningFilters.end();
    for (it = m_longRunningFilters.begin(); it != end; ++it) {
        KisFilter * f = (*it);
        delete f;
    }
    m_longRunningFilters.clear();
    //delete m_exifInfo;
}

DCOPObject *KisPaintDevice::dcopObject()
{
    if (!m_dcop) {
        m_dcop = new KisPaintDeviceIface(this);
        Q_CHECK_PTR(m_dcop);
    }
    return m_dcop;
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


void KisPaintDevice::move(Q_INT32 x, Q_INT32 y)
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

    emit positionChanged(this);
}

void KisPaintDevice::move(const QPoint& pt)
{
    move(pt.x(), pt.y());
}

KNamedCommand * KisPaintDevice::moveCommand(Q_INT32 x, Q_INT32 y)
{
    KNamedCommand * cmd = new MoveCommand(this, QPoint(m_x, m_y), QPoint(x, y));
    Q_CHECK_PTR(cmd);
    cmd->execute();
    return cmd;
}

void KisPaintDevice::extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const
{
    m_datamanager->extent(x, y, w, h);
    kdDebug() << x << " " << y << " ;; " << m_x << endl;
    x += m_x;
    y += m_y;
}

QRect KisPaintDevice::extent() const
{
    Q_INT32 x, y, w, h;
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

void KisPaintDevice::exactBounds(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const
{
    QRect r = exactBounds();
    x = r.x();
    y = r.y();
    w = r.width();
    h = r.height();
}

QRect KisPaintDevice::exactBoundsOldMethod() const
{
    Q_INT32 x, y, w, h, boundX, boundY, boundW, boundH;
    extent(x, y, w, h);

    extent(boundX, boundY, boundW, boundH);

    const Q_UINT8* defaultPixel = m_datamanager->defaultPixel();

    bool found = false;

    for (Q_INT32 y2 = y; y2 < y + h ; ++y2) {
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

    for (Q_INT32 y2 = y + h; y2 > y ; --y2) {
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

    for (Q_INT32 x2 = x; x2 < x + w ; ++x2) {
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
    for (Q_INT32 x2 = x + w; x2 > x ; --x2) {
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

QRect KisPaintDevice::exactBoundsImprovedOldMethod() const
{
    // Solution n°2
    Q_INT32  x, y, w, h, boundX2, boundY2, boundW2, boundH2;
    extent(x, y, w, h);
    extent(boundX2, boundY2, boundW2, boundH2);
    
    const Q_UINT8* defaultPixel = m_datamanager->defaultPixel();
    bool found = false;
    {
        KisHLineIterator it = const_cast<KisPaintDevice *>(this)->createHLineIterator(x, y, w, false);
        for (Q_INT32 y2 = y; y2 < y + h ; ++y2) {
            while (!it.isDone() && found == false) {
                if (memcmp(it.rawData(), defaultPixel, m_pixelSize) != 0) {
                    boundY2 = y2;
                    found = true;
                    break;
                }
                ++it;
            }
            if (found) break;
            it.nextRow();
        }
    }

    found = false;

    for (Q_INT32 y2 = y + h; y2 > y ; --y2) {
        KisHLineIterator it = const_cast<KisPaintDevice *>(this)->createHLineIterator(x, y2, w, false);
        while (!it.isDone() && found == false) {
            if (memcmp(it.rawData(), defaultPixel, m_pixelSize) != 0) {
                boundH2 = y2 - boundY2 + 1;
                found = true;
                break;
            }
            ++it;
        }
        if (found) break;
    }
    found = false;

    {
        KisVLineIterator it = const_cast<KisPaintDevice *>(this)->createVLineIterator(x, boundY2, boundH2, false);
        for (Q_INT32 x2 = x; x2 < x + w ; ++x2) {
            while (!it.isDone() && found == false) {
                if (memcmp(it.rawData(), defaultPixel, m_pixelSize) != 0) {
                    boundX2 = x2;
                    found = true;
                    break;
                }
                ++it;
            }
            if (found) break;
            it.nextCol();
        }
    }

    found = false;

    // Look for right edge )
    {
        for (Q_INT32 x2 = x + w; x2 > x ; --x2) {
            KisVLineIterator it = const_cast<KisPaintDevice *>(this)->createVLineIterator(/*x + w*/ x2, boundY2, boundH2, false);
            while (!it.isDone() && found == false) {
                if (memcmp(it.rawData(), defaultPixel, m_pixelSize) != 0) {
                    boundW2 = x2 - boundX2 + 1; // XXX: I commented this
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
    }
    return QRect(boundX2, boundY2, boundW2, boundH2);
}


QRect KisPaintDevice::exactBounds() const
{
    QRect r1 = exactBoundsOldMethod();
    QRect r2 = exactBoundsImprovedOldMethod();
    if(r1 != r2)
    {
        kdDebug() << "EXACTBOUNDSERROR : " << r1 << " " << r2 << endl;
    }
    return r2;
}

void KisPaintDevice::crop(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
     m_datamanager->setExtent(x - m_x, y - m_y, w, h);
}


void KisPaintDevice::crop(QRect r)
{
    r.moveBy(-m_x, -m_y); m_datamanager->setExtent(r);
}

void KisPaintDevice::clear()
{
    m_datamanager->clear();
}

void KisPaintDevice::fill(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const Q_UINT8 *fillPixel)
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

    for (Q_INT32 y = r.top(); y <= r.bottom(); ++y) {
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


    Q_INT32 y1, y2;
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

void KisPaintDevice::convertTo(KisColorSpace * dstColorSpace, Q_INT32 renderingIntent)
{
    kdDebug(41004) << "Converting " << name() << " to " << dstColorSpace->id().id() << " from "
              << m_colorSpace->id().id() << "\n";
    if ( (colorSpace()->id() == dstColorSpace->id()) )
    {
        return;
    }

    KisPaintDevice dst(dstColorSpace);
    dst.setX(getX());
    dst.setY(getY());

    Q_INT32 x, y, w, h;
    extent(x, y, w, h);

    for (Q_INT32 row = y; row < y + h; ++row) {

        Q_INT32 column = x;
        Q_INT32 columnsRemaining = w;

        while (columnsRemaining > 0) {

            Q_INT32 numContiguousDstColumns = dst.numContiguousColumns(column, row, row);
            Q_INT32 numContiguousSrcColumns = numContiguousColumns(column, row, row);

            Q_INT32 columns = QMIN(numContiguousDstColumns, numContiguousSrcColumns);
            columns = QMIN(columns, columnsRemaining);

            //const Q_UINT8 *srcData = pixel(column, row);
            //Q_UINT8 *dstData = dst.writablePixel(column, row);
            KisHLineIteratorPixel srcIt = createHLineIterator(column, row, columns, false);
            KisHLineIteratorPixel dstIt = dst.createHLineIterator(column, row, columns, true);

            const Q_UINT8 *srcData = srcIt.rawData();
            Q_UINT8 *dstData = dstIt.rawData();


            m_colorSpace->convertPixelsTo(srcData, dstData, dstColorSpace, columns, renderingIntent);

            column += columns;
            columnsRemaining -= columns;
        }
    }

    KisDataManagerSP oldData = m_datamanager;
    KisColorSpace *oldColorSpace = m_colorSpace;

    setData(dst.m_datamanager, dstColorSpace);

    if (undoAdapter() && undoAdapter()->undo()) {
        undoAdapter()->addCommand(new KisConvertLayerTypeCmd(undoAdapter(), this, oldData, oldColorSpace, m_datamanager, m_colorSpace));
    }
}

void KisPaintDevice::setProfile(KisProfile * profile)
{
    if (profile == 0) return;

    KisColorSpace * dstSpace =
            KisMetaRegistry::instance()->csRegistry()->getColorSpace( colorSpace()->id(),
                                                                      profile);
    if (dstSpace)
        m_colorSpace = dstSpace;

}

void KisPaintDevice::setData(KisDataManagerSP data, KisColorSpace * colorSpace)
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
                                           Q_INT32 offsetX, Q_INT32 offsetY)
{
    QImage img = image;

    // Krita is little-endian inside.
    if (img.bitOrder() == QImage::LittleEndian) {
	img = img.convertBitOrder(QImage::BigEndian);
    }
    kdDebug() << k_funcinfo << img.bitOrder()<< endl;
    // Krita likes bgra (convertDepth returns *this is the img is alread 32 bits)
    img = img.convertDepth( 32 );
#if 0
    // XXX: Apply import profile
    if (colorSpace() == KisMetaRegistry::instance()->csRegistry() ->getColorSpace(KisID("RGBA",""),"")) {
        writeBytes(img.bits(), 0, 0, img.width(), img.height());
    }
    else {
#endif
        Q_UINT8 * dstData = new Q_UINT8[img.width() * img.height() * pixelSize()];
        KisMetaRegistry::instance()->csRegistry()
                ->getColorSpace(KisID("RGBA",""),srcProfileName)->
                        convertPixelsTo(img.bits(), dstData, colorSpace(), img.width() * img.height());
        writeBytes(dstData, offsetX, offsetY, img.width(), img.height());
//    }
}

QImage KisPaintDevice::convertToQImage(KisProfile *  dstProfile, float exposure)
{
    Q_INT32 x1;
    Q_INT32 y1;
    Q_INT32 w;
    Q_INT32 h;

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
QImage KisPaintDevice::convertToQImage(KisProfile *  dstProfile, Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h, float exposure)
{
    if (w < 0)
        return QImage();

    if (h < 0)
        return QImage();

    Q_UINT8 * data = new Q_UINT8 [w * h * m_pixelSize];
    Q_CHECK_PTR(data);

    // XXX: Is this really faster than converting line by line and building the QImage directly?
    //      This copies potentially a lot of data.
    readBytes(data, x1, y1, w, h);
    QImage image = colorSpace()->convertToQImage(data, w, h, dstProfile, INTENT_PERCEPTUAL, exposure);
    delete[] data;

    return image;
}

KisPaintDeviceSP KisPaintDevice::createThumbnailDevice(Q_INT32 w, Q_INT32 h)
{
    KisPaintDeviceSP thumbnail = new KisPaintDevice(colorSpace(), "thumbnail");
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
        h = Q_INT32(double(srcw) / w * h);
    }
    if (h > srch)
    {
        h = srch;
        w = Q_INT32(double(srch) / h * w);
    }

    if (srcw > srch)
        h = Q_INT32(double(srch) / srcw * w);
    else if (srch > srcw)
        w = Q_INT32(double(srcw) / srch * h);

    for (Q_INT32 y=0; y < h; ++y) {
        Q_INT32 iY = (y * srch ) / h;
        for (Q_INT32 x=0; x < w; ++x) {
            Q_INT32 iX = (x * srcw ) / w;
            thumbnail->setPixel(x, y, colorAt(iX, iY));
        }
    }

    return thumbnail;

}


QImage KisPaintDevice::createThumbnail(Q_INT32 w, Q_INT32 h)
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
        h = Q_INT32(double(srcw) / w * h);
    }
    if (h > srch)
    {
        h = srch;
        w = Q_INT32(double(srch) / h * w);
    }

    if (srcw > srch)
        h = Q_INT32(double(srch) / srcw * w);
    else if (srch > srcw)
        w = Q_INT32(double(srcw) / srch * h);

    QColor c;
    Q_UINT8 opacity;
    QImage img(w,h,32);

    for (Q_INT32 y=0; y < h; ++y) {
        Q_INT32 iY = (y * srch ) / h;
        for (Q_INT32 x=0; x < w; ++x) {
            Q_INT32 iX = (x * srcw ) / w;
            pixel(iX, iY, &c, &opacity);
            const QRgb rgb = c.rgb();
            img.setPixel(x, y, qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), opacity));
        }
    }

    return img;
}

KisRectIteratorPixel KisPaintDevice::createRectIterator(Q_INT32 left, Q_INT32 top, Q_INT32 w, Q_INT32 h, bool writable)
{
    if(hasSelection())
        return KisRectIteratorPixel(this, m_datamanager, m_selection->m_datamanager, left, top, w, h, m_x, m_y, writable);
    else
        return KisRectIteratorPixel(this, m_datamanager, NULL, left, top, w, h, m_x, m_y, writable);
}

KisHLineIteratorPixel  KisPaintDevice::createHLineIterator(Q_INT32 x, Q_INT32 y, Q_INT32 w, bool writable)
{
    if(hasSelection())
        return KisHLineIteratorPixel(this, m_datamanager, m_selection->m_datamanager, x, y, w, m_x, m_y, writable);
    else
        return KisHLineIteratorPixel(this, m_datamanager, NULL, x, y, w, m_x, m_y, writable);
}

KisVLineIteratorPixel  KisPaintDevice::createVLineIterator(Q_INT32 x, Q_INT32 y, Q_INT32 h, bool writable)
{
    if(hasSelection())
        return KisVLineIteratorPixel(this, m_datamanager, m_selection->m_datamanager, x, y, h, m_x, m_y, writable);
    else
        return KisVLineIteratorPixel(this, m_datamanager, NULL, x, y, h, m_x, m_y, writable);

}

KisRandomAccessorPixel KisPaintDevice::createRandomAccessor(Q_INT32 x, Q_INT32 y, bool writable) {
    if(hasSelection())
        return KisRandomAccessorPixel(m_datamanager, m_selection->m_datamanager, x, y, m_x, m_y, writable);
    else
        return KisRandomAccessorPixel(m_datamanager, NULL, x, y, m_x, m_y, writable);
}

KisRandomSubAccessorPixel KisPaintDevice::createRandomSubAccessor()
{
    return KisRandomSubAccessorPixel(this);
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
        m_selection = new KisSelection(this);
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

    KisPainter painter(this->selection().data());
    QRect r = selection->selectedExactRect();
    painter.bitBlt(r.x(), r.y(), COMPOSITE_OVER, selection.data(), r.x(), r.y(), r.width(), r.height());
    painter.end();
}

void KisPaintDevice::subtractSelection(KisSelectionSP selection) {
    KisPainter painter(this->selection().data());
    selection->invert();

    QRect r = selection->selectedExactRect();
    painter.bitBlt(r.x(), r.y(), COMPOSITE_ERASE, selection.data(), r.x(), r.y(), r.width(), r.height());
    
    selection->invert();
    painter.end();
}

void KisPaintDevice::clearSelection()
{
    if (!hasSelection()) return;

    QRect r = m_selection->selectedExactRect();

    if (r.isValid()) {

        for (Q_INT32 y = 0; y < r.height(); y++) {

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

    for (Q_INT32 y = r.top(); y <= r.bottom(); ++y) {

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
    else return 0;
}

bool KisPaintDevice::pixel(Q_INT32 x, Q_INT32 y, QColor *c, Q_UINT8 *opacity)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, false);

    Q_UINT8 *pix = iter.rawData();

    if (!pix) return false;

    colorSpace()->toQColor(pix, c, opacity);

    return true;
}


bool KisPaintDevice::pixel(Q_INT32 x, Q_INT32 y, KisColor * kc)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, false);

    Q_UINT8 *pix = iter.rawData();

    if (!pix) return false;

    kc->setColor(pix, m_colorSpace);

    return true;
}

KisColor KisPaintDevice::colorAt(Q_INT32 x, Q_INT32 y)
{
    //return KisColor(m_datamanager->pixel(x - m_x, y - m_y), m_colorSpace);
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, true);
    return KisColor(iter.rawData(), m_colorSpace);
}

bool KisPaintDevice::setPixel(Q_INT32 x, Q_INT32 y, const QColor& c, Q_UINT8  opacity)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, true);

    colorSpace()->fromQColor(c, opacity, iter.rawData());

    return true;
}

bool KisPaintDevice::setPixel(Q_INT32 x, Q_INT32 y, const KisColor& kc)
{
    Q_UINT8 * pix;
    if (kc.colorSpace() != m_colorSpace) {
        KisColor kc2 (kc, m_colorSpace);
        pix = kc2.data();
    }
    else {
        pix = kc.data();
    }

    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, true);
    memcpy(iter.rawData(), pix, m_colorSpace->pixelSize());

    return true;
}


Q_INT32 KisPaintDevice::numContiguousColumns(Q_INT32 x, Q_INT32 minY, Q_INT32 maxY)
{
    return m_datamanager->numContiguousColumns(x - m_x, minY - m_y, maxY - m_y);
}

Q_INT32 KisPaintDevice::numContiguousRows(Q_INT32 y, Q_INT32 minX, Q_INT32 maxX)
{
    return m_datamanager->numContiguousRows(y - m_y, minX - m_x, maxX - m_x);
}

Q_INT32 KisPaintDevice::rowStride(Q_INT32 x, Q_INT32 y)
{
    return m_datamanager->rowStride(x - m_x, y - m_y);
}

const Q_UINT8* KisPaintDevice::pixel(Q_INT32 x, Q_INT32 y)
{
    return m_datamanager->pixel(x - m_x, y - m_y);
}

Q_UINT8* KisPaintDevice::writablePixel(Q_INT32 x, Q_INT32 y)
{
    return m_datamanager->writablePixel(x - m_x, y - m_y);
}

void KisPaintDevice::setX(Q_INT32 x)
{
    m_x = x;
    if(m_selection)
        m_selection->setX(x);
}

void KisPaintDevice::setY(Q_INT32 y)
{
    m_y = y;
    if(m_selection)
        m_selection->setY(y);
}


void KisPaintDevice::readBytes(Q_UINT8 * data, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    m_datamanager->readBytes(data, x - m_x, y - m_y, w, h);
}

void KisPaintDevice::writeBytes(const Q_UINT8 * data, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
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
    QRect rc = extent();
    if (!m_longRunningFilters.isEmpty()) {
        QValueList<KisFilter*>::iterator it;
        QValueList<KisFilter*>::iterator end = m_longRunningFilters.end();
        for (it = m_longRunningFilters.begin(); it != end; ++it) {
            (*it)->process(this, this, 0, rc);
        }
    }
    if (m_parentLayer) m_parentLayer->setDirty(rc);
}

#include "kis_paint_device.moc"
