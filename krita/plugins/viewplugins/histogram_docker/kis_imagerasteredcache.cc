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

#include <QApplication>

#include <kdebug.h>

#include <kis_doc.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>

#include "kis_imagerasteredcache.h"

KisImageRasteredCache::KisImageRasteredCache(KisView* view, Observer* o) 
    : m_observer(o->createNew(0, 0, 0, 0)), m_view(view)
{
    m_busy = false;
    m_imageProjection = 0;
    m_rasterSize = 64;
    m_timeOutMSec = 1000;

    KisImageSP img = view->canvasSubject()->currentImg();

    if (!img) {
        return;
    }

    imageSizeChanged(img->width(), img->height());

    connect(img.data(), SIGNAL(sigImageUpdated(QRect)),
            this, SLOT(imageUpdated(QRect)));
    connect(img.data(), SIGNAL(sigSizeChanged(qint32, qint32)),
            this, SLOT(imageSizeChanged(qint32, qint32)));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeOut()));
}

KisImageRasteredCache::~KisImageRasteredCache() {
    cleanUpElements();
}

void KisImageRasteredCache::imageUpdated(QRect rc) {

    if (rc.isValid()) {
        QRect r(0, 0, m_width * m_rasterSize, m_height * m_rasterSize);
        r &= rc;

        uint x = static_cast<int>(r.x() / m_rasterSize);
        uint y = static_cast<int>(r.y() / m_rasterSize);
        uint x2 = static_cast<int>(ceil(float(r.x() + r.width()) / float(m_rasterSize)));
        uint y2 = static_cast<int>(ceil(float(r.y() + r.height()) / float(m_rasterSize)));

        if (!m_raster.empty()) {
            for ( ; x < x2; x++) {
                for (uint i = y; i < y2; i++) {
                    if (x < m_raster.size()) {
                        if (i < m_raster.at(x).size()) {
                            Element* e = m_raster.at(x).at(i);
                            if (e && e->valid) {
                                e->valid = false;
                                m_queue.push_back(e);
                            }
                        }
                    }
                }
            }
        }
    }

    if (!m_busy) {
        // If the timer is already started, this resets it. That way, we update always
        // m_timeOutMSec milliseconds after the lastly monitored activity
        m_timer.start(m_timeOutMSec, true); // true->singleshot
    }
}

void KisImageRasteredCache::imageSizeChanged(qint32 w, qint32 h) {

    KisImageSP image = m_view->canvasSubject()->currentImg();

    cleanUpElements();
    m_busy = false;

    m_width = static_cast<int>(ceil(float(w) / float(m_rasterSize)));
    m_height = static_cast<int>(ceil(float(h) / float(m_rasterSize)));

    m_raster.resize(m_width);

    int rasterX = 0;

    for (int i = 0; i < m_width *  m_rasterSize; i += m_rasterSize) {
        int rasterY = 0;

        m_raster.at(rasterX).resize(m_height + 1);

        for (int j = 0; j < m_height * m_rasterSize; j += m_rasterSize) {
            Element* e = new Element(m_observer->createNew(i, j, m_rasterSize, m_rasterSize));
            m_raster.at(rasterX).at(rasterY) = e;
            rasterY++;
        }
        rasterX++;
    }

    imageUpdated(QRect(0,0, image->width(), image->height()));
}

void KisImageRasteredCache::timeOut() {
    m_busy = true;
    KisImageSP img = m_view->canvasSubject()->currentImg();

    // Temporary cache: while we are busy, we won't get the mergeImage time and again.
    if (!m_imageProjection)
        m_imageProjection = img->mergedImage();

    // Pick one element of the cache, and update it
    if (!m_queue.isEmpty()) {
        m_queue.front()->observer->regionUpdated(m_imageProjection);
        m_queue.front()->valid = true;
        m_queue.pop_front();
    }

    // If there are still elements, we need to be called again (this emulates processEvents)
    if (!m_queue.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(timeOut()));
    } else {
        emit cacheUpdated();
        m_imageProjection = 0;
        m_busy = false;
    }
}

void KisImageRasteredCache::cleanUpElements() {
    for (uint i = 0; i < m_raster.count(); i++) {
        for (uint j = 0; j < m_raster.at(i).count(); j++) {
            delete m_raster.at(i).at(j);
        }
        m_raster.at(i).clear();
    }
    m_raster.clear();
    m_queue.clear();
}

#include "kis_imagerasteredcache.moc"
