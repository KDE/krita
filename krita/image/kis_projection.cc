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
#include <QApplication>

#include <kis_debug.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "kis_image.h"
#include "kis_group_layer.h"

class KisProjection::Private
{
public:

    KisImageUpdater* updater;
    KisImageWSP image;
    bool locked;
    int updateRectSize;
    QRect roi; // Region of interest
    bool useRegionOfInterest; // If false, update all dirty bits, if
    // true, update only region of interest.
};

KisProjection::KisProjection(KisImageWSP image)
        : QThread()
        , m_d(new Private)
{
    m_d->image = image;
    m_d->updater = 0;
    updateSettings();
}


KisProjection::~KisProjection()
{
    m_d->updater->deleteLater();
    delete m_d;
}

void KisProjection::stop()
{
    quit();
}

void KisProjection::run()
{
    // The image updater is created in the run() method so it lives in the thread, otherwise
    // startUpdate will be executed in gui thread.
    m_d->updater = new KisImageUpdater();
    connect(this, SIGNAL(sigUpdateProjection(KisNodeSP, QRect)), m_d->updater, SLOT(startUpdate(KisNodeSP, QRect)));
    connect(m_d->updater, SIGNAL(updateDone(QRect)), m_d->image, SLOT(slotProjectionUpdated(QRect)));
    exec(); // start the event loop
}

void KisProjection::lock()
{
    if (m_d->updater) {
        m_d->updater->blockSignals(true);
    }
    blockSignals(true);
}

void KisProjection::unlock()
{
    if (m_d->updater) {
        m_d->updater->blockSignals(false);
    }
    blockSignals(false);
}

void KisProjection::setRegionOfInterest(const QRect & roi)
{
    m_d->roi = roi;
}


void KisProjection::updateProjection(KisNodeSP node, const QRect& rc)
{
    if (!m_d->image)return;

    // The chunks do not run concurrently (there is only one KisImageUpdater and only
    // one event loop), but it is still useful, since intermediate results are passed
    // back to the main thread where the gui can be updated.
    QRect interestingRect = rc.intersected(m_d->image->bounds());

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
}

void KisImageUpdater::startUpdate(KisNodeSP node, const QRect& rc)
{
    update(node, 0, rc);
    emit updateDone(rc);
}

void KisImageUpdater::update(KisNodeSP node, KisNodeSP child, const QRect & rc)
{
    QRect dirtyRect = rc;

    // group layers get special treatment so we can optimize if there
    // are clean adjustmentlayers in the stack
    KisGroupLayer* grouplayer = dynamic_cast<KisGroupLayer*>(node.data());

    if (grouplayer && child) {
        grouplayer->setDirtyNode(child);
    }

    // update the projection of the layer: this may increase the dirty rect
    if (KisLayer* layer = dynamic_cast<KisLayer*>(node.data())) {
        dirtyRect |= layer->updateProjection(dirtyRect);
    }

    if (grouplayer) {
        grouplayer->setDirtyNode(0);
    }

    // go up in the stack, recursively.
    if (node->parent()) {
        update(node->parent(), node, dirtyRect);
    }
}


#include "kis_projection.moc"
