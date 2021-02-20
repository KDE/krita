/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAZY_FILL_CAPACITY_MAP_H
#define __KIS_LAZY_FILL_CAPACITY_MAP_H

#include <KoColorSpace.h>

#include "kis_lazy_fill_graph.h"
#include "kis_paint_device.h"
#include "kis_types.h"
#include "kis_painter.h"
#include "kis_random_accessor_ng.h"
#include "kis_global.h"
#include <KisRegion.h>


class KisLazyFillCapacityMap
{
    typedef KisLazyFillCapacityMap type;
    typedef typename boost::graph_traits<KisLazyFillGraph>::vertex_descriptor VertexDescriptor;
    typedef typename boost::graph_traits<KisLazyFillGraph>::edge_descriptor EdgeDescriptor;

public:
    typedef EdgeDescriptor key_type;
    typedef int value_type;
    typedef const int& reference;
    typedef boost::readable_property_map_tag category;

    KisLazyFillCapacityMap(KisPaintDeviceSP mainImage,
                           KisPaintDeviceSP aLabelImage,
                           KisPaintDeviceSP bLabelImage,
                           KisPaintDeviceSP maskImage,
                           const QRect &boundingRect)
        : m_mainImage(mainImage),
          m_aLabelImage(aLabelImage),
          m_bLabelImage(bLabelImage),
          m_maskImage(maskImage),
          m_mainRect(boundingRect),
          m_aLabelRect(m_aLabelImage->exactBounds() & boundingRect),
          m_bLabelRect(m_bLabelImage->exactBounds() & boundingRect),
          m_colorSpace(mainImage->colorSpace()),
          m_pixelSize(m_colorSpace->pixelSize()),
          m_graph(m_mainRect,
                  m_aLabelImage->regionExact() & boundingRect,
                  m_bLabelImage->regionExact() & boundingRect)
    {
        KIS_ASSERT_RECOVER_NOOP(m_mainImage->colorSpace()->pixelSize() == 1);
        KIS_ASSERT_RECOVER_NOOP(m_aLabelImage->colorSpace()->pixelSize() == 1);
        KIS_ASSERT_RECOVER_NOOP(m_bLabelImage->colorSpace()->pixelSize() == 1);

        m_mainAccessor = m_mainImage->createRandomConstAccessorNG();
        m_aAccessor = m_aLabelImage->createRandomConstAccessorNG();
        m_bAccessor = m_bLabelImage->createRandomConstAccessorNG();
        m_maskAccessor = m_maskImage->createRandomConstAccessorNG();
        m_srcPixelBuf.resize(m_pixelSize);
    }

    int maxCapacity() const {
        const int k  = 2 * (m_mainRect.width() + m_mainRect.height());
        return k + 1;
    }

    friend value_type get(type &map,
                          const key_type &key)
        {
            VertexDescriptor src = source(key, map.m_graph);
            VertexDescriptor dst = target(key, map.m_graph);

            if (src.type == VertexDescriptor::NORMAL) {
                map.m_maskAccessor->moveTo(src.x, src.y);
                if (*map.m_maskAccessor->rawDataConst()) {
                    return 0;
                }
            }

            if (dst.type == VertexDescriptor::NORMAL) {
                map.m_maskAccessor->moveTo(dst.x, dst.y);
                if (*map.m_maskAccessor->rawDataConst()) {
                    return 0;
                }
            }

            bool srcLabelA = src.type == VertexDescriptor::LABEL_A;
            bool srcLabelB = src.type == VertexDescriptor::LABEL_B;
            bool dstLabelA = dst.type == VertexDescriptor::LABEL_A;
            bool dstLabelB = dst.type == VertexDescriptor::LABEL_B;

            if (srcLabelA || srcLabelB) {
                std::swap(src, dst);
                std::swap(srcLabelA, dstLabelA);
                std::swap(srcLabelB, dstLabelB);
            }

            Q_ASSERT(!srcLabelA && !srcLabelB);


            // TODO: precalculate!
            const int k  = 2 * (map.m_mainRect.width() + map.m_mainRect.height());

            static const int unitValue = 256;

            qreal value = 0.0;

            if (dstLabelA) {
                map.m_aAccessor->moveTo(src.x, src.y);
                const int i0 = *((quint8*)map.m_aAccessor->rawDataConst());
                value = i0 / 255.0 * k;

            } else if (dstLabelB) {
                map.m_bAccessor->moveTo(src.x, src.y);
                const int i0 = *((quint8*)map.m_bAccessor->rawDataConst());
                value = i0 / 255.0 * k;

            } else {
                map.m_mainAccessor->moveTo(src.x, src.y);
                memcpy(map.m_srcPixelBuf.data(), map.m_mainAccessor->rawDataConst(), map.m_pixelSize);
                map.m_mainAccessor->moveTo(dst.x, dst.y);

                //const quint8 diff = map.m_colorSpace->differenceA((quint8*)map.m_srcPixelBuf.data(), map.m_mainAccessor->rawDataConst());
                //const quint8 i0 = map.m_colorSpace->intensity8((quint8*)map.m_srcPixelBuf.data());
                //const quint8 i1 = map.m_colorSpace->intensity8(map.m_mainAccessor->rawDataConst());

                const quint8 i0 = *((quint8*)map.m_srcPixelBuf.data());
                const quint8 i1 = *map.m_mainAccessor->rawDataConst();

                const quint8 diff = qAbs(i1 - i0);

                const qreal diffPenalty = qBound(0.0, qreal(diff) / 10.0, 1.0);
                const qreal intensityPenalty = 1.0 - i1 / 255.0;

                const qreal totalPenalty = qMax(0.0 * diffPenalty, intensityPenalty);

                value = 1.0 + k * (1.0 - pow2(totalPenalty));
            }

            return value * unitValue;
        }

    KisLazyFillGraph& graph() {
        return m_graph;
    }

private:
    KisPaintDeviceSP m_mainImage;
    KisPaintDeviceSP m_aLabelImage;
    KisPaintDeviceSP m_bLabelImage;
    KisPaintDeviceSP m_maskImage;

    QRect m_mainRect;
    QRect m_aLabelRect;
    QRect m_bLabelRect;

    const KoColorSpace *m_colorSpace;
    int m_pixelSize;
    KisRandomConstAccessorSP m_mainAccessor;
    KisRandomConstAccessorSP m_aAccessor;
    KisRandomConstAccessorSP m_bAccessor;
    KisRandomConstAccessorSP m_maskAccessor;
    QByteArray m_srcPixelBuf;

    KisLazyFillGraph m_graph;
};

#endif /* __KIS_LAZY_FILL_CAPACITY_MAP_H */
