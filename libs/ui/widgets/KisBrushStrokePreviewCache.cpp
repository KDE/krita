/*
 *  SPDX-FileCopyrightText: 2026 Tenzin Rangdol <tenzindraws@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushStrokePreviewCache.h"
#include "KisBrushStrokePreviewGenerator.h"
#include "KisResourceMetaDataModel.h"
#include "KisResourceLocator.h"

#include <QDir>
#include <QFile>
#include <QHash>
#include <QList>
#include <QPainter>
#include <QPointer>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>
#include <QThreadPool>

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
    /// Cached stroke previews
    QHash<QString, QImage> cache;
    /// Scaled images
    QHash<QString, QImage> scaledCache;
    QList<QString> accessOrder;
    int maxCacheSize = 500;
    KisPaintOpPresetSP currentPreset;
    KisSignalAutoConnectionsStore presetConnections;
    QString currentSettingsFingerprint;
    QString diskCachePath;
    QScopedPointer<KisBrushStrokePreviewGenerator> generator;

    struct PlaceholderStroke {
        QImage image;
        qreal pixelRatio;
    };

    std::optional<PlaceholderStroke> placeholder;
    std::optional<PlaceholderStroke> noPreviewPlaceholder;

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

QImage KisBrushStrokePreviewCache::loadFromDiskCache(int presetId)
{
    QImage image;
    const QString filename = getDiskCacheFilename(presetId);

    if (QFile::exists(filename)) {
        image.load(filename);
    }

    return image;
}

void KisBrushStrokePreviewCache::saveToDiskCache(int presetId, QImage image)
{
    if (image.isNull()) {
        return;
    }

    // The preset must've been loaded before to generate the preview, so loading it again is basically free and
    // shouldn't impact performace
    KoResourceSP preset = KisResourceLocator::instance()->resourceForId(presetId);

    //Just don't save dirty previews
    if (preset->isDirty()) {
        return;
    }

    //Save the md5 sum into the preview so that we can know it's the same brush
    QString md5 = preset->md5Sum();
    image.setText("md5", md5);

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

QImage KisBrushStrokePreviewCache::getPreview(int presetId, const QSize &size, qreal pixelRatio )
{
    if (presetId < 0 || size.isEmpty()) {
        return size.isEmpty() ? QImage() : generatePlaceholder(size, pixelRatio);
    }

    QString paintopId =
        KisResourceModelProvider::resourceMetadataModel()->metaDataValue(presetId, "paintopid").toString();

    // Return "No Preview" placeholder for unsupported brush engines
    if (isNoPreviewEngine(paintopId)) {
        return generateNoPreviewPlaceholder(size, pixelRatio);
    }

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
    if (it != m_d->cache.end() && !it->isNull()) {
        updateAccessOrder(baseKey);
        QImage scaled = scalePreviewToSize(*it, size);
        m_d->scaledCache.insert(sizedKey, scaled);
        return scaled;
    }

    // Try disk cache.
    if (hasDiskCache(presetId)) {

        // Get md5 first
        QString md5 = KisResourceLocator::instance()->md5ForId(presetId);

        const QImage diskImage = loadFromDiskCache(presetId);

        if (!diskImage.isNull()) {

            // If there's an md5 mismatch request preview regeneration
            if (diskImage.text("md5") != md5) {
                removeDiskCache(presetId);
                requestPreviewGeneration(presetId);
                return generatePlaceholder(size, pixelRatio);
            }
            evictIfNeeded();
            m_d->cache.insert(baseKey, diskImage);
            m_d->accessOrder.append(baseKey);
            QImage scaled = scalePreviewToSize(diskImage, size);
            m_d->scaledCache.insert(sizedKey, scaled);

            return scaled;
        }
    }

    // We schedule a preview to be generated here and in the meantime show a placeholder
    requestPreviewGeneration(presetId);

    return generatePlaceholder(size, pixelRatio);
}

void KisBrushStrokePreviewCache::requestPreviewGeneration(int presetId)
{
    if (m_d->generator) {
        // ask for it to be executed later, so that we don't clog up a single frame with a dozen brush preset requests
        QTimer::singleShot(0, [this, presetId] {
            KoResourceSP resource = KisResourceLocator::instance()->resourceForId(presetId);
            KisPaintOpPresetSP preset = resource.dynamicCast<KisPaintOpPreset>();
            m_d->generator->requestPreview(preset);
        });
    }
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

QImage KisBrushStrokePreviewCache::generatePlaceholder(const QSize &size, qreal pixelRatio) const
{
    // Cache previews with the same size, just because on first startup there will be A LOT of requests to generate them
    // and we don't need to waste the work
    if (m_d->placeholder.has_value()) {
        if (m_d->placeholder.value().image.size() == size && qFuzzyCompare(m_d->placeholder.value().pixelRatio, pixelRatio)) {
            return m_d->placeholder.value().image;
        }
    }

    QImage placeholder(size, QImage::Format_ARGB32_Premultiplied);
    placeholder.fill(Qt::transparent);

    QPainter painter(&placeholder);
    painter.setPen(Qt::white);
    QFont font;
    font.setPixelSize(qMin(size.height() / 2, qRound(10.0 * pixelRatio)));
    painter.setFont(font);
    painter.drawText(placeholder.rect(), Qt::AlignCenter, i18n("Loading..."));

    m_d->placeholder = Private::PlaceholderStroke{placeholder, pixelRatio};

    return placeholder;
}

QImage KisBrushStrokePreviewCache::generateNoPreviewPlaceholder(const QSize &size, qreal pixelRatio) const
{
    if (m_d->noPreviewPlaceholder.has_value()) {
        if (m_d->noPreviewPlaceholder.value().image.size() == size && qFuzzyCompare(m_d->noPreviewPlaceholder.value().pixelRatio, pixelRatio)) {
            return m_d->noPreviewPlaceholder.value().image;
        }
    }

    QImage placeholder(size, QImage::Format_ARGB32_Premultiplied);
    placeholder.fill(Qt::transparent);

    QPainter painter(&placeholder);
    painter.setPen(Qt::white);
    QFont font;
    font.setPixelSize(qMin(size.height() / 2, qRound(10.0 * pixelRatio)));
    painter.setFont(font);
    painter.drawText(placeholder.rect(), Qt::AlignCenter, i18n("No Preview"));

    m_d->noPreviewPlaceholder = Private::PlaceholderStroke{placeholder, pixelRatio};

    return placeholder;
}

void KisBrushStrokePreviewCache::slotLivePreviewImageReady(int presetId, QImage previewImage)
{
    if (presetId < 0 || previewImage.isNull()) {
        return;
    }

    saveToDiskCache(presetId, previewImage);

    slotPreviewImageReady(presetId, previewImage);
}

void KisBrushStrokePreviewCache::slotPreviewImageReady(int presetId, QImage previewImage)
{
    {
        const QString baseKey = generateCacheKey(presetId);

        auto it = m_d->cache.find(baseKey);
        if (it != m_d->cache.end()) {
            *it = previewImage;
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

    const QString baseKey = generateCacheKey(presetId);

    m_d->removeScaledVariants(baseKey);
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
