/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PAINT_DEVICE_DATA_H
#define __KIS_PAINT_DEVICE_DATA_H

#include "KoAlwaysInline.h"


class KisPaintDeviceData
{
public:
    KisPaintDeviceData(KisPaintDevice *paintDevice)
        : m_cache(paintDevice),
          m_x(0), m_y(0),
          m_colorSpace(0),
          m_levelOfDetail(0)
        {
        }

    KisPaintDeviceData(const KisPaintDeviceData *rhs, bool cloneContent)
        : m_dataManager(cloneContent ?
                        new KisDataManager(*rhs->m_dataManager) :
                        new KisDataManager(rhs->m_dataManager->pixelSize(), rhs->m_dataManager->defaultPixel())),
          m_cache(rhs->m_cache),
          m_x(rhs->m_x),
          m_y(rhs->m_y),
          m_colorSpace(rhs->m_colorSpace),
          m_levelOfDetail(rhs->m_levelOfDetail)
        {
            m_cache.setupCache();
        }

    KUndo2Command* convertDataColorSpace(const KoColorSpace *cs);

    ALWAYS_INLINE KisDataManagerSP dataManager() const {
        return m_dataManager;
    }
    ALWAYS_INLINE void setDataManager(KisDataManagerSP value) {
        m_dataManager = value;
    }

    ALWAYS_INLINE void invalidateCache() {
        m_cache.invalidate();
    }

    ALWAYS_INLINE KisPaintDeviceCache* cache() {
        return &m_cache;
    }

    ALWAYS_INLINE qint32 x() const {
        return m_x;
    }
    ALWAYS_INLINE void setX(qint32 value) {
        m_x = value;
    }

    ALWAYS_INLINE qint32 y() const {
        return m_y;
    }
    ALWAYS_INLINE void setY(qint32 value) {
        m_y = value;
    }

    ALWAYS_INLINE const KoColorSpace* colorSpace() const {
        return m_colorSpace;
    }
    ALWAYS_INLINE void setColorSpace(const KoColorSpace *value) {
        m_colorSpace = value;
    }

    ALWAYS_INLINE qint32 levelOfDetail() const {
        return m_levelOfDetail;
    }
    ALWAYS_INLINE void setLevelOfDetail(qint32 value) {
        m_levelOfDetail = value;
    }

private:

    KisDataManagerSP m_dataManager;
    KisPaintDeviceCache m_cache;
    qint32 m_x;
    qint32 m_y;
    const KoColorSpace* m_colorSpace;
    qint32 m_levelOfDetail;
};

#endif /* __KIS_PAINT_DEVICE_DATA_H */
