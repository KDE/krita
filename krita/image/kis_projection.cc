/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org
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

#include "kis_projection.h"

#include <QRegion>
#include <QRect>
#include <QThread>
#include <QMutex>

#include <kis_debug.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "kis_image.h"
#include "kis_group_layer.h"
#include "kis_projection_update_strategy.h"

class KisProjection::Private {
    public:

    KisImageUpdater* updater;
    KisImageWSP image;
    bool locked;
    int updateRectSize;
    QRect roi; // Region of interest
    bool useRegionOfInterest; // If false, update all dirty bits, if
    // true, update only region of interest.
    bool useThreading;

};

KisProjection::KisProjection(KisImageWSP image)
        : QThread()
        , m_d(new Private)
{
    m_d->updater = 0;
    m_d->image = image;
    updateSettings();
}


KisProjection::~KisProjection()
{
    exit(); // stop the event loop
    delete m_d->updater;
    delete m_d;
}

void KisProjection::run()
{
    m_d->updater = new KisImageUpdater();
    connect(this, SIGNAL(sigUpdateProjection(KisNodeSP,QRect)), m_d->updater, SLOT(startUpdate(KisNodeSP,QRect)));
//    connect(m_d->updater, SIGNAL(updateDone(QRect)), m_d->image, SLOT(slotProjectionUpdated(QRect)));

    exec(); // start the event loop
}

void KisProjection::lock()
{
    m_d->updater->blockSignals(true);
    blockSignals(true);
}

void KisProjection::unlock()
{
    m_d->updater->blockSignals(false);
    blockSignals(false);
}

void KisProjection::setRegionOfInterest(const QRect & roi)
{
    m_d->roi = roi;
}


void KisProjection::updateProjection(KisNodeSP node, const QRect& rc)
{
    if (!m_d->image)return;

    if (!m_d->useThreading) {
        node->updateStrategy()->setDirty(rc);
        m_d->image->slotProjectionUpdated(rc);
    }

    // The chunks do not run concurrently (there is only one KisImageUpdater and only
    // one event loop), but it is still useful, since intermediate results are passed
    // back to the main thread where the gui can be updated.
    QRect interestingRect;

    if (m_d->useRegionOfInterest) {
        interestingRect = rc.intersected(m_d->roi);
    } else {
        interestingRect = rc;
    }

    int h = interestingRect.height();
    int w = interestingRect.width();
    int x = interestingRect.x();
    int y = interestingRect.y();

    // Note: we're doing columns first, so when we have a small strip left
    // at the bottom, we have as few and as long runs of pixels left
    // as possible.
    if (w <= m_d->updateRectSize && h <= m_d->updateRectSize) {
        emit sigUpdateProjection(node, interestingRect);
        return;
    }
    int wleft = w;
    int col = 0;
    while (wleft > 0) {
        int hleft = h;
        int row = 0;
        while (hleft > 0) {
            QRect rc2(col + x, row + y, qMin(wleft, m_d->updateRectSize), qMin(hleft, m_d->updateRectSize));

            emit sigUpdateProjection(node, rc2);

            hleft -= m_d->updateRectSize;
            row += m_d->updateRectSize;

        }
        wleft -= m_d->updateRectSize;
        col += m_d->updateRectSize;
    }
}

void KisProjection::updateSettings()
{
    KConfigGroup cfg = KGlobal::config()->group("");
    m_d->updateRectSize = cfg.readEntry("updaterectsize", 512);
    m_d->useRegionOfInterest = cfg.readEntry("use_region_of_interest", false);
    m_d->useThreading = cfg.readEntry("use_threading", true);
}


void KisProjection::setRootLayer(KisGroupLayerSP rootLayer)
{
    connect(rootLayer, SIGNAL(settingsUpdated()), this, SLOT(updateSettings()));
}

void KisImageUpdater::startUpdate(KisNodeSP node, const QRect& rc)
{
    /**
     * KisImage::slotProjectionUpdated will be called directly out of
     * updateStategy at the end of an update
     *
     * FIXME: We need to break this circle
     * layer->image->projection->updater->upd.startegy->layer
     */
    node->updateStrategy()->setDirty(rc);
//    emit updateDone(rc);
}

#include "kis_projection.moc"
