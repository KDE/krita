/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISSCALINGSIZEBRUSH_H
#define KISSCALINGSIZEBRUSH_H

#include "kritabrush_export.h"
#include "kis_brush.h"


class BRUSH_EXPORT KisScalingSizeBrush : public KisBrush
{
public:

    KisScalingSizeBrush();
    KisScalingSizeBrush(const QString& filename);
    KisScalingSizeBrush(const KisScalingSizeBrush &rhs);

    qreal userEffectiveSize() const override;
    void setUserEffectiveSize(qreal value) override;
};

#endif // KISSCALINGSIZEBRUSH_H
