/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisTextureMaskInfo.h"

#include <kis_paintop_settings.h>
#include <resources/KoPattern.h>
#include "kis_embedded_pattern_manager.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_algebra_2d.h>
#include <kis_lod_transform.h>
#include <kis_iterator_ng.h>

#include <QGlobalStatic>

/**********************************************************************/
/*       KisTextureMaskInfo                                           */
/**********************************************************************/


KisTextureMaskInfo::KisTextureMaskInfo(int levelOfDetail)
    : m_levelOfDetail(levelOfDetail)
{
}

KisTextureMaskInfo::KisTextureMaskInfo(const KisTextureMaskInfo &rhs)
    : m_levelOfDetail(rhs.m_levelOfDetail),
      m_pattern(rhs.m_pattern),
      m_scale(rhs.m_scale),
      m_brightness(rhs.m_brightness),
      m_contrast(rhs.m_contrast),
      m_neutralPoint(rhs.m_neutralPoint),
      m_invert(rhs.m_invert),
      m_cutoffLeft(rhs.m_cutoffLeft),
      m_cutoffRight(rhs.m_cutoffRight),
      m_cutoffPolicy(rhs.m_cutoffPolicy)
{
}

KisTextureMaskInfo::~KisTextureMaskInfo()
{
}

bool operator==(const KisTextureMaskInfo &lhs, const KisTextureMaskInfo &rhs) {
    return
            lhs.m_levelOfDetail == rhs.m_levelOfDetail &&
            (lhs.m_pattern == rhs.m_pattern ||
             (lhs.m_pattern &&
              rhs.m_pattern &&
              lhs.m_pattern->md5() == rhs.m_pattern->md5())) &&
            qFuzzyCompare(lhs.m_scale, rhs.m_scale) &&
            qFuzzyCompare(lhs.m_brightness, rhs.m_brightness) &&
            qFuzzyCompare(lhs.m_contrast, rhs.m_contrast) &&
            qFuzzyCompare(lhs.m_neutralPoint, rhs.m_neutralPoint) &&
            lhs.m_invert == rhs.m_invert &&
            lhs.m_cutoffLeft == rhs.m_cutoffLeft &&
            lhs.m_cutoffRight == rhs.m_cutoffRight &&
            lhs.m_cutoffPolicy == rhs.m_cutoffPolicy;
}

KisTextureMaskInfo &KisTextureMaskInfo::operator=(const KisTextureMaskInfo &rhs)
{
    m_levelOfDetail = rhs.m_levelOfDetail;
    m_pattern = rhs.m_pattern;
    m_scale = rhs.m_scale;
    m_brightness = rhs.m_brightness;
    m_contrast = rhs.m_contrast;
    m_neutralPoint = rhs.m_neutralPoint;
    m_invert = rhs.m_invert;
    m_cutoffLeft = rhs.m_cutoffLeft;
    m_cutoffRight = rhs.m_cutoffRight;
    m_cutoffPolicy = rhs.m_cutoffPolicy;

    return *this;
}

int KisTextureMaskInfo::levelOfDetail() const {
    return m_levelOfDetail;
}

bool KisTextureMaskInfo::hasMask() const {
    return m_mask;
}

KisPaintDeviceSP KisTextureMaskInfo::mask() {
    return m_mask;
}

QRect KisTextureMaskInfo::maskBounds() const {
    return m_maskBounds;
}

bool KisTextureMaskInfo::fillProperties(const KisPropertiesConfigurationSP setting)
{

    if (!setting->hasProperty("Texture/Pattern/PatternMD5")) {
        return false;
    }

    m_pattern = KisEmbeddedPatternManager::loadEmbeddedPattern(setting);

    if (!m_pattern) {
        warnKrita << "WARNING: Couldn't load the pattern for a stroke";
        return false;
    }

    m_scale = setting->getDouble("Texture/Pattern/Scale", 1.0);
    m_brightness = setting->getDouble("Texture/Pattern/Brightness");
    m_contrast = setting->getDouble("Texture/Pattern/Contrast", 1.0);
    m_neutralPoint = setting->getDouble("Texture/Pattern/NeutralPoint", 0.5);
    m_invert = setting->getBool("Texture/Pattern/Invert");
    m_cutoffLeft = setting->getInt("Texture/Pattern/CutoffLeft", 0);
    m_cutoffRight = setting->getInt("Texture/Pattern/CutoffRight", 255);
    m_cutoffPolicy = setting->getInt("Texture/Pattern/CutoffPolicy", 0);

    return true;
}

