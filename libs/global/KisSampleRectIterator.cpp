/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSampleRectIterator.h"

#include <kis_algebra_2d.h>

struct KisSampleRectIterator::HaltonSampler : QSharedData {
    HaltonSampler() : x(2), y(3) {}
    HaltonSampler(const HaltonSampler &rhs) = default;
    HaltonSampler(HaltonSampler &&rhs) = default;

    void step() {
        x.step();
        y.step();
    }

    KisAlgebra2D::HaltonSequenceGenerator x;
    KisAlgebra2D::HaltonSequenceGenerator y;
};

KisSampleRectIterator::KisSampleRectIterator() = default;
KisSampleRectIterator::~KisSampleRectIterator() = default;
KisSampleRectIterator::KisSampleRectIterator(const KisSampleRectIterator &rhs) = default;
KisSampleRectIterator::KisSampleRectIterator(KisSampleRectIterator &&rhs) = default;
KisSampleRectIterator& KisSampleRectIterator::operator=(const KisSampleRectIterator &rhs) = default;
KisSampleRectIterator& KisSampleRectIterator::operator=(KisSampleRectIterator &&rhs) = default;

KisSampleRectIterator::KisSampleRectIterator(const QRectF &rect)
    : m_rect(rect)
{
}

int KisSampleRectIterator::numSamples() const {
    return m_index + 1;
}

void KisSampleRectIterator::increment() {
    m_index++;

    if (m_index >= 9) {
        if (m_index == 9) {
            KIS_SAFE_ASSERT_RECOVER_RETURN(!m_sampler);
            m_sampler = new HaltonSampler();
        }
        m_sampler->step();
    }
}

QPointF KisSampleRectIterator::dereference() const {
    switch (m_index) {
    case 0:
        return m_rect.topLeft();
    case 1:
        return m_rect.topRight();
    case 2:
        return m_rect.bottomRight();
    case 3:
        return m_rect.bottomLeft();
    case 4:
        return 0.5 * (m_rect.bottomLeft() + m_rect.topLeft());
    case 5:
        return 0.5 * (m_rect.topRight() + m_rect.topLeft());
    case 6:
        return 0.5 * (m_rect.topRight() + m_rect.bottomRight());
    case 7:
        return 0.5 * (m_rect.bottomLeft() + m_rect.bottomRight());
    case 8:
        return m_rect.center();
    default:
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_sampler, m_rect.center());
        return KisAlgebra2D::relativeToAbsolute(
            QPointF(m_sampler->x.currentValue(), m_sampler->y.currentValue()),
            m_rect);
    }

    Q_UNREACHABLE();
}
