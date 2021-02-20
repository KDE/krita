/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_MULTIHAND_HELPER_H
#define __KIS_TOOL_MULTIHAND_HELPER_H

#include "kis_tool_freehand_helper.h"


class KRITAUI_EXPORT KisToolMultihandHelper : public KisToolFreehandHelper
{
    Q_OBJECT

public:
    KisToolMultihandHelper(KisPaintingInformationBuilder *infoBuilder,
                           KoCanvasResourceProvider *resourceManager,
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
