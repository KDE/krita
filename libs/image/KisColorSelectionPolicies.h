/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCOLORSELECTIONPOLICIES
#define KISCOLORSELECTIONPOLICIES

#include <QStack>

#include <KoAlwaysInline.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <kis_random_accessor_ng.h>

namespace KisColorSelectionPolicies
{

class SlowDifferencePolicy
{
public:
    SlowDifferencePolicy(const KoColor &referenceColor, int threshold)
        : m_colorSpace(referenceColor.colorSpace())
        , m_referenceColor(referenceColor)
        , m_referenceColorPtr(m_referenceColor.data())
        , m_referenceColorIsTransparent(m_colorSpace->opacityU8(m_referenceColorPtr) == OPACITY_TRANSPARENT_U8)
        , m_threshold(threshold)
    {}

    ALWAYS_INLINE quint8 difference(const quint8 *colorPtr) const
    {
        if (m_threshold == 1) {
            const bool colorIsTransparent = (m_colorSpace->opacityU8(colorPtr) == OPACITY_TRANSPARENT_U8);
            if ((m_referenceColorIsTransparent && colorIsTransparent) ||
                memcmp(m_referenceColorPtr, colorPtr, m_colorSpace->pixelSize()) == 0) {
                return 0;
            }
            return quint8_MAX;
        }
        else {
            return m_colorSpace->differenceA(m_referenceColorPtr, colorPtr);
        }
    }

protected:
    const KoColorSpace *m_colorSpace;
    KoColor m_referenceColor;
    const quint8 *m_referenceColorPtr;
    const bool m_referenceColorIsTransparent;
    int m_threshold;
};

template <typename SrcPixelType>
class OptimizedDifferencePolicy : public SlowDifferencePolicy
{
public:
    OptimizedDifferencePolicy(const KoColor &referenceColor, int threshold)
        : SlowDifferencePolicy(referenceColor, threshold)
    {}

    ALWAYS_INLINE quint8 difference(const quint8 *colorPtr) const
    {
        HashKeyType key = *reinterpret_cast<const HashKeyType*>(colorPtr);

        quint8 result;

        typename HashType::iterator it = m_differences.find(key);

        if (it != m_differences.end()) {
            result = *it;
        } else {
            result = SlowDifferencePolicy::difference(colorPtr);
            m_differences.insert(key, result);
        }

        return result;
    }

protected:
    using HashKeyType = SrcPixelType;
    using HashType = QHash<HashKeyType, quint8>;

    mutable HashType m_differences;
};

class SlowColorOrTransparentDifferencePolicy : public SlowDifferencePolicy
{
public:
    SlowColorOrTransparentDifferencePolicy(const KoColor &referenceColor, int threshold)
        : SlowDifferencePolicy(referenceColor, threshold)
    {}

    ALWAYS_INLINE quint8 difference(const quint8 *colorPtr) const
    {
        if (m_threshold == 1) {
            if (memcmp(m_referenceColorPtr, colorPtr, m_colorSpace->pixelSize()) == 0 ||
                m_colorSpace->opacityU8(colorPtr) == 0) {
                return 0;
            }
            return quint8_MAX;
        }
        else {
            const quint8 colorDifference = m_colorSpace->difference(m_referenceColorPtr, colorPtr);
            const quint8 opacityDifference = m_colorSpace->opacityU8(colorPtr) * 100 / quint8_MAX;
            return qMin(colorDifference, opacityDifference);
        }
    }
};

template <typename SrcPixelType>
class OptimizedColorOrTransparentDifferencePolicy : public OptimizedDifferencePolicy<SrcPixelType>
{
public:
    OptimizedColorOrTransparentDifferencePolicy(const KoColor &referenceColor, int threshold)
        : OptimizedDifferencePolicy<SrcPixelType>(referenceColor, threshold)
    {}

