/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LOD_TRANSFORM_BASE_H
#define KIS_LOD_TRANSFORM_BASE_H


#include <QtCore/qmath.h>
#include <QTransform>


#include <kritaglobal_export.h>

class KRITAGLOBAL_EXPORT KisLodTransformBase {
public:
    KisLodTransformBase(int levelOfDetail) {
        qreal scale = lodToScale(levelOfDetail);
        m_transform = QTransform::fromScale(scale, scale);
        m_levelOfDetail = levelOfDetail;
    }



    static int scaleToLod(qreal scale, int maxLod) {
        return qMin(maxLod, qMax(0, qFloor(std::log2(1.0 / scale))));
    }

    static qreal lodToScale(int levelOfDetail) {
        return levelOfDetail > 0 ? 1.0 / (1 << qMax(0, levelOfDetail)) : 1.0;
    }

    static qreal lodToInvScale(int levelOfDetail) {
        return 1 << qMax(0, levelOfDetail);
    }

    template <class PaintDeviceTypeSP>
    static qreal lodToScale(PaintDeviceTypeSP device) {
        return lodToScale(device->defaultBounds()->currentLevelOfDetail());
    }

    QRectF map(const QRectF &rc) const {
        return m_transform.mapRect(rc);
    }

    QRect map(const QRect &rc) const {
        return m_transform.mapRect(rc);
    }

    QRectF mapInverted(const QRectF &rc) const {
        return m_transform.inverted().mapRect(rc);
    }

    QRect mapInverted(const QRect &rc) const {
        return m_transform.inverted().mapRect(rc);
    }



    template <class T>
    T map(const T &object) const {
        return m_transform.map(object);
    }

    static inline QRect alignedRect(const QRect &srcRect, int lod)
    {
        qint32 alignment = 1 << lod;

        qint32 x1, y1, x2, y2;
        srcRect.getCoords(&x1, &y1, &x2, &y2);

        alignByPow2Lo(x1, alignment);
        alignByPow2Lo(y1, alignment);

        /**
         * Here is a workaround of Qt's QRect::right()/bottom()
         * "historical reasons". It should be one pixel smaller
         * than actual right/bottom position
         */
        alignByPow2ButOneHi(x2, alignment);
        alignByPow2ButOneHi(y2, alignment);

        QRect rect;
        rect.setCoords(x1, y1, x2, y2);

        return rect;
    }

    static inline QRect scaledRect(const QRect &srcRect, int lod) {
        qint32 x1, y1, x2, y2;
        srcRect.getCoords(&x1, &y1, &x2, &y2);

        KIS_ASSERT_RECOVER_NOOP(!(x1 & 1));
        KIS_ASSERT_RECOVER_NOOP(!(y1 & 1));
        KIS_ASSERT_RECOVER_NOOP(!((x2 + 1) & 1));
        KIS_ASSERT_RECOVER_NOOP(!((y2 + 1) & 1));

        x1 = divideSafe(x1, lod);
        y1 = divideSafe(y1, lod);
        x2 = divideSafe(x2 + 1, lod) - 1;
        y2 = divideSafe(y2 + 1, lod) - 1;

        QRect rect;
        rect.setCoords(x1, y1, x2, y2);

        return rect;
    }

    static QRect upscaledRect(const QRect &srcRect, int lod) {
        qint32 x1, y1, x2, y2;
        srcRect.getCoords(&x1, &y1, &x2, &y2);

        x2++;
        y2++;

        x1 <<= lod;
        y1 <<= lod;
        x2 <<= lod;
        y2 <<= lod;

        x2--;
        y2--;

        QRect rect;
        rect.setCoords(x1, y1, x2, y2);

        return rect;
    }

    static inline int coordToLodCoord(int x, int lod) {
        return divideSafe(x, lod);
    }

    QTransform transform() const {
        return m_transform;
    }

private:
    friend class KisRectsGrid;
    /**
     * Aligns @value to the lowest integer not smaller than @value and
     * that is, increased by one, a divident of alignment
     */
    static inline void alignByPow2ButOneHi(qint32 &value, qint32 alignment)
    {
        qint32 mask = alignment - 1;
        value |= mask;
    }

    /**
     * Aligns @value to the highest integer not exceeding @value and
     * that is a divident of @alignment
     */
    static inline void alignByPow2Lo(qint32 &value, qint32 alignment)
    {
        qint32 mask = alignment - 1;
         value &= ~mask;
    }

    static inline int divideSafe(int x, int lod) {
        return x > 0 ? x >> lod : -( -x >> lod);
    }

protected:
    QTransform m_transform;
    int m_levelOfDetail;
};

class KisLodTransformScalar {
public:
    KisLodTransformScalar(int lod) {
        m_scale = KisLodTransformBase::lodToScale(lod);
    }

    template <class PaintDeviceTypeSP>
    KisLodTransformScalar(PaintDeviceTypeSP device) {
        m_scale = KisLodTransformBase::lodToScale(device->defaultBounds()->currentLevelOfDetail());
    }

    qreal scale(qreal value) const {
        return m_scale * value;
    }

private:
    qreal m_scale;
};

#endif // KIS_LOD_TRANSFORM_BASE_H
