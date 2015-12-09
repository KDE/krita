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

#ifndef __KIS_LAYER_UTILS_H
#define __KIS_LAYER_UTILS_H

#include "kis_types.h"
#include "kritaimage_export.h"

namespace KisMetaData
{
    class MergeStrategy;
}

namespace KisLayerUtils
{
    KRITAIMAGE_EXPORT void mergeDown(KisImageSP image, KisLayerSP layer, const KisMetaData::MergeStrategy* strategy);

    KRITAIMAGE_EXPORT QSet<int> fetchLayerFrames(KisNodeSP node);
    KRITAIMAGE_EXPORT QSet<int> fetchLayerFramesRecursive(KisNodeSP rootNode);

    KRITAIMAGE_EXPORT void mergeMultipleLayers(KisImageSP image, QList<KisNodeSP> mergedNodes, KisNodeSP putAfter);
    KRITAIMAGE_EXPORT bool tryMergeSelectionMasks(KisImageSP image, QList<KisNodeSP> mergedNodes, KisNodeSP putAfter);
};

#endif /* __KIS_LAYER_UTILS_H */