    ALWAYS_INLINE quint8 difference(const quint8 *colorPtr) const
    {
        HashKeyType key = *reinterpret_cast<const HashKeyType*>(colorPtr);

        quint8 result;

        typename HashType::iterator it = this->m_differences.find(key);

        if (it != this->m_differences.end()) {
            result = *it;
        } else {
            const quint8 colorDifference = this->m_colorSpace->difference(this->m_referenceColorPtr, colorPtr);
            const quint8 opacityDifference = this->m_colorSpace->opacityU8(colorPtr) * 100 / quint8_MAX;
            result = qMin(colorDifference, opacityDifference);
            this->m_differences.insert(key, result);
        }

        return result;
    }

protected:
    using HashKeyType = typename OptimizedDifferencePolicy<SrcPixelType>::HashKeyType;
    using HashType = typename OptimizedDifferencePolicy<SrcPixelType>::HashType;
};

class SlowIsNonNullDifferencePolicy
{
public:
    SlowIsNonNullDifferencePolicy(int pixelSize)
        : m_pixelSize(pixelSize)
        , m_testColor(pixelSize, 0)
    {}

    ALWAYS_INLINE quint8 difference(const quint8 *colorPtr) const
    {
        if (memcmp(m_testColor.data(), colorPtr, m_pixelSize) == 0) {
            return 0;
        }
        return quint8_MAX;
    }

private:
    int m_pixelSize {0};
    QByteArray m_testColor;
};

template <typename SrcPixelType>
class OptimizedIsNonNullDifferencePolicy
{
public:
    ALWAYS_INLINE quint8 difference(const quint8 *colorPtr) const
    {
        const SrcPixelType *pixel = reinterpret_cast<const SrcPixelType*>(colorPtr);
        return *pixel == 0;
    }
};

class HardSelectionPolicy
{
public:
    HardSelectionPolicy(int threshold) : m_threshold(threshold) {}

    ALWAYS_INLINE quint8 opacityFromDifference(quint8 difference) const
    {
        return difference <= m_threshold ? MAX_SELECTED : MIN_SELECTED;
    }

protected:
    int m_threshold;
};

class SoftSelectionPolicy : public HardSelectionPolicy
{
public:
    SoftSelectionPolicy(int threshold, int softness)
        : HardSelectionPolicy(threshold)
        , m_softness(softness)
    {}

    ALWAYS_INLINE quint8 opacityFromDifference(quint8 difference) const
    {
        if (m_threshold == 0) {
            return MIN_SELECTED;
        }
        // Integer version of: (threshold - diff) / (threshold * softness)
        if (difference < m_threshold) {
            const int v = (m_threshold - difference) * MAX_SELECTED * 100 / (m_threshold * m_softness);
            return v > MAX_SELECTED ? MAX_SELECTED : v;
        } else {
            return MIN_SELECTED;
        }
    }

protected:
    int m_softness;
};

class SelectAllUntilColorHardSelectionPolicy: public HardSelectionPolicy
{
public:
    SelectAllUntilColorHardSelectionPolicy(int threshold)
        : HardSelectionPolicy(threshold)
    {}

    ALWAYS_INLINE quint8 opacityFromDifference(quint8 difference) const
    {
        return difference > m_threshold ? MAX_SELECTED : MIN_SELECTED;
    }
};

class SelectAllUntilColorSoftSelectionPolicy : public SoftSelectionPolicy
{
public:
    SelectAllUntilColorSoftSelectionPolicy(int threshold, int softness)
        : SoftSelectionPolicy(threshold, softness)
    {}

    ALWAYS_INLINE quint8 opacityFromDifference(quint8 difference) const
    {
        if (m_threshold == 0) {
            return MAX_SELECTED;
        }
        // Integer version of: 1 - ((1-threshold) - diff) / ((1-threshold) * softness)
        if (difference < m_threshold) {
            const int v = MAX_SELECTED - (m_threshold - difference) * MAX_SELECTED * 100 / (m_threshold * m_softness);
            return v < MIN_SELECTED ? MIN_SELECTED : v;
        } else {
            return MAX_SELECTED;
        }
    }
};

}

#endif
