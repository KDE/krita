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

#include "kis_imagerasteredcache.h"

#include <cmath>

#include <QApplication>

#include <kis_debug.h>

#include <kis_global.h>
#include <kis_types.h>
#include <kis_image.h>
#include <kis_view2.h>

KisImageRasteredCache::KisImageRasteredCache(Observer* o)
        : m_observer(o->createNew(0, 0, 0, 0))
        , m_docker(0)
{
    m_visible = false;
    m_busy = false;
    m_imageProjection = 0;
    m_rasterSize = 64 * 4;
    m_timeOutMSec = 1000;

    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeOut()));

    m_visibilityTimer.setSingleShot(true);
    connect(&m_visibilityTimer, SIGNAL(timeout()), this, SLOT(checkVisibility()));
}

KisImageRasteredCache::~KisImageRasteredCache()
{
    cleanUpElements();
}

void KisImageRasteredCache::setDocker(QDockWidget* docker)
{
    m_docker = docker;
    connect(m_docker, SIGNAL(visibilityChanged(bool)), SLOT(setDockerVisible(bool)));
}

void KisImageRasteredCache::setImage(KisImageWSP image)
{
    m_image = image;
    if (image && image.isValid()) {
        imageSizeChanged(image->width(), image->height());
    }
    m_visible = m_docker->isVisible();
}

void KisImageRasteredCache::setDockerVisible(bool visible)
{
    if (visible) {
        m_visibilityTimer.start(200);
    }
    else {
        m_visibilityTimer.stop();
    }
}

void KisImageRasteredCache::checkVisibility()
{
    if (m_docker->isVisible() && m_image.isValid()) {
        m_visible = true;
        connect(m_image, SIGNAL(sigImageUpdated(QRect)), this, SLOT(imageUpdated(QRect)));
        connect(m_image, SIGNAL(sigSizeChanged(qint32, qint32)),  this, SLOT(imageSizeChanged(qint32, qint32)));
    } else {
        m_visible = false;
        disconnect();
    }
}

void KisImageRasteredCache::imageUpdated(QRect rc)
{
    // Do our but against global warming: don't waste cpu if the histogram isn't visible anyway.
    if (!m_visible) return;
    if (!m_image.isValid()) return;

    QRect r(0, 0, m_width * m_rasterSize, m_height * m_rasterSize);
    r &= rc;

    int x = static_cast<int>(r.x() / m_rasterSize);
    int y = static_cast<int>(r.y() / m_rasterSize);
    int x2 = static_cast<int>(ceil(float(r.x() + r.width()) / float(m_rasterSize)));
    int y2 = static_cast<int>(ceil(float(r.y() + r.height()) / float(m_rasterSize)));

    if (!m_raster.empty()) {
        for (; x < x2; x++) {
            for (int i = y; i < y2; i++) {
                if (x < m_raster.size()) {
                    if (i < m_raster[x].size()) {
                        Element* e = m_raster[x][i];
                        if (e && e->valid) {
                            e->valid = false;
                            m_queue.push_back(e);
                        }
                    }
                }
            }
        }
    }

    if (!m_busy) {
        // If the timer is already started, this resets it. That way, we update always
        // m_timeOutMSec milliseconds after the lastly monitored activity
        m_timer.start(m_timeOutMSec);
    }
}

void KisImageRasteredCache::imageSizeChanged(qint32 w, qint32 h)
{
    cleanUpElements();
    m_busy = false;

    m_width = static_cast<int>(ceil(float(w) / float(m_rasterSize)));
    m_height = static_cast<int>(ceil(float(h) / float(m_rasterSize)));

    m_raster.resize(m_width);

    int rasterX = 0;

    for (int i = 0; i < m_width *  m_rasterSize; i += m_rasterSize) {
        int rasterY = 0;

        m_raster[rasterX].resize(m_height + 1);

        for (int j = 0; j < m_height * m_rasterSize; j += m_rasterSize) {
            Element* e = new Element(m_observer->createNew(i, j, m_rasterSize, m_rasterSize));
            m_raster[rasterX][rasterY] = e;
            rasterY++;
        }
        rasterX++;
    }

    imageUpdated(QRect(0, 0, m_image->width(), m_image->height()));
}

void KisImageRasteredCache::timeOut()
{
    m_busy = true;
    if (!m_image) {
        return;
    }
    // Temporary cache: while we are busy, we won't get the mergeImage time and again.
    if (!m_imageProjection) {
        m_imageProjection = m_image->mergedImage();
    }
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

void KisImageRasteredCache::cleanUpElements()
{
    for (int i = 0; i < m_raster.count(); i++) {
        for (int j = 0; j < m_raster[i].count(); j++) {
            delete m_raster[i][j];
        }
        m_raster[i].clear();
    }
    m_raster.clear();
    m_queue.clear();
}

#include "kis_imagerasteredcache.moc"
