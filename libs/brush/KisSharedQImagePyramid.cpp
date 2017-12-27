/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

