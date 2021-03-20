/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSharedQImagePyramid.h"

#include <QMutexLocker>

#include "kis_qimage_pyramid.h"
#include "kis_brush.h"


KisSharedQImagePyramid::KisSharedQImagePyramid()
{
}

KisSharedQImagePyramid::~KisSharedQImagePyramid()
{
}

const KisQImagePyramid *KisSharedQImagePyramid::pyramid(const KisBrush *brush) const
{
    const KisQImagePyramid * result = 0;

    if (m_cachedPyramidPointer) {
        result = m_cachedPyramidPointer;
    } else {
        QMutexLocker l(&m_mutex);

        if (!m_pyramid) {
            m_pyramid.reset(new KisQImagePyramid(brush->brushTipImage()));
        }

        m_cachedPyramidPointer = m_pyramid.data();
        result = m_pyramid.data();
    }

    return result;
}



bool KisSharedQImagePyramid::isNull() const
{
    QMutexLocker l(&m_mutex);
    return bool(m_pyramid);
}

