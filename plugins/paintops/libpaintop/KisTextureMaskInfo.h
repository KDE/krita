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

#ifndef KISTEXTUREMASKINFO_H
#define KISTEXTUREMASKINFO_H


#include <kis_paint_device.h>
#include <QSharedPointer>
#include <QMutex>


#include <boost/operators.hpp>

#include <KoPattern.h>

class KisTextureMaskInfo;

class KisTextureMaskInfo : public boost::equality_comparable<KisTextureMaskInfo>
{
public:
    KisTextureMaskInfo(int levelOfDetail);
    KisTextureMaskInfo(const KisTextureMaskInfo &rhs);

    ~KisTextureMaskInfo();

    friend bool operator==(const KisTextureMaskInfo &lhs, const KisTextureMaskInfo &rhs);

    KisTextureMaskInfo& operator=(const KisTextureMaskInfo &rhs);

    int levelOfDetail() const;

    bool hasMask() const;

    KisPaintDeviceSP mask();

    QRect maskBounds() const;

    bool fillProperties(const KisPropertiesConfigurationSP setting);

    void recalculateMask();

private:
    int m_levelOfDetail = 0;

    KoPatternSP m_pattern = 0;

    qreal m_scale = 1.0;
    qreal m_brightness = 0.0;
    qreal m_contrast = 1.0;
    bool m_invert = false;

    int m_cutoffLeft = 0;
    int m_cutoffRight = 255;
    int m_cutoffPolicy = 0;

    KisPaintDeviceSP m_mask;
    QRect m_maskBounds;

};

typedef QSharedPointer<KisTextureMaskInfo> KisTextureMaskInfoSP;

struct KisTextureMaskInfoCache
{
    static KisTextureMaskInfoCache *instance();
    KisTextureMaskInfoSP fetchCachedTextureInfo(KisTextureMaskInfoSP info);

private:
    QMutex m_mutex;
    QSharedPointer<KisTextureMaskInfo> m_lodInfo;
    QSharedPointer<KisTextureMaskInfo> m_mainInfo;
};

#endif // KISTEXTUREMASKINFO_H
