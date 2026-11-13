/*
 *  SPDX-FileCopyrightText: 2026 Tenzin Rangdol <tenzindraws@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brush_stroke_preview_cache_test.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>

#include "KisBrushStrokePreviewCache.h"
#include <brushengine/kis_paintop_preset.h>

namespace {

QImage makeSplitImage(const QSize &size, const QColor &left, const QColor &right)
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(left);

    QPainter painter(&image);
    painter.fillRect(QRect(size.width() / 2, 0,
                           size.width() - size.width() / 2,
                           size.height()),
                     right);
    return image;
}

QString cacheFilePathForPreset(int presetId)
{
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QStringLiteral("/brush_stroke_previews");
    QDir dir(cacheDir);
    dir.mkpath(QStringLiteral("."));
    return dir.filePath(QString::number(presetId) + QStringLiteral(".png"));
}

} // namespace

void KisBrushStrokePreviewCacheTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void KisBrushStrokePreviewCacheTest::testPlaceholderForInvalidPreset()
{
    KisPaintOpPresetSP preset(new KisPaintOpPreset());
    preset->setResourceId(-1);

    const QSize size(32, 16);
    QImage preview = KisBrushStrokePreviewCache::instance()->getPreview(preset, size);

    QCOMPARE(preview.size(), size);
    QVERIFY(!preview.isNull());
    QCOMPARE(preview.format(), QImage::Format_ARGB32_Premultiplied);
    QCOMPARE(preview.pixelColor(0, 0).alpha(), 0);
}

void KisBrushStrokePreviewCacheTest::testScaledCacheInvalidation()
{
    KisBrushStrokePreviewCache *cache = KisBrushStrokePreviewCache::instance();

    KisPaintOpPresetSP preset(new KisPaintOpPreset());
    const int presetId = 12001;
    preset->setResourceId(presetId);

    const QSize sourceSize(100, 50);
    const QSize targetSize(50, 50);

    QImage firstImage = makeSplitImage(sourceSize, Qt::red, Qt::blue);
    cache->slotLivePreviewImageReady(presetId, firstImage);

    QImage firstPreview = cache->getPreview(preset, targetSize);
    QCOMPARE(firstPreview.size(), targetSize);
    QVERIFY(firstPreview.pixelColor(10, 25).red() > 200);
    QVERIFY(firstPreview.pixelColor(40, 25).blue() > 200);

    QImage secondImage = makeSplitImage(sourceSize, Qt::green, Qt::blue);
    cache->slotLivePreviewImageReady(presetId, secondImage);

    QImage secondPreview = cache->getPreview(preset, targetSize);
    QCOMPARE(secondPreview.size(), targetSize);
    QVERIFY(secondPreview.pixelColor(10, 25).green() > 200);
    QVERIFY(secondPreview.pixelColor(40, 25).blue() > 200);

    cache->invalidatePreset(presetId);
}

void KisBrushStrokePreviewCacheTest::testDiskCacheFallback()
{
    KisBrushStrokePreviewCache *cache = KisBrushStrokePreviewCache::instance();

    KisPaintOpPresetSP preset(new KisPaintOpPreset());
    const int presetId = 12002;
    preset->setResourceId(presetId);

    const QSize sourceSize(80, 40);
    const QSize targetSize(40, 40);
    QImage sourceImage = makeSplitImage(sourceSize, Qt::red, Qt::blue);

    const QString filePath = cacheFilePathForPreset(presetId);
    QFile::remove(filePath);
    QVERIFY(sourceImage.save(filePath, "PNG"));

    QImage preview = cache->getPreview(preset, targetSize);
    QCOMPARE(preview.size(), targetSize);
    QVERIFY(preview.pixelColor(8, 20).red() > 200);
    QVERIFY(preview.pixelColor(30, 20).blue() > 200);

    QFile::remove(filePath);
    cache->invalidatePreset(presetId);
}

SIMPLE_TEST_MAIN(KisBrushStrokePreviewCacheTest)
