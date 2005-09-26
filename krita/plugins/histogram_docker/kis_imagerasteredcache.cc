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

#include <cmath>

#include <kdebug.h>

#include <kis_doc.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>

#include "kis_imagerasteredcache.h"

KisImageRasteredCache::KisImageRasteredCache(KisView* view, Observer* o) {
    m_rasterSize = 64;
    m_timeOutMSec = 125;

    KisImageSP img = view -> getCanvasSubject() -> currentImg();

    if (!img) {
        kdDebug() << "No image for rasteredcache" << endl;
        return;
    }

    int h = img -> height();
    int w = img -> width();
    m_width = static_cast<int>(ceil(float(w) / float(m_rasterSize)));
    m_height = static_cast<int>(ceil(float(h) / float(m_rasterSize)));

    m_raster.resize(m_width);

    int rasterX = 0;

    for (int i = 0; i < m_width *  m_rasterSize; i += m_rasterSize) {
        int rasterY = 0;

        m_raster.at(rasterX).resize(m_height + 1);

        for (int j = 0; j < m_height * m_rasterSize; j += m_rasterSize) {
            Element* e = new Element(o -> createNew(i, j, m_rasterSize, m_rasterSize));
            m_raster.at(rasterX).at(rasterY) = e;
            rasterY++;
        }
        rasterX++;
    }

    imageUpdated(img, QRect(0,0, img -> width(), img -> height()));

    connect(img, SIGNAL(sigImageUpdated(KisImageSP, const QRect&)),
            this, SLOT(imageUpdated(KisImageSP, const QRect&)));
    connect(&m_timer, SIGNAL(timeout()),
             this, SLOT(timeOut()));
}

void KisImageRasteredCache::imageUpdated(KisImageSP image, const QRect& rc) {
    m_dev = image -> mergedImage(); // XXX use img -> projection or so?
    QRect r(0, 0, m_width * m_rasterSize, m_height * m_rasterSize);
    r &= rc;
    r = r.normalize();

    int x = static_cast<int>(r.x() / m_rasterSize);
    int y = static_cast<int>(r.y() / m_rasterSize);
    int x2 = static_cast<int>(ceil(float(r.x() + r.width()) / float(m_rasterSize)));
    int y2 = static_cast<int>(ceil(float(r.y() + r.height()) / float(m_rasterSize)));

    for ( ; x < x2; x++) {
        for (int i = y; i < y2; i++) {
            Element* e = m_raster.at(x).at(i);
            if (e -> valid) {
                e -> valid = false;
                m_queue.push_back(e);
            }
        }
    }

    // If the timer is already started, this resets it. That way, we update always
    // m_timeOutMSec milliseconds after the lastly monitored activity
    m_timer.start(m_timeOutMSec, true); // true -> singleshot
}

void KisImageRasteredCache::timeOut() {
    while(!m_queue.isEmpty()) {
        m_queue.front() -> observer -> regionUpdated(m_dev);
        m_queue.front() -> valid = true;
        m_queue.pop_front();
    }

    emit cacheUpdated();
}

#include "kis_imagerasteredcache.moc"
