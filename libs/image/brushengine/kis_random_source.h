/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_RANDOM_SOURCE_H
#define __KIS_RANDOM_SOURCE_H

#include <QScopedPointer>
#include "kis_shared.h"
#include "kis_shared_ptr.h"

#include "kritaimage_export.h"

/**
 * KisRandomSource is a special object that wraps around random number
 * generation routines.
 *
 * It has the following properties:
 *
 * 1) Two KisRandomSource objects will generate exactly the same sequences of
 *    numbers if created with the same seed.
 *
 * 2) After copy-construction or assignment the two objects will
 *    continue to generate exactly the same numbers. Imagine like the
 *    history got forked.
 *
 * 3) Copying of a KisRandomSource object is fast. It uses Tauss88
 *    algorithm to achieve this.
 */
class KRITAIMAGE_EXPORT KisRandomSource : public KisShared
{
public:
    KisRandomSource();
    KisRandomSource(int seed);
    KisRandomSource(const KisRandomSource &rhs);
    KisRandomSource& operator=(const KisRandomSource &rhs);

    ~KisRandomSource();

    qint64 min() const;
    qint64 max() const;

    /**
     * Generates a random number in a range from min() to max()
     */
    qint64 generate() const;

    /**
     * Generates a random number in a range from \p min to \p max
     */
    int generate(int min, int max) const;

    /**
     * Generates a random number in a closed range [0; 1.0]
     */
    qreal generateNormalized() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

class KisRandomSource;
typedef KisSharedPtr<KisRandomSource> KisRandomSourceSP;
typedef KisWeakSharedPtr<KisRandomSource> KisRandomSourceWSP;

#endif /* __KIS_RANDOM_SOURCE_H */
