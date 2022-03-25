/*
 *  SPDX-FileCopyrightText: 2005 Michael Thaler
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KIS_SELECTION_FILTERS_H
#define KIS_SELECTION_FILTERS_H

#include "kis_types.h"
#include "kritaimage_export.h"
#include "kis_default_bounds_base.h"

#include <QRect>
#include <QString>

class KUndo2MagicString;


class KRITAIMAGE_EXPORT KisSelectionFilter
{
public:
    virtual ~KisSelectionFilter();

    virtual void process(KisPixelSelectionSP pixelSelection,
                         const QRect &rect) = 0;

    virtual KUndo2MagicString name();
    virtual QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds);

protected:
    void computeBorder(qint32  *circ, qint32  xradius, qint32  yradius);

    void rotatePointers(quint8  **p, quint32 n);

    void computeTransition(quint8* transition, quint8** buf, qint32 width);
};

class KRITAIMAGE_EXPORT KisErodeSelectionFilter : public KisSelectionFilter
{
public:
    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;
};

class KRITAIMAGE_EXPORT KisDilateSelectionFilter : public KisSelectionFilter
{
public:
    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;
};

class KRITAIMAGE_EXPORT KisBorderSelectionFilter : public KisSelectionFilter
{
public:
    KisBorderSelectionFilter(qint32 xRadius, qint32 yRadius, bool fade);

    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;

private:
    qint32 m_xRadius;
    qint32 m_yRadius;
    bool m_antialiasing;
};

class KRITAIMAGE_EXPORT KisFeatherSelectionFilter : public KisSelectionFilter
{
public:
    KisFeatherSelectionFilter(qint32 radius);

    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;
private:
    qint32 m_radius;
};

class KRITAIMAGE_EXPORT KisGrowSelectionFilter : public KisSelectionFilter
{
public:
    KisGrowSelectionFilter(qint32 xRadius, qint32 yRadius);

    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;

private:
    qint32 m_xRadius;
    qint32 m_yRadius;
};

class KRITAIMAGE_EXPORT KisShrinkSelectionFilter : public KisSelectionFilter
{
public:
    KisShrinkSelectionFilter(qint32 xRadius, qint32 yRadius, bool edgeLock);

    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;

private:
    qint32 m_xRadius;
    qint32 m_yRadius;
    qint32 m_edgeLock;
};

class KRITAIMAGE_EXPORT KisSmoothSelectionFilter : public KisSelectionFilter
{
public:
    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;
};

class KRITAIMAGE_EXPORT KisInvertSelectionFilter : public KisSelectionFilter
{
public:
    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;
};

/**
 * @brief AntiAlias filter for selections inspired by FXAA 
 */
class KRITAIMAGE_EXPORT KisAntiAliasSelectionFilter : public KisSelectionFilter
{
public:
    KUndo2MagicString name() override;
    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;

private:
    /**
     * @brief Edges with gradient less than this value will not be antiAliasied
     */
    static constexpr qint32 edgeThreshold {4};
    /**
     * @brief Number of steps to jump when searching for one of the ends of the
     *        antiAliased span.
     */
    static constexpr qint32 numSteps {30};
    /**
     * @brief This array of @ref numSteps size holds the number of pixels to
     *        jump in each step.
     */
    static constexpr qint32 offsets[numSteps] {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2
    };
    /**
     * @brief The size of the border added internally to the left and right of
     *        the scanline buffer so that we can read outside the selection rect
     *        without problems. It must be equal to the largest value in @ref offsets. 
     */
    static constexpr qint32 horizontalBorderSize {2};
    /**
     * @brief The size of the border added internally to the top and bottom of
     *        the scanline buffer so that we can read outside the selection rect
     *        without problems. It must be equal to the sum of all values in @ref offsets. 
     */
    static constexpr qint32 verticalBorderSize {40};
    /**
     * @brief Number of scanlines in the internal buffer.
     */
    static constexpr qint32 numberOfScanlines {2 * verticalBorderSize + 1};
    /**
     * @brief Offset of the current scanline in the buffer (The middle scanline).
     */
    static constexpr qint32 currentScanlineIndex {verticalBorderSize};
    /**
     * @brief Get a interpolation value to linearly interpolate the current
     *        pixel with its edge neighbor.
     * @return true if we must apply the interpolation. false otherwise.
     */
    bool getInterpolationValue(qint32 negativeSpanEndDistance, qint32 positiveSpanEndDistance,
                               qint32 negativePixelDiff, qint32 positivePixelDiff, qint32 currentPixelDiff,
                               bool negativeSpanExtremeValid, bool positiveSpanExtremeValid,
                               qint32 *interpolationValue) const;
    /**
     * @brief Get the extreme point of the span for the current pixel in the 
     *        given direction
     */
    void findSpanExtreme(quint8 **scanlines, qint32 x, qint32 pixelOffset,
                         qint32 rowMultiplier, qint32 colMultiplier, qint32 direction,
                         qint32 pixelAvg, qint32 scaledGradient, qint32 currentPixelDiff,
                         qint32 *spanEndDistance, qint32 *pixelDiff, bool *spanExtremeValidType) const;
    /**
     * @brief Get the extreme points of the span for the current pixel
     */
    void findSpanExtremes(quint8 **scanlines, qint32 x, qint32 pixelOffset,
                          qint32 rowMultiplier, qint32 colMultiplier,
                          qint32 pixelAvg, qint32 scaledGradient, qint32 currentPixelDiff,
                          qint32 *negativeSpanEndDistance, qint32 *positiveSpanEndDistance,
                          qint32 *negativePixelDiff, qint32 *positivePixelDiff,
                          bool *negativeSpanExtremeValid, bool *positiveSpanExtremeValid) const;
};

#endif // KIS_SELECTION_FILTERS_H
