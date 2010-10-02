/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_IMAGE_CONFIG_H_
#define KIS_IMAGE_CONFIG_H_

#include <kconfiggroup.h>
#include "krita_export.h"


class KRITAIMAGE_EXPORT KisImageConfig
{
public:
    KisImageConfig();
    ~KisImageConfig();

    bool useUpdateScheduler() const;
    void setUseUpdateScheduler(bool useUpdateScheduler);

    int updatePatchHeight() const;
    void setUpdatePatchHeight(int value);
    int updatePatchWidth() const;
    void setUpdatePatchWidth(int value);

    qreal maxCollectAlpha() const;
    qreal maxMergeAlpha() const;
    qreal maxMergeCollectAlpha() const;

    int maxSwapSize() const;
    void setMaxSwapSize(int value);

    int swapSlabSize() const;
    void setSwapSlabSize(int value);

    int swapWindowSize() const;
    void setSwapWindowSize(int value);

    int memoryHardLimit() const; // MiB
    int memorySoftLimit() const; // MiB

    qreal memoryHardLimitPercent() const; // % of total RAM
    qreal memorySoftLimitPercent() const; // % of total RAM
    void setMemoryHardLimitPercent(qreal value);
    void setMemorySoftLimitPercent(qreal value);

    static int totalRAM(); // MiB

private:
    Q_DISABLE_COPY(KisImageConfig);

private:
    KConfigGroup m_config;
};


#endif /* KIS_IMAGE_CONFIG_H_ */