void KisTextureMaskInfo::recalculateMask()
{
    if (!m_pattern) return;

    const KoColorSpace* cs;
    bool hasAlpha = m_pattern->hasAlpha();

    if (hasAlpha) {
        cs = KoColorSpaceRegistry::instance()->rgb8();
    } else {
        cs = KoColorSpaceRegistry::instance()->alpha8();
    }
    if (!m_mask) {
        m_mask = new KisPaintDevice(cs);
    }

    QImage mask = m_pattern->pattern();

    if ((mask.format() != QImage::Format_RGB32) |
        (mask.format() != QImage::Format_ARGB32)) {

        mask = mask.convertToFormat(QImage::Format_ARGB32);
    }

    qreal scale = m_scale * KisLodTransform::lodToScale(m_levelOfDetail);

    if (!qFuzzyCompare(scale, 0.0) && !qFuzzyCompare(scale, 1.0)) {
        QTransform tf;
        tf.scale(scale, scale);
        QRect rc = KisAlgebra2D::ensureRectNotSmaller(tf.mapRect(mask.rect()), QSize(2,2));
        mask = mask.scaled(rc.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        // detach the mask from the file loaded from the storage
        mask = QImage(mask);
    }

    QRgb* pixel = reinterpret_cast<QRgb*>(mask.bits());
    const int width = mask.width();
    const int height = mask.height();

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            const QRgb currentPixel = pixel[row * width + col];

            const int red = qRed(currentPixel);
            const int green = qGreen(currentPixel);
            const int blue = qBlue(currentPixel);
            float alpha = qAlpha(currentPixel) / 255.0;

            const int grayValue = (red * 11 + green * 16 + blue * 5) / 32;
            float maskValue = (grayValue / 255.0) * alpha + (1 - alpha);

            maskValue = maskValue - m_brightness;

            maskValue = ((maskValue - 0.5)*m_contrast)+0.5;

            if (maskValue > 1.0) {maskValue = 1;}
            else if (maskValue < 0) {maskValue = 0;}

            if (m_invert) {
                maskValue = 1 - maskValue;
            }

            if (m_cutoffPolicy == 1 && (maskValue < (m_cutoffLeft / 255.0) || maskValue > (m_cutoffRight / 255.0))) {
                // mask out the dab if it's outside the pattern's cuttoff points
                alpha = OPACITY_TRANSPARENT_F;
            }
            else if (m_cutoffPolicy == 2 && (maskValue < (m_cutoffLeft / 255.0) || maskValue > (m_cutoffRight / 255.0))) {
                alpha = OPACITY_OPAQUE_F;
            }

            maskValue = qBound(0.0f, maskValue, 1.0f);

            float neutralAdjustedValue;
            
            //Adjust neutral point in linear fashion.  Uses separate linear equations from 0 to neutralPoint, and neutralPoint to 1,
            //to prevent loss of detail (clipping).
            if (m_neutralPoint == 1 || (m_neutralPoint != 0 && maskValue <= m_neutralPoint)) {
                neutralAdjustedValue = maskValue / (2 * m_neutralPoint);
            }
            else {
                neutralAdjustedValue = 0.5 +  (maskValue - m_neutralPoint) / (2 - 2 * m_neutralPoint);
            }

            int finalValue = neutralAdjustedValue * 255;
            pixel[row * width + col] = QColor(finalValue, finalValue, finalValue, alpha * 255).rgba();

        }
    }

    m_mask->convertFromQImage(mask, 0);
    m_maskBounds = QRect(0, 0, width, height);
}

bool KisTextureMaskInfo::hasAlpha() {
    return m_pattern->hasAlpha();
}

/**********************************************************************/
/*       KisTextureMaskInfoCache                                      */
/**********************************************************************/

Q_GLOBAL_STATIC(KisTextureMaskInfoCache, s_instance)

KisTextureMaskInfoCache *KisTextureMaskInfoCache::instance()
{
    return s_instance;
}

KisTextureMaskInfoSP KisTextureMaskInfoCache::fetchCachedTextureInfo(KisTextureMaskInfoSP info) {
    QMutexLocker locker(&m_mutex);

    KisTextureMaskInfoSP &cachedInfo =
            info->levelOfDetail() > 0 ? m_lodInfo : m_mainInfo;

    if (!cachedInfo || *cachedInfo != *info) {
        cachedInfo = info;
        cachedInfo->recalculateMask();
    }

    return cachedInfo;
}
