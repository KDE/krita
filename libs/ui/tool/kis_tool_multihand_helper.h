/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TOOL_MULTIHAND_HELPER_H
#define __KIS_TOOL_MULTIHAND_HELPER_H

#include "kis_tool_freehand_helper.h"


class KRITAUI_EXPORT KisToolMultihandHelper : public KisToolFreehandHelper
{
    Q_OBJECT

public:
    KisToolMultihandHelper(KisPaintingInformationBuilder *infoBuilder,
                           const KUndo2MagicString &transactionText);
    ~KisToolMultihandHelper() override;

    void setupTransformations(const QVector<QTransform> &transformations);

protected:
    void createPainters(QVector<KisFreehandStrokeInfo*> &strokeInfos,
                        const KisDistanceInformation &startDist) override;

    void paintAt(const KisPaintInformation &pi) override;

    void paintLine(const KisPaintInformation &pi1,
                   const KisPaintInformation &pi2) override;

    void paintBezierCurve(const KisPaintInformation &pi1,
                          const QPointF &control1,
                          const QPointF &control2,
                          const KisPaintInformation &pi2) override;

    using KisToolFreehandHelper::paintAt;
    using KisToolFreehandHelper::paintLine;
    using KisToolFreehandHelper::paintBezierCurve;

private:
    struct Private;
    Private * const d;
};

#endif /* __KIS_TOOL_MULTIHAND_HELPER_H */
