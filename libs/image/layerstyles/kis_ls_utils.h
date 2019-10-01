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

#ifndef __KIS_LS_UTILS_H
#define __KIS_LS_UTILS_H

#include "kis_types.h"
#include "kritaimage_export.h"

#include "kis_lod_transform.h"


struct psd_layer_effects_context;
class psd_layer_effects_shadow_base;
struct psd_layer_effects_overlay_base;
class KisLayerStyleFilterEnvironment;
class KoPattern;
class KisMultipleProjection;


namespace KisLsUtils
{

    QRect growSelectionUniform(KisPixelSelectionSP selection, int growSize, const QRect &applyRect);

    KRITAIMAGE_EXPORT KisSelectionSP selectionFromAlphaChannel(KisPaintDeviceSP device,
                                                               const QRect &srcRect);

    void findEdge(KisPixelSelectionSP selection, const QRect &applyRect, const bool edgeHidden);
    QRect growRectFromRadius(const QRect &rc, int radius);
    void applyGaussianWithTransaction(KisPixelSelectionSP selection,
                                      const QRect &applyRect,
                                      qreal radius);

    static const int FULL_PERCENT_RANGE = 100;
    void adjustRange(KisPixelSelectionSP selection, const QRect &applyRect, const int range);

    void applyContourCorrection(KisPixelSelectionSP selection,
                                const QRect &applyRect,
                                const quint8 *lookup_table,
                                bool antiAliased,
                                bool edgeHidden);

    extern const int noiseNeedBorder;

    void applyNoise(KisPixelSelectionSP selection,
                    const QRect &applyRect,
                    int noise,
                    const psd_layer_effects_context *context,
                    const KisLayerStyleFilterEnvironment *env);

    void knockOutSelection(KisPixelSelectionSP selection,
                           KisPixelSelectionSP knockOutSelection,
                           const QRect &srcRect,
                           const QRect &dstRect,
                           const QRect &totalNeedRect,
                           const bool knockOutInverted);

    void fillPattern(KisPaintDeviceSP fillDevice,
                     const QRect &applyRect,
                     KisLayerStyleFilterEnvironment *env,
                     int scale,
                     KoPattern *pattern,
                     int horizontalPhase,
                     int verticalPhase,
                     bool alignWithLayer);

    void fillOverlayDevice(KisPaintDeviceSP fillDevice,
                           const QRect &applyRect,
                           const psd_layer_effects_overlay_base *config,
                           KisLayerStyleFilterEnvironment *env);

    void applyFinalSelection(const QString &projectionId,
                             KisSelectionSP baseSelection,
                             KisPaintDeviceSP srcDevice,
                             KisMultipleProjection *dst,
                             const QRect &srcRect,
                             const QRect &dstRect,
                             const psd_layer_effects_context *context,
                             const psd_layer_effects_shadow_base *config,
                             const KisLayerStyleFilterEnvironment *env);

    bool checkEffectEnabled(const psd_layer_effects_shadow_base *config, KisMultipleProjection *dst);

    template<class ConfigStruct>
    struct LodWrapper
    {
        LodWrapper(int lod,
                   const ConfigStruct *srcStruct)

            {
                if (lod > 0) {
                    storage.reset(new ConfigStruct(*srcStruct));

                    const qreal lodScale = KisLodTransform::lodToScale(lod);
                    storage->scaleLinearSizes(lodScale);

                    config = storage.data();
                } else {
                    config = srcStruct;
                }
            }

        const ConfigStruct *config;

    private:
        QScopedPointer<ConfigStruct> storage;
    };

}

#endif /* __KIS_LS_UTILS_H */
