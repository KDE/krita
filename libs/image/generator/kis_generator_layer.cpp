/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include <QMutex>
#include <QMutexLocker>

#include "kis_generator_layer.h"

#include <klocalizedstring.h>
#include "kis_debug.h"

#include <KoIcon.h>
#include <kis_icon.h>
#include "kis_selection.h"
#include "filter/kis_filter_configuration.h"
#include "kis_processing_information.h"
#include <kis_processing_visitor.h>
#include "generator/kis_generator_registry.h"
#include "generator/kis_generator.h"
#include "kis_node_visitor.h"
#include "kis_thread_safe_signal_compressor.h"
#include "kis_recalculate_generator_layer_job.h"
#include "kis_generator_stroke_strategy.h"

#define UPDATE_DELAY 100 /*ms */

struct Q_DECL_HIDDEN KisGeneratorLayer::Private
{
    Private()
        : updateSignalCompressor(UPDATE_DELAY, KisSignalCompressor::FIRST_INACTIVE)
    {
    }

    KisThreadSafeSignalCompressor updateSignalCompressor;
    QRect preparedRect;
    KisFilterConfigurationSP preparedForFilter;
    QWeakPointer<bool> updateCookie;
    QMutex mutex;
};


KisGeneratorLayer::KisGeneratorLayer(KisImageWSP image,
                                     const QString &name,
                                     KisFilterConfigurationSP kfc,
                                     KisSelectionSP selection)
    : KisSelectionBasedLayer(image, name, selection, kfc, true),
      m_d(new Private)
{
    connect(&m_d->updateSignalCompressor, SIGNAL(timeout()), SLOT(slotDelayedStaticUpdate()));
}

KisGeneratorLayer::KisGeneratorLayer(const KisGeneratorLayer& rhs)
    : KisSelectionBasedLayer(rhs),
      m_d(new Private)
{
    connect(&m_d->updateSignalCompressor, SIGNAL(timeout()), SLOT(slotDelayedStaticUpdate()));
}

KisGeneratorLayer::~KisGeneratorLayer()
{
}

void KisGeneratorLayer::setFilter(KisFilterConfigurationSP filterConfig)
{
    QMutexLocker locker(&m_d->mutex);
    KisSelectionBasedLayer::setFilter(filterConfig);
    m_d->preparedRect = QRect();
    m_d->updateSignalCompressor.start();
}

void KisGeneratorLayer::slotDelayedStaticUpdate()
{
    /**
     * The mask might have been deleted from the layers stack in the
     * meanwhile. Just ignore the updates in the case.
     */

    KisLayerSP parentLayer(qobject_cast<KisLayer*>(parent().data()));
    if (!parentLayer) return;

    KisImageSP image = parentLayer->image();
    if (image) {
        this->update();
    }
}

void KisGeneratorLayer::update()
{
    QMutexLocker locker(&m_d->mutex);

    KisImageSP image = this->image().toStrongRef();
    const QRect updateRect = extent() | image->bounds();

    KisFilterConfigurationSP filterConfig = filter();
    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);

    if (filterConfig != m_d->preparedForFilter) {
        locker.unlock();
        resetCache();
        locker.relock();
    }

    const QRegion processRegion(QRegion(updateRect) - m_d->preparedRect);
    if (processRegion.isEmpty()) return;

    KisGeneratorSP f = KisGeneratorRegistry::instance()->value(filterConfig->name());
    KIS_SAFE_ASSERT_RECOVER_RETURN(f);

    KisPaintDeviceSP originalDevice = original();

    KisGeneratorStrokeStrategy *stroke = new KisGeneratorStrokeStrategy(image);

    KisStrokeId strokeId = image->startStroke(stroke);

    QSharedPointer<bool> cookie(new bool(true));

    auto rc = processRegion.begin();
    while (rc != processRegion.end()) {
        QList<KisStrokeJobData *> jobs = KisGeneratorStrokeStrategy::createJobsData(this, cookie, f, originalDevice, *rc, filterConfig);

        Q_FOREACH (KisStrokeJobData *job, jobs) {
            image->addJob(strokeId, job);
        }

        rc++;
    }

    image->endStroke(strokeId);

    m_d->updateCookie = cookie;
    m_d->preparedRect = updateRect;
    m_d->preparedForFilter = filterConfig;
}

bool KisGeneratorLayer::accept(KisNodeVisitor & v)
{
    return v.visit(this);
}

void KisGeneratorLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

QIcon KisGeneratorLayer::icon() const
{
    return KisIconUtils::loadIcon("fillLayer");
}

KisBaseNode::PropertyList KisGeneratorLayer::sectionModelProperties() const
{
    KisFilterConfigurationSP filterConfig = filter();

    KisBaseNode::PropertyList l = KisLayer::sectionModelProperties();
    l << KisBaseNode::Property(KoID("generator", i18n("Generator")),
                               KisGeneratorRegistry::instance()->value(filterConfig->name())->name());

    return l;
}

void KisGeneratorLayer::setX(qint32 x)
{
    QMutexLocker(&m_d->mutex);
    KisSelectionBasedLayer::setX(x);
    m_d->preparedRect = QRect();
    m_d->updateSignalCompressor.start();
}

void KisGeneratorLayer::setY(qint32 y)
{
    QMutexLocker(&m_d->mutex);
    KisSelectionBasedLayer::setY(y);
    m_d->preparedRect = QRect();
    m_d->updateSignalCompressor.start();
}

void KisGeneratorLayer::resetCache()
{
    QMutexLocker(&m_d->mutex);
    KisSelectionBasedLayer::resetCache();
    m_d->preparedRect = QRect();
    m_d->updateSignalCompressor.start();
}

void KisGeneratorLayer::setDirty(const QVector<QRect> &rects)
{
    KisSelectionBasedLayer::setDirty(rects);
    m_d->updateSignalCompressor.start();
}

