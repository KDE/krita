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


class KRITAIMAGE_EXPORT KisLodCapableLayerOffset
{
public:
    KisLodCapableLayerOffset(KisDefaultBoundsBaseSP defaultBounds);
    KisLodCapableLayerOffset(const KisLodCapableLayerOffset &rhs);
    KisLodCapableLayerOffset& operator=(const KisLodCapableLayerOffset &rhs);

    ~KisLodCapableLayerOffset();

    int x() const;
    int y() const;

    void setX(int value);
    void setY(int value);

    void syncLodOffset();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LOD_CAPABLE_LAYER_OFFSET_H */
