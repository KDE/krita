/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTEXTUREMASKINFO_H
#define KISTEXTUREMASKINFO_H


#include <kis_paint_device.h>
#include <QSharedPointer>
#include <QMutex>


#include <boost/operators.hpp>

#include <KoPattern.h>

class KisTextureMaskInfo;
class KisResourcesInterface;

class KisTextureMaskInfo : public boost::equality_comparable<KisTextureMaskInfo>
{
public:
    KisTextureMaskInfo(int levelOfDetail, bool preserveAlpha);
    KisTextureMaskInfo(const KisTextureMaskInfo &rhs);

    ~KisTextureMaskInfo();

    friend bool operator==(const KisTextureMaskInfo &lhs, const KisTextureMaskInfo &rhs);

    KisTextureMaskInfo& operator=(const KisTextureMaskInfo &rhs);

    int levelOfDetail() const;

    bool hasMask() const;

    KisPaintDeviceSP mask();

    QRect maskBounds() const;

    bool fillProperties(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface);

    void recalculateMask();

    bool hasAlpha();

private:
    int m_levelOfDetail = 0;
    bool m_preserveAlpha = false;

    KoPatternSP m_pattern = 0;

    qreal m_scale = 1.0;
    qreal m_brightness = 0.0;
    qreal m_contrast = 1.0;
    qreal m_neutralPoint = 0.5;
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
