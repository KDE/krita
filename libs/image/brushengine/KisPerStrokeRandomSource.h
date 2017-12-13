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

#ifndef KISPERSTROKERANDOMSOURCE_H
#define KISPERSTROKERANDOMSOURCE_H

#include <QScopedPointer>
#include "kis_shared.h"
#include "kis_shared_ptr.h"

#include "kritaimage_export.h"


class KRITAIMAGE_EXPORT KisPerStrokeRandomSource : public KisShared
{
public:
    KisPerStrokeRandomSource();
    KisPerStrokeRandomSource(const KisPerStrokeRandomSource &rhs);

    ~KisPerStrokeRandomSource();

    /**
     * Generates a random number in a range from \p min to \p max
     */
    int generate(const QString &key, int min, int max) const;

    /**
     * Generates a random number in a closed range [0; 1.0]
     */
    qreal generateNormalized(const QString &key) const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

class KisPerStrokeRandomSource;
typedef KisSharedPtr<KisPerStrokeRandomSource> KisPerStrokeRandomSourceSP;
typedef KisWeakSharedPtr<KisPerStrokeRandomSource> KisPerStrokeRandomSourceWSP;

#endif // KISPERSTROKERANDOMSOURCE_H
