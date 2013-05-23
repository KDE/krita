/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006,2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_PERSPECTIVE_GRID_MANAGER_H
#define KIS_PERSPECTIVE_GRID_MANAGER_H

#include "kis_canvas_decoration.h"
#include <QPainter>

#include <krita_export.h>

class KAction;
class KActionCollection;
class KToggleAction;
class KisView2;

class KRITAUI_EXPORT KisPerspectiveGridManager : public KisCanvasDecoration
{
    Q_OBJECT
public:
    /** Create a perspective manager for this view
     */
    KisPerspectiveGridManager(KisView2 * parent);
    ~KisPerspectiveGridManager();
    void setup(KActionCollection * collection);
    /**
     * Call this function to start editing the grid, to disable display
     */
    void startEdition();
    /**
     * Call this function when the edition of the grid is finished. Trigger a redisplay of the perspective
     * grid if necesserary
     */
    void stopEdition();
public slots:
    void updateGUI();
    /**
     * Call this to remove all the perspective subgrids.
     */
    void clearPerspectiveGrid();
protected:
    void drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter,KisCanvas2* canvas = 0);

private:
    class LineWrapper;
    struct SubdivisionLinesInfo;

    SubdivisionLinesInfo getSubdivisionsInfo(const LineWrapper &l0,
                                             const LineWrapper &l1,
                                             const QPointF &focusPoint,
                                             int numSubdivisions);

    void drawSubdivisions(QPainter& gc, const SubdivisionLinesInfo &info);

private:
    KisView2* m_view;
    KToggleAction* m_toggleGrid;
    KAction* m_gridClear;
};

#endif
