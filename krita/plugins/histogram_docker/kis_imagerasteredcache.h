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

#include <qobject.h>
#include <qvaluevector.h>
#include <qvaluelist.h>
#include <qtimer.h>

#include <kis_paint_device_impl.h>

class KisView;

class KisImageRasteredCache : public QObject {
Q_OBJECT

public:
    class Observer {
    public:
        virtual Observer* createNew(int x, int y, int w, int h) = 0;
        virtual void regionUpdated(KisPaintDeviceImplSP dev) = 0;
        virtual ~Observer() {}
    };

    KisImageRasteredCache(KisView* view, Observer* o);

signals:
    void cacheUpdated();

private slots:
    void imageUpdated(KisImageSP image, const QRect& rc);
    void timeOut();

private:
    class Element {
    public:
        Element(Observer* o) : observer(o), valid(true) {}
        Observer* observer;
        bool valid;
    };
    typedef QValueVector< QValueVector<Element*> > Raster;
    typedef QValueList<Element*> Queue;

    Raster m_raster;
    Queue m_queue;
    QTimer m_timer;
    int m_timeOutMSec;
    int m_rasterSize;
    int m_width, m_height;
    KisView * m_view;
};

#endif // _KIS_IMAGE_RASTERED_CACHE_H_
