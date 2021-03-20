/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BRUSHES_PIPE_H
#define __KIS_BRUSHES_PIPE_H

#include <kis_fixed_paint_device.h>
#include <kis_brush.h>

template<class BrushType>
class KisBrushesPipe
{
public:
    KisBrushesPipe() {
    }

    KisBrushesPipe(const KisBrushesPipe &rhs) {
        m_brushes.clear();
        Q_FOREACH (QSharedPointer<BrushType> brush, rhs.m_brushes) {
            KoResourceSP clonedBrush = brush->clone();
            QSharedPointer<BrushType> actualClonedBrush = clonedBrush.dynamicCast<BrushType>();
            m_brushes.append(actualClonedBrush );
            KIS_ASSERT_RECOVER(clonedBrush) {continue;}
        }
    }

    virtual ~KisBrushesPipe() {
    }

    virtual void clear() {
        m_brushes.clear();
    }

    QSharedPointer<BrushType> firstBrush() const {
        return m_brushes.first();
    }

    QSharedPointer<BrushType> lastBrush() const {
        return m_brushes.last();
    }


    QSharedPointer<BrushType> currentBrush(const KisPaintInformation& info) {
        Q_UNUSED(info);
        return !m_brushes.isEmpty() ? m_brushes.at(currentBrushIndex()) : 0;
    }

    qint32 maskWidth(KisDabShape const& shape, double subPixelX, double subPixelY, const KisPaintInformation& info) {
        QSharedPointer<BrushType> brush = currentBrush(info);
        return brush ? brush->maskWidth(shape, subPixelX, subPixelY, info) : 0;
    }

    qint32 maskHeight(KisDabShape const& shape, double subPixelX, double subPixelY, const KisPaintInformation& info) {
        QSharedPointer<BrushType> brush = currentBrush(info);
        return brush ? brush->maskHeight(shape, subPixelX, subPixelY, info) : 0;
    }

    void setAngle(qreal angle) {
        Q_FOREACH (QSharedPointer<BrushType> brush, m_brushes) {
            brush->setAngle(angle);
        }
    }

    void setScale(qreal scale) {
        Q_FOREACH (QSharedPointer<BrushType> brush, m_brushes) {
            brush->setScale(scale);
        }
    }

    void setSpacing(double spacing) {
        Q_FOREACH (QSharedPointer<BrushType> brush, m_brushes) {
            brush->setSpacing(spacing);
        }
    }

    bool isImageType() const {
        Q_FOREACH (QSharedPointer<BrushType> brush, m_brushes) {
            if (brush->isImageType()) return true;
        }
        return false;
    }

    bool hasColorAndTransparency() const {
        Q_FOREACH (QSharedPointer<BrushType> brush, m_brushes) {
            if (brush->hasColorAndTransparency()) return true;
        }
        return false;
    }

    void setBrushApplication(enumBrushApplication brushApplication) const {
        Q_FOREACH(QSharedPointer<BrushType> brush, m_brushes) {
            brush->setBrushApplication(brushApplication);
        }
    }

    void setGradient(KoAbstractGradientSP gradient) const {
        Q_FOREACH(QSharedPointer<BrushType> brush, m_brushes) {
            brush->setGradient(gradient);
        }
    }

    void prepareForSeqNo(const KisPaintInformation& info, int seqNo) {
        chooseNextBrush(info);
        updateBrushIndexes(info, seqNo);
    }

    void generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation,
            KisDabShape const& shape,
            const KisPaintInformation& info,
            double subPixelX , double subPixelY,
            qreal softnessFactor, qreal lightnessStrength = DEFAULT_LIGHTNESS_STRENGTH) {

        QSharedPointer<BrushType> brush = currentBrush(info);
        if (!brush) return;


        brush->generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, shape, info, subPixelX, subPixelY, softnessFactor, lightnessStrength);
    }

    KisFixedPaintDeviceSP paintDevice(const KoColorSpace * colorSpace,
                                      KisDabShape const& shape,
                                      const KisPaintInformation& info,
                                      double subPixelX, double subPixelY) {

        QSharedPointer<BrushType> brush = currentBrush(info);
        if (!brush) return 0;

        KisFixedPaintDeviceSP device = brush->paintDevice(colorSpace, shape, info, subPixelX, subPixelY);
        return device;
    }

    QVector<QSharedPointer<BrushType>> brushes() {
        return m_brushes;
    }

    void testingSelectNextBrush(const KisPaintInformation& info) {
        (void) chooseNextBrush(info);
        updateBrushIndexes(info, -1);
    }

    /**
     * Is called by the paint op when a paintop starts a stroke. The
     * brushes are shared among different strokes, so sometimes the
     * brush should be reset.
     */
    virtual void notifyStrokeStarted() = 0;

protected:
    void addBrush(QSharedPointer<BrushType> brush) {
        m_brushes.append(brush);
    }

    int sizeBrush() {
        return m_brushes.size();
    }

    /**
     * Returns the index of the next brush that corresponds to the current
     * values of \p info. This method is called *before* the dab is
     * actually painted.
     *
     */
    virtual int chooseNextBrush(const KisPaintInformation& info) = 0;

    /**
     * Returns the current index of the brush
     * This method is called *before* the dab is
     * actually painted.
     *
     * The method is const, so no internal counters of the brush should
     * change during its execution
     */
    virtual int currentBrushIndex() = 0;

    /**
     * Updates internal counters of the brush *after* a dab has been
     * painted on the canvas. Some incremental switching of the brushes
     * may me implemented in this method.
     *
     * If \p seqNo is equal or greater than zero, then incremental iteration is
     * overridden by this seqNo value
     */
    virtual void updateBrushIndexes(const KisPaintInformation& info, int seqNo) = 0;

protected:
    QVector<QSharedPointer<BrushType>> m_brushes;
};

#endif /* __KIS_BRUSHES_PIPE_H */
