/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
