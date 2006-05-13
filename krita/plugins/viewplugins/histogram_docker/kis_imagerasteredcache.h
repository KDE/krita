/*
 * This file is part of the KDE project
 *
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#ifndef _KIS_IMAGE_RASTERED_CACHE_H_
#define _KIS_IMAGE_RASTERED_CACHE_H_

#include <QObject>
#include <q3valuevector.h>
#include <q3valuelist.h>
#include <QTimer>

#include <kis_paint_device.h>

class KisView;

class KisImageRasteredCache : public QObject {
Q_OBJECT

public:
    class Observer {
    public:
        virtual Observer* createNew(int x, int y, int w, int h) = 0;
        virtual void regionUpdated(KisPaintDeviceSP dev) = 0;
        virtual ~Observer() {}
    };

    KisImageRasteredCache(KisView* view, Observer* o);
    virtual ~KisImageRasteredCache();

signals:
    void cacheUpdated();

private slots:
    void imageUpdated(QRect rc);
    void imageSizeChanged(qint32 w, qint32 h);
    void timeOut();

private:
    class Element {
    public:
        Element(Observer* o) : observer(o), valid(true) {}
        Observer* observer;
        bool valid;
    };
    typedef Q3ValueVector< Q3ValueVector<Element*> > Raster;
    typedef Q3ValueList<Element*> Queue;

    void cleanUpElements();

    Observer* m_observer;
    Raster m_raster;
    Queue m_queue;
    QTimer m_timer;
    int m_timeOutMSec;
    int m_rasterSize;
    int m_width, m_height;
    KisView * m_view;
    bool m_busy;

    KisPaintDeviceSP m_imageProjection;
};

#endif // _KIS_IMAGE_RASTERED_CACHE_H_
