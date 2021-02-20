/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
