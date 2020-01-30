/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TOOL_LINE_HELPER_H
#define __KIS_TOOL_LINE_HELPER_H

#include "kis_tool_freehand_helper.h"


class KisToolLineHelper : private KisToolFreehandHelper
{
public:
    KisToolLineHelper(KisPaintingInformationBuilder *infoBuilder,
                      const KUndo2MagicString &transactionText);

    ~KisToolLineHelper() override;

    void setEnabled(bool value);
    void setUseSensors(bool value);

    void repaintLine(KoCanvasResourceProvider *resourceManager,
                     KisImageWSP image,
                     KisNodeSP node,
                     KisStrokesFacade *strokesFacade);

    void start(KoPointerEvent *event, KoCanvasResourceProvider *resourceManager);
    void addPoint(KoPointerEvent *event, const QPointF &overridePos = QPointF());
    void translatePoints(const QPointF &offset);
    void end();
    void cancel();
    void clearPoints();
    void clearPaint();

    using KisToolFreehandHelper::isRunning;

private:
    void adjustPointsToDDA(QVector<KisPaintInformation> &points);

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_TOOL_LINE_HELPER_H */
