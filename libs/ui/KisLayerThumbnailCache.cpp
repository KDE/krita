/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisLayerThumbnailCache.h"

#include "kis_config.h"
#include "kis_image.h"
#include "KisIdleTasksManager.h"
#include "kis_layer_utils.h"

#include "KisRunnableStrokeJobUtils.h"
#include "KisRunnableStrokeJobsInterface.h"


namespace {
struct ThumbnailRecord {
    QImage image;
    int seqNo = -1;
    int maxSize = 0;
};
} // namespace

struct ThumbnailsStroke : KisIdleTaskStrokeStrategy
{
    Q_OBJECT
public:

    ThumbnailsStroke(KisImageSP image, int maxSize, const QMap<KisNodeWSP, ThumbnailRecord> &cache)
        : KisIdleTaskStrokeStrategy(QLatin1String("layer-thumbnails-stroke"), kundo2_i18n("Update layer thumbnails"))
        , m_root(image->root())
        , m_maxSize(maxSize)
        , m_cache(cache)
    {
        // thread-safety!
        m_cache.detach();
    }

    void initStrokeCallback() override
    {
        KisIdleTaskStrokeStrategy::initStrokeCallback();

        using KisLayerUtils::recursiveApplyNodes;
        using KritaUtils::addJobConcurrent;

        QVector<KisRunnableStrokeJobData*> jobs;
        recursiveApplyNodes(m_root, [&jobs, this] (KisNodeSP node) {


            if (!node->parent()) return;
            if (node->isFakeNode()) return;

            bool shouldRegenerateThumbnail = false;

            auto it = m_cache.find(node);

            if (it != m_cache.end()) {
                ThumbnailRecord rec = *it;

                if (rec.seqNo != node->thumbnailSeqNo() ||
                    rec.maxSize != m_maxSize) {

                    shouldRegenerateThumbnail = true;
                }
            } else {
                shouldRegenerateThumbnail = true;
            }

            if (shouldRegenerateThumbnail) {
                const int timeout = KisConfig(true).layerThumbnailGenerationTimeout();

                addJobConcurrent(jobs, [node, timeout, this] () mutable {
                    QElapsedTimer timer;
                    timer.start();
                    QImage image = node->createPreferredThumbnail(m_maxSize, m_maxSize, Qt::KeepAspectRatio);
                    const int measuredTime = timer.elapsed();

                    if (node->preferredThumbnailBoundsMode() == KisThumbnailBoundsMode::Precise &&
                        measuredTime > timeout) {
                        warnUI << "WARNING: thumbnail generation for" << node->name() << "took longer than expected:" << measuredTime << "(timeout:" << timeout << ")";
                        warnUI << "         This layer's thumbnail will be rendered in imprecise mode from now on";
                        node->setPreferredThumbnailBoundsMode(KisThumbnailBoundsMode::Coarse);
                    }

                    this->sigThumbnailGenerated(node, node->thumbnailSeqNo(), m_maxSize, image);
                });
            }
        });

        runnableJobsInterface()->addRunnableJobs(jobs);
    }

Q_SIGNALS:
    void sigThumbnailGenerated(KisNodeSP node, int maxSize, int seqNo, const QImage &thumb);
private:

    KisNodeSP m_root;
    int m_maxSize;
    QMap<KisNodeWSP, ThumbnailRecord> m_cache;

};

struct KisLayerThumbnailCache::Private
{
    KisImageWSP image;

    KisIdleTasksManager::TaskGuard taskGuard;
    int maxSize = 32;
    QMap<KisNodeWSP, ThumbnailRecord> cache;

    void cleanupDeletedNodes();
};


KisLayerThumbnailCache::KisLayerThumbnailCache()
    : m_d(new Private())
{
}

KisLayerThumbnailCache::~KisLayerThumbnailCache() = default;

void KisLayerThumbnailCache::setIdleTaskManagerImpl(KisIdleTasksManager *manager)
{
    if (manager) {
        m_d->taskGuard = manager->addIdleTaskWithGuard([this] (KisImageSP image) {
            ThumbnailsStroke *stroke = new ThumbnailsStroke(image, m_d->maxSize, m_d->cache);
            connect(stroke, SIGNAL(sigThumbnailGenerated(KisNodeSP, int, int, QImage)), this, SLOT(slotThumbnailGenerated(KisNodeSP, int, int, QImage)));
            return stroke;
        });
    } else {
        m_d->taskGuard = KisIdleTasksManager::TaskGuard();
    }
}

void KisLayerThumbnailCache::setImage(KisImageSP image)
{
    m_d->image = image;
    m_d->cache.clear();

    if (m_d->image && m_d->taskGuard.isValid()) {
        m_d->taskGuard.trigger();
    }
}

void KisLayerThumbnailCache::setIdleTaskManager(KisIdleTasksManager *manager)
{
    setIdleTaskManagerImpl(manager);
    if (m_d->image && m_d->taskGuard.isValid()) {
        m_d->taskGuard.trigger();
    }
}

void KisLayerThumbnailCache::setImage(KisImageSP image, KisIdleTasksManager *manager)
{
    setIdleTaskManagerImpl(manager);
    setImage(image);
    // the update is triggered only by the second call, not the first one!
}

void KisLayerThumbnailCache::setMaxSize(int maxSize)
{
    m_d->maxSize = maxSize;
    if (m_d->image && m_d->taskGuard.isValid()) {
        m_d->taskGuard.trigger();
    }
}

int KisLayerThumbnailCache::maxSize() const
{
    return m_d->maxSize;
}

QImage KisLayerThumbnailCache::thumbnail(KisNodeSP node) const
{
    QImage image;

    auto it = m_d->cache.find(node);
    if (it != m_d->cache.end()) {
        image = it->image;

        if (it->maxSize > m_d->maxSize) {
            image = image.scaled(m_d->maxSize, m_d->maxSize, Qt::KeepAspectRatio);
        }
    } else {
        image = QImage(1, 1, QImage::Format_ARGB32);
        image.fill(0);
    }

    return image;
}

void KisLayerThumbnailCache::Private::cleanupDeletedNodes()
{
    for (auto it = cache.begin(); it != cache.end();) {
        if (!it.key()) {
            it = cache.erase(it);
        } else {
            ++it;
        }
    }
}

void KisLayerThumbnailCache::notifyNodeRemoved(KisNodeSP node)
{
    Q_UNUSED(node);
    m_d->cleanupDeletedNodes();
}

void KisLayerThumbnailCache::notifyNodeAdded(KisNodeSP node)
{
    Q_UNUSED(node);
    m_d->cleanupDeletedNodes();

    if (m_d->image && m_d->taskGuard.isValid()) {
        m_d->taskGuard.trigger();
    }
}

void KisLayerThumbnailCache::clear()
{
    m_d->cache.clear();
}

void KisLayerThumbnailCache::slotThumbnailGenerated(KisNodeSP node, int seqNo, int maxSize, const QImage &thumb)
{
    if (node->image() != m_d->image) {
        qWarning() << "KisLayerThumbnailCache::slotThumbnailGenerated: node does not belong to the attached image anymore!" << ppVar(node) << ppVar(m_d->image);
        return;
    }

    m_d->cache[node] = {thumb, seqNo, maxSize};
    Q_EMIT sigLayerThumbnailUpdated(node);
}

#include "KisLayerThumbnailCache.moc"
