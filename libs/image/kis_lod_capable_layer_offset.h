/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LOD_CAPABLE_LAYER_OFFSET_H
#define __KIS_LOD_CAPABLE_LAYER_OFFSET_H

#include <QScopedPointer>
#include "kritaimage_export.h"
#include "kis_default_bounds_base.h"


namespace KisLodSwitchingWrapperDetail
{
/**
 * By default forward the syncing request to the wrapped
 * object itself.
 */
template <typename T>
inline T syncLodNValue(const T &value, int lod) {
    return value.syncLodNValue(lod);
}

/**
 * Otherwise just use specialized functions for that
 */
KRITAIMAGE_EXPORT
QPoint syncLodNValue(const QPoint &value, int lod);

} // namespace KisLodSwitchingWrapperDetail

template <typename T>
class KisLodSwitchingWrapper
{
public:
    KisLodSwitchingWrapper(T &&initialValue, KisDefaultBoundsBaseSP defaultBounds)
        : m_defaultBounds(defaultBounds),
          m_data(std::forward<T>(initialValue)),
          m_lodNData(KisLodSwitchingWrapperDetail::syncLodNValue(m_data, defaultBounds->currentLevelOfDetail()))
    {
    }

    KisLodSwitchingWrapper(KisDefaultBoundsBaseSP defaultBounds)
        : m_defaultBounds(defaultBounds)
    {
    }

    KisLodSwitchingWrapper(const KisLodSwitchingWrapper &rhs)
        : m_defaultBounds(rhs.m_defaultBounds)
        , m_data(rhs.m_data)
        , m_lodNData(rhs.m_lodNData)
    {
    }

    KisLodSwitchingWrapper& operator=(const KisLodSwitchingWrapper &rhs)
    {
        if (this != &rhs) {
            m_defaultBounds = rhs.m_defaultBounds;
            m_data = rhs.m_data;
            m_lodNData = rhs.m_lodNData;
        }

        return *this;
    }


    KisDefaultBoundsBaseSP defaultBounds() const {
        return m_defaultBounds;
    }

    void setDefaultBounds(KisDefaultBoundsBaseSP defaultBounds) {
        m_defaultBounds = defaultBounds;
    }

    const T *operator->() const noexcept
    {
        return m_defaultBounds->currentLevelOfDetail() > 0 ? &m_lodNData : &m_data;
    }

    T *operator->() noexcept
    {
        return m_defaultBounds->currentLevelOfDetail() > 0 ? &m_lodNData : &m_data;
    }

    const T& operator*() const noexcept
    {
        return m_defaultBounds->currentLevelOfDetail() > 0 ? m_lodNData : m_data;
    }

    T& operator*() noexcept
    {
        return m_defaultBounds->currentLevelOfDetail() > 0 ? m_lodNData : m_data;
    }

    void syncLodCache() {
        m_lodNData =
            KisLodSwitchingWrapperDetail::syncLodNValue(
                m_data, m_defaultBounds->currentLevelOfDetail());
    }

    using LodState = std::pair<int, T>;

    void setLodState(const LodState &state) {
        (state.first > 0 ? m_lodNData : m_data) = state.second;
    }

    LodState lodState() const {
        return std::make_pair(m_defaultBounds->currentLevelOfDetail(), *(*this));
    }

    operator LodState() const {
        return lodState();
    }

    KisLodSwitchingWrapper& operator=(const LodState &rhs)
    {
        setLodState(rhs);
        return *this;
    }


private:
    KisDefaultBoundsBaseSP m_defaultBounds;
    T m_data;
    T m_lodNData;
};

using KisLodCapableLayerOffset = KisLodSwitchingWrapper<QPoint>;

#endif /* __KIS_LOD_CAPABLE_LAYER_OFFSET_H */
