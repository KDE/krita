/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <kis_generator_stroke_strategy.h>
#include <KisRunnableStrokeJobData.h>


#define UPDATE_DELAY 100 /*ms */

struct Q_DECL_HIDDEN KisGeneratorLayer::Private
{
    Private()
        : updateSignalCompressor(UPDATE_DELAY, KisSignalCompressor::FIRST_INACTIVE)
    {
    }

    KisThreadSafeSignalCompressor updateSignalCompressor;
    QRect preparedRect;
    QRect preparedImageBounds;
    KisFilterConfigurationSP preparedForFilter;
    QWeakPointer<boost::none_t> updateCookie;
    QMutex mutex;
};


KisGeneratorLayer::KisGeneratorLayer(KisImageWSP image,
                                     const QString &name,
                                     KisFilterConfigurationSP kfc,
                                     KisSelectionSP selection)
    : KisSelectionBasedLayer(image, name, selection, kfc),
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

void KisGeneratorLayer::setFilter(KisFilterConfigurationSP filterConfig, bool checkCompareConfig)
{
    setFilterWithoutUpdate(filterConfig, checkCompareConfig);
    m_d->updateSignalCompressor.start();
}

void KisGeneratorLayer::setFilterWithoutUpdate(KisFilterConfigurationSP filterConfig, bool checkCompareConfig)
{
    if (filter().isNull() || (!checkCompareConfig || !filter()->compareTo(filterConfig.constData()))) {
        KisSelectionBasedLayer::setFilter(filterConfig);
        {
            QMutexLocker locker(&m_d->mutex);
            m_d->preparedRect = QRect();
        }
    }
}

void KisGeneratorLayer::slotDelayedStaticUpdate()
{
    /**
     * Don't try to start a regeneration stroke while image
     * is locked. It may happen on loading, when all necessary
     * conversions are not yet finished.
     */
    if (KisImageSP image = this->image(); image && image->locked()) {
        m_d->updateSignalCompressor.start();
        return;
    }

    /**
     * The mask might have been deleted from the layers stack in the
     * meanwhile. Just ignore the updates in the case.
     */

    KisLayerSP parentLayer(qobject_cast<KisLayer*>(parent().data()));
    if (!parentLayer) return;

    KisImageSP image = parentLayer->image();

    if (image) {
        if (!m_d->updateCookie) {
            this->update();
        } else {
            m_d->updateSignalCompressor.start();
        }
    }
}

void KisGeneratorLayer::requestUpdateJobsWithStroke(KisStrokeId strokeId, KisFilterConfigurationSP filterConfig)
{
    QMutexLocker locker(&m_d->mutex);
    
    KisImageSP image = this->image().toStrongRef();
    const QRect updateRect = extent() | image->bounds();

    if (filterConfig != m_d->preparedForFilter) {
        locker.unlock();
        resetCacheWithoutUpdate(this->paintDevice()->colorSpace());
        locker.relock();
    }

    if (m_d->preparedImageBounds != image->bounds()) {
        m_d->preparedRect = QRect();
    }

    const QRegion processRegion(QRegion(updateRect) - m_d->preparedRect);
    if (processRegion.isEmpty())
        return;

    KisGeneratorSP f = KisGeneratorRegistry::instance()->value(filterConfig->name());
    KIS_SAFE_ASSERT_RECOVER_RETURN(f);

    KisPaintDeviceSP originalDevice = original();

    QSharedPointer<boost::none_t> cookie(new boost::none_t(boost::none));

    auto jobs = KisGeneratorStrokeStrategy::createJobsData(this, cookie, f, originalDevice, processRegion, filterConfig);

    Q_FOREACH (auto job, jobs) {
        image->addJob(strokeId, job);
    }

    m_d->updateCookie = cookie;
    m_d->preparedRect = updateRect;
    m_d->preparedImageBounds = image->bounds();
    m_d->preparedForFilter = filterConfig;
}

QWeakPointer<boost::none_t> KisGeneratorLayer::previewWithStroke(const KisStrokeId strokeId)
{
    KisFilterConfigurationSP filterConfig = filter();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(filterConfig, QWeakPointer<boost::none_t>());

    requestUpdateJobsWithStroke(strokeId, filterConfig);
    return m_d->updateCookie;
}

void KisGeneratorLayer::update()
{
    KisImageSP image = this->image().toStrongRef();

    KisFilterConfigurationSP filterConfig = filter();
    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);

    KisGeneratorStrokeStrategy *stroke = new KisGeneratorStrokeStrategy();

    KisStrokeId strokeId = image->startStroke(stroke);

    requestUpdateJobsWithStroke(strokeId, filterConfig);

    image->endStroke(strokeId);
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
    KisSelectionBasedLayer::setX(x);
    {
        QMutexLocker locker(&m_d->mutex);
        m_d->preparedRect = QRect();
    }
    m_d->updateSignalCompressor.start();
}

void KisGeneratorLayer::setY(qint32 y)
{
    KisSelectionBasedLayer::setY(y);
    {
        QMutexLocker locker(&m_d->mutex);
        m_d->preparedRect = QRect();
    }
    m_d->updateSignalCompressor.start();
}

void KisGeneratorLayer::resetCache(const KoColorSpace *colorSpace)
{
    resetCacheWithoutUpdate(colorSpace);
    m_d->updateSignalCompressor.start();
}

void KisGeneratorLayer::forceUpdateTimedNode()
{
    if (hasPendingTimedUpdates()) {
        m_d->updateSignalCompressor.stop();
        m_d->updateCookie.clear();

        slotDelayedStaticUpdate();
    }
}

bool KisGeneratorLayer::hasPendingTimedUpdates() const
{
    return m_d->updateSignalCompressor.isActive();
}

void KisGeneratorLayer::resetCacheWithoutUpdate(const KoColorSpace *colorSpace)
{
    KisSelectionBasedLayer::resetCache(colorSpace);
    {
        QMutexLocker locker(&m_d->mutex);
        m_d->preparedRect = QRect();
    }
}

void KisGeneratorLayer::setDirty(const QVector<QRect> &rects)
{
    setDirtyWithoutUpdate(rects);
    m_d->updateSignalCompressor.start();
}

void KisGeneratorLayer::setDirtyWithoutUpdate(const QVector<QRect> &rects)
{
    KisSelectionBasedLayer::setDirty(rects);
}
