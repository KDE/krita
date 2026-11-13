/*
 *  SPDX-FileCopyrightText: 2026 Tenzin Rangdol <tenzindraws@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushStrokePreviewCache.h"
#include "KisBrushStrokePreviewGenerator.h"

#include <QDir>
#include <QFile>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QPointer>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>

#include <klocalizedstring.h>
#include <kis_paintop_settings.h>
#include <KisPaintOpPresetUpdateProxy.h>
#include <KisResourceModelProvider.h>
#include <KisResourceTypes.h>
#include <KisResourceModel.h>
#include <KoCanvasResourcesIds.h>
#include <kis_signal_auto_connection.h>

namespace {

/// Returns preset's resource ID, or -1 if preset is null.
inline int validPresetId(KisPaintOpPresetSP preset)
{
    return preset ? preset->resourceId() : -1;
}

} // namespace

struct KisBrushStrokePreviewCache::Private {
    struct CacheEntry {
        QImage image;
    };

    mutable QMutex mutex;
    QHash<QString, CacheEntry> cache;
    QHash<QString, QImage> scaledCache;
    QList<QString> accessOrder;
    int maxCacheSize = 500;
    KisPaintOpPresetSP currentPreset;
    KisSignalAutoConnectionsStore presetConnections;
    QString currentSettingsFingerprint;
    QString diskCachePath;
    QScopedPointer<KisBrushStrokePreviewGenerator> generator;
    bool pendingGenerateAll {false};

    /// Remove all scaled cache entries for a base key.
    void removeScaledVariants(const QString &baseKey)
    {
        QMutableHashIterator<QString, QImage> it(scaledCache);
        const QString prefix = baseKey + QLatin1Char('_');
        while (it.hasNext()) {
            it.next();
            if (it.key().startsWith(prefix)) {
                it.remove();
            }
        }
    }
};

KisBrushStrokePreviewCache::KisBrushStrokePreviewCache()
    : QObject(nullptr)
    , m_d(new Private)
{
    initDiskCache();
    m_d->generator.reset(new KisBrushStrokePreviewGenerator(this));

    KisAllResourcesModel *model = KisResourceModelProvider::resourceModel(ResourceType::PaintOpPresets);
    if (model) {
        connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(slotScheduleGenerateAllPreviews()));
        connect(model, SIGNAL(modelReset()),
                this, SLOT(slotScheduleGenerateAllPreviews()));
    }
}

KisBrushStrokePreviewCache::~KisBrushStrokePreviewCache()
{
}

KisBrushStrokePreviewCache* KisBrushStrokePreviewCache::instance()
{
    static KisBrushStrokePreviewCache s_instance;
    return &s_instance;
}

bool KisBrushStrokePreviewCache::needsStripedBackground(const QString &paintOpId)
{
    return paintOpId == QLatin1String("colorsmudge")
        || paintOpId == QLatin1String("deformbrush")
        || paintOpId == QLatin1String("filter");
}

bool KisBrushStrokePreviewCache::isNoPreviewEngine(const QString &paintOpId)
{
    return paintOpId == QLatin1String("roundmarker")
        || paintOpId == QLatin1String("experimentbrush")
        || paintOpId == QLatin1String("duplicate");
}

void KisBrushStrokePreviewCache::initDiskCache()
{
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_d->diskCachePath = cacheDir + QStringLiteral("/brush_stroke_previews");
    QDir dir(m_d->diskCachePath);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
}

QString KisBrushStrokePreviewCache::getDiskCacheFilename(int presetId) const
{
    return QDir(m_d->diskCachePath).filePath(QString::number(presetId) + QStringLiteral(".png"));
}

bool KisBrushStrokePreviewCache::loadFromDiskCache(int presetId, QImage &image)
{
    const QString filename = getDiskCacheFilename(presetId);
    return QFile::exists(filename) && image.load(filename);
}

void KisBrushStrokePreviewCache::saveToDiskCache(int presetId, const QImage &image)
{
    if (image.isNull()) {
        return;
    }

    QSaveFile file(getDiskCacheFilename(presetId));
    if (file.open(QIODevice::WriteOnly) && image.save(&file, "PNG", 50)) {
        file.commit();
    }
}

bool KisBrushStrokePreviewCache::hasDiskCache(int presetId) const
{
    return QFile::exists(getDiskCacheFilename(presetId));
}

void KisBrushStrokePreviewCache::removeDiskCache(int presetId)
{
    QFile::remove(getDiskCacheFilename(presetId));
}

QString KisBrushStrokePreviewCache::generateCacheKey(int presetId) const
{
    return QString::number(presetId);
}

QString KisBrushStrokePreviewCache::generateSizedCacheKey(int presetId, const QSize &size) const
{
    return QStringLiteral("%1_%2x%3").arg(presetId).arg(size.width()).arg(size.height());
}

void KisBrushStrokePreviewCache::evictIfNeeded()
{
    while (m_d->cache.size() >= m_d->maxCacheSize && !m_d->accessOrder.isEmpty()) {
        const QString oldestKey = m_d->accessOrder.takeFirst();
        m_d->cache.remove(oldestKey);
        m_d->removeScaledVariants(oldestKey);
    }
}

void KisBrushStrokePreviewCache::updateAccessOrder(const QString &key)
{
    m_d->accessOrder.removeAll(key);
    m_d->accessOrder.append(key);
}

QImage KisBrushStrokePreviewCache::getPreview(KisPaintOpPresetSP preset, const QSize &size)
{
    const int presetId = validPresetId(preset);
    if (presetId < 0 || size.isEmpty()) {
        return size.isEmpty() ? QImage() : generatePlaceholder(size);
    }

    // Return "No Preview" placeholder for unsupported brush engines
    if (preset && isNoPreviewEngine(preset->paintOp().id())) {
        return generateNoPreviewPlaceholder(size);
    }

    QMutexLocker locker(&m_d->mutex);

    const QString baseKey = generateCacheKey(presetId);
    const QString sizedKey = generateSizedCacheKey(presetId, size);

    // Check scaled cache first.
    auto scaledIt = m_d->scaledCache.find(sizedKey);
    if (scaledIt != m_d->scaledCache.end()) {
        updateAccessOrder(baseKey);
        return scaledIt.value();
    }

    // Check base cache.
    auto it = m_d->cache.find(baseKey);
    if (it != m_d->cache.end() && !it->image.isNull()) {
        updateAccessOrder(baseKey);
        QImage scaled = scalePreviewToSize(it->image, size);
        m_d->scaledCache.insert(sizedKey, scaled);
        return scaled;
    }

    // Try disk cache.
    QImage diskImage;
    if (loadFromDiskCache(presetId, diskImage) && !diskImage.isNull()) {
        evictIfNeeded();
        m_d->cache.insert(baseKey, {diskImage});
        m_d->accessOrder.append(baseKey);
        QImage scaled = scalePreviewToSize(diskImage, size);
        m_d->scaledCache.insert(sizedKey, scaled);
        return scaled;
    }

    return generatePlaceholder(size);
}

QImage KisBrushStrokePreviewCache::scalePreviewToSize(const QImage &source, const QSize &targetSize) const
{
    if (source.isNull() || targetSize.isEmpty()) {
        return source;
    }

    // Center-crop to fill target.
    const qreal sourceAspect = static_cast<qreal>(source.width()) / source.height();
    const qreal targetAspect = static_cast<qreal>(targetSize.width()) / targetSize.height();

    QImage scaled;
    if (sourceAspect > targetAspect) {
        int scaledHeight = targetSize.height();
        int scaledWidth = static_cast<int>(scaledHeight * sourceAspect);
        scaled = source.scaled(scaledWidth, scaledHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        int xOffset = (scaled.width() - targetSize.width()) / 2;
        scaled = scaled.copy(xOffset, 0, targetSize.width(), targetSize.height());
    } else {
        int scaledWidth = targetSize.width();
        int scaledHeight = static_cast<int>(scaledWidth / sourceAspect);
        scaled = source.scaled(scaledWidth, scaledHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        int yOffset = (scaled.height() - targetSize.height()) / 2;
        scaled = scaled.copy(0, yOffset, targetSize.width(), targetSize.height());
    }

    return scaled;
}

QImage KisBrushStrokePreviewCache::generatePlaceholder(const QSize &size) const
{
    QImage placeholder(size, QImage::Format_ARGB32_Premultiplied);
    placeholder.fill(Qt::transparent);

    QPainter painter(&placeholder);
    painter.setPen(Qt::white);
    QFont font;
    font.setPixelSize(qMin(size.height() / 2, 10));
    painter.setFont(font);
    painter.drawText(placeholder.rect(), Qt::AlignCenter, i18n("Loading..."));

    return placeholder;
}

QImage KisBrushStrokePreviewCache::generateNoPreviewPlaceholder(const QSize &size) const
{
    QImage placeholder(size, QImage::Format_ARGB32_Premultiplied);
    placeholder.fill(Qt::transparent);

    QPainter painter(&placeholder);
    painter.setPen(Qt::white);
    QFont font;
    font.setPixelSize(qMin(size.height() / 2, 10));
    painter.setFont(font);
    painter.drawText(placeholder.rect(), Qt::AlignCenter, i18n("No Preview"));

    return placeholder;
}

void KisBrushStrokePreviewCache::slotLivePreviewImageReady(int presetId, const QImage &previewImage)
{
    if (presetId < 0 || previewImage.isNull()) {
        return;
    }

    saveToDiskCache(presetId, previewImage);

    {
        QMutexLocker locker(&m_d->mutex);

        const QString baseKey = generateCacheKey(presetId);

        auto it = m_d->cache.find(baseKey);
        if (it != m_d->cache.end()) {
            it->image = previewImage;
            updateAccessOrder(baseKey);
        } else {
            evictIfNeeded();
            m_d->cache.insert(baseKey, {previewImage});
            m_d->accessOrder.append(baseKey);
        }

        m_d->removeScaledVariants(baseKey);
    }

    Q_EMIT sigPreviewReady(presetId);
}

void KisBrushStrokePreviewCache::invalidatePreset(int presetId)
{
    if (presetId < 0) {
        return;
    }

    removeDiskCache(presetId);

    QMutexLocker locker(&m_d->mutex);

    const QString baseKey = generateCacheKey(presetId);

    m_d->removeScaledVariants(baseKey);
}

void KisBrushStrokePreviewCache::generateAllPreviews()
{
    KisAllResourcesModel *model = KisResourceModelProvider::resourceModel(ResourceType::PaintOpPresets);
    if (!model) {
        return;
    }

    QList<KisPaintOpPresetSP> missing;
    const int rowCount = model->rowCount();

    for (int i = 0; i < rowCount; ++i) {
        QModelIndex idx = model->index(i, 0);
        KisPaintOpPresetSP preset = model->resourceForIndex(idx).dynamicCast<KisPaintOpPreset>();

        const int presetId = validPresetId(preset);
        if (presetId < 0) {
            continue;
        }

        const QString baseKey = generateCacheKey(presetId);
        bool cached;
        {
            QMutexLocker locker(&m_d->mutex);
            cached = m_d->cache.contains(baseKey);
        }

        if (!cached && !hasDiskCache(presetId)) {
            missing.append(preset);
        }
    }

    if (!missing.isEmpty() && m_d->generator) {
        m_d->generator->startBatch(missing);
    }
}

void KisBrushStrokePreviewCache::slotScheduleGenerateAllPreviews()
{
    if (m_d->pendingGenerateAll) {
        return;
    }

    m_d->pendingGenerateAll = true;
    QTimer::singleShot(0, this, SLOT(slotRunGenerateAllPreviews()));
}

void KisBrushStrokePreviewCache::slotRunGenerateAllPreviews()
{
    m_d->pendingGenerateAll = false;
    generateAllPreviews();
}

void KisBrushStrokePreviewCache::registerLivePreviewView(KisPresetLivePreviewView *view)
{
    if (m_d->generator) {
        m_d->generator->setSourceView(view);
    }
}

void KisBrushStrokePreviewCache::slotPresetSettingsChanged(KisPaintOpPresetSP preset)
{
    const int presetId = validPresetId(preset);
    if (presetId < 0) {
        return;
    }

    invalidatePreset(presetId);

    if (m_d->generator) {
        m_d->generator->requestPreview(preset);
    }

    Q_EMIT sigPreviewReady(presetId);
}

void KisBrushStrokePreviewCache::slotSetCurrentPreset(KisPaintOpPresetSP preset)
{
    m_d->presetConnections.clear();
    m_d->currentPreset = preset;
    m_d->currentSettingsFingerprint.clear();

    if (!preset) {
        return;
    }

    m_d->currentSettingsFingerprint = generateSettingsFingerprint(preset);

    QPointer<KisPaintOpPresetUpdateProxy> proxy = preset->updateProxy();
    if (proxy) {
        m_d->presetConnections.addConnection(
            proxy, SIGNAL(sigSettingsChanged()),
            this, SLOT(slotCurrentPresetSettingsChanged()));
    }
}

void KisBrushStrokePreviewCache::slotCurrentPresetSettingsChanged()
{
    if (!m_d->currentPreset) {
        return;
    }

    const QString newFingerprint = generateSettingsFingerprint(m_d->currentPreset);
    if (newFingerprint == m_d->currentSettingsFingerprint) {
        return;
    }

    m_d->currentSettingsFingerprint = newFingerprint;
    slotPresetSettingsChanged(m_d->currentPreset);
}

void KisBrushStrokePreviewCache::slotCanvasResourceChanged(int key, const QVariant &value)
{
    Q_UNUSED(value);

    if (!m_d->currentPreset) {
        return;
    }

    const bool affectsBrush = key == KoCanvasResource::Size
        || key == KoCanvasResource::Opacity
        || key == KoCanvasResource::Flow
        || key == KoCanvasResource::BrushRotation;

    if (!affectsBrush) {
        return;
    }

    const int presetId = m_d->currentPreset->resourceId();
    invalidatePreset(presetId);

    if (m_d->generator) {
        m_d->generator->requestPreview(m_d->currentPreset);
    }

    if (presetId >= 0) {
        Q_EMIT sigPreviewReady(presetId);
    }
}

QString KisBrushStrokePreviewCache::generateSettingsFingerprint(KisPaintOpPresetSP preset) const
{
    if (!preset || !preset->settings()) {
        return QString();
    }

    QString xml = preset->settings()->toXML();

    // Strip stroke-time params that vary per-stroke.
    static const QStringList strokeTimeProperties = {
        QStringLiteral("Texture/Pattern/OffsetX"),
        QStringLiteral("Texture/Pattern/OffsetY")
    };

    Q_FOREACH (const QString &propName, strokeTimeProperties) {
        QRegularExpression re(
            QStringLiteral("<param name=\"%1\"[^>]*>[^<]*</param>\\s*")
                .arg(QRegularExpression::escape(propName)));
        xml.remove(re);
    }

    return xml;
}
