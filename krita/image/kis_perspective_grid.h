/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_PERSPECTIVE_GRID_H
#define KIS_PERSPECTIVE_GRID_H

#include <QList>
#include <QPointF>

#include <kis_perspective_math.h>
#include <kis_shared.h>
#include <kis_shared_ptr.h>
#include <krita_export.h>

class KisPerspectiveGridNode;
typedef KisSharedPtr<KisPerspectiveGridNode> KisPerspectiveGridNodeSP;
class KisSubPerspectiveGrid;

/**
 * This class is the corner of sub perspective grid, it can be shared between various grid.
 */
class KRITAIMAGE_EXPORT KisPerspectiveGridNode : public QPointF, public KisShared
{
    friend class KisSubPerspectiveGrid;
public:
    KisPerspectiveGridNode(double x, double y);
    KisPerspectiveGridNode(QPointF p);
    KisPerspectiveGridNode(const KisPerspectiveGridNode&);
    ~KisPerspectiveGridNode();
    void mergeWith(KisPerspectiveGridNodeSP);
private:
    void registerSubPerspectiveGrid(KisSubPerspectiveGrid*);
    void unRegisterSubPerspectiveGrid(KisSubPerspectiveGrid*);
    bool containsSubPerspectiveGrid(KisSubPerspectiveGrid*);
private:
    struct Private;
    Private* const d;
};

/**
 * This class contains the information of a sub perspective grid.
 */
class KRITAIMAGE_EXPORT KisSubPerspectiveGrid
{
public:
    KisSubPerspectiveGrid(KisPerspectiveGridNodeSP topLeft, KisPerspectiveGridNodeSP topRight, KisPerspectiveGridNodeSP bottomRight, KisPerspectiveGridNodeSP bottomLeft);
    ~KisSubPerspectiveGrid();

    QPointF topBottomVanishingPoint();
    QPointF leftRightVanishingPoint();

    /**
     * @return the top left corner of the grid
     */
    const KisPerspectiveGridNodeSP topLeft() const;
    KisPerspectiveGridNodeSP topLeft();
    void setTopLeft(KisPerspectiveGridNodeSP);
    const KisPerspectiveGridNodeSP topRight() const;
    KisPerspectiveGridNodeSP topRight();
    void setTopRight(KisPerspectiveGridNodeSP);
    const KisPerspectiveGridNodeSP bottomLeft() const;
    KisPerspectiveGridNodeSP bottomLeft();
    void setBottomLeft(KisPerspectiveGridNodeSP);
    const KisPerspectiveGridNodeSP bottomRight() const;
    KisPerspectiveGridNodeSP bottomRight();
    void setBottomRight(KisPerspectiveGridNodeSP);
    int subdivisions() const;

    /**
     * @return the center of the sub perspective grid
     */
    QPointF center() const;

    /**
     * Return the index of the subgrid, the value is automatically set when the KisSubPerspectiveGrid, it is useful for
     * drawing the perspective grid, to avoid drawing twice the same border, or points
     */
    int index() const;

    /**
     * @return true if the point p is contain by the grid
     */
    bool contains(const QPointF p) const;
private:
    inline QPointF computeVanishingPoint(KisPerspectiveGridNodeSP p11, KisPerspectiveGridNodeSP p12, KisPerspectiveGridNodeSP p21, KisPerspectiveGridNodeSP p22);
private:
    struct Private;
    Private* const d;
};

/**
 * This class contains the list of sub perspective grid
 */
class KRITAIMAGE_EXPORT KisPerspectiveGrid
{
public:
    KisPerspectiveGrid();
    ~KisPerspectiveGrid();
    /**
     * @return false if the grid wasn't added, note that subgrids must be attached to an other grid, except if it's the first grid
     */
    bool addNewSubGrid(KisSubPerspectiveGrid* ng);
    QList<KisSubPerspectiveGrid*>::const_iterator begin() const;
    QList<KisSubPerspectiveGrid*>::const_iterator end() const;
    bool hasSubGrids() const;
    void clearSubGrids();
    int countSubGrids() const;
    /**
     * Delete the grid given as argument and remove it from the list of grids.
     */
    void deleteSubGrid(KisSubPerspectiveGrid* grid);
    /**
     * @return the first grid hit by the point p
     */
    KisSubPerspectiveGrid* gridAt(QPointF p);
private:
    struct Private;
    Private* const d;
};

#endif
