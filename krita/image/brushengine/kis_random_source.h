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

    qint64 generate() const;

    int generate(int min, int max) const;
    qreal generateNormalized() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

class KisRandomSource;
typedef KisSharedPtr<KisRandomSource> KisRandomSourceSP;
typedef KisWeakSharedPtr<KisRandomSource> KisRandomSourceWSP;

#endif /* __KIS_RANDOM_SOURCE_H */
