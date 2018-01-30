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

#ifndef KISSHAREDQIMAGEPYRAMID_H
#define KISSHAREDQIMAGEPYRAMID_H

#include "kritabrush_export.h"

#include <QSharedPointer>
#include <QMutex>
#include <QAtomicPointer>


class KisQImagePyramid;
class KisBrush;

/**
 * A special class for storing the shared brushes pyramid among different brushes,
 * which can be used by different threads. All the calls to pyramid() are thread-safe.
 *
 * Please note, that one cannot alter the pyramid. If the brush alters the pyramid,
 * it should just detach from this object and create a new, unshared one.
 */

class BRUSH_EXPORT KisSharedQImagePyramid
{
public:
    KisSharedQImagePyramid();
    ~KisSharedQImagePyramid();

public:

    // lazy create and return the pyramid
    const KisQImagePyramid* pyramid(const KisBrush *brush) const;

    // return true if the pyramid is already prepared
    bool isNull() const;

private:
    mutable QMutex m_mutex;

    mutable QSharedPointer<const KisQImagePyramid> m_pyramid;
    mutable QAtomicPointer<const KisQImagePyramid> m_cachedPyramidPointer;
};

#endif // KISSHAREDQIMAGEPYRAMID_H
