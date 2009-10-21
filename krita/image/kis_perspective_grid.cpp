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

#include "kis_perspective_grid.h"
#include <kis_debug.h>//--------------------------------------------//
//---------- KisPerspectiveGridNode ----------//
//--------------------------------------------//

struct KisPerspectiveGridNode::Private {
    QList<KisSubPerspectiveGrid*> subGrids;
};

KisPerspectiveGridNode::KisPerspectiveGridNode(double x, double y) : QPointF(x, y), d(new Private)
{
}
KisPerspectiveGridNode::KisPerspectiveGridNode(QPointF p) : QPointF(p), d(new Private)
{
}

KisPerspectiveGridNode::KisPerspectiveGridNode(const KisPerspectiveGridNode& rhs) : QPointF(rhs), d(new Private())
{
}

KisPerspectiveGridNode::~KisPerspectiveGridNode()
{
    Q_ASSERT(d->subGrids.empty());
    delete d;
}

void KisPerspectiveGridNode::registerSubPerspectiveGrid(KisSubPerspectiveGrid* grid)
{
    Q_ASSERT(!d->subGrids.contains(grid));
    d->subGrids.append(grid);
}

void KisPerspectiveGridNode::unRegisterSubPerspectiveGrid(KisSubPerspectiveGrid* grid)
{
    d->subGrids.removeAll(grid);
}

bool KisPerspectiveGridNode::containsSubPerspectiveGrid(KisSubPerspectiveGrid* grid)
{
    return d->subGrids.contains(grid);
}

void KisPerspectiveGridNode::mergeWith(KisPerspectiveGridNodeSP node)
{
    foreach(KisSubPerspectiveGrid* grid, node->d->subGrids) {
        if (grid->topLeft() == node) {
            grid->setTopLeft(this);
        }
        if (grid->topRight() == node) {
            grid->setTopRight(this);
        }
        if (grid->bottomLeft() == node) {
            grid->setBottomLeft(this);
        }
        if (grid->bottomRight() == node) {
            grid->setBottomRight(this);
        }
    }
}

//-------------------------------------------//
//---------- KisSubPerspectiveGrid ----------//
//-------------------------------------------//

struct KisSubPerspectiveGrid::Private {
    KisPerspectiveGridNodeSP topLeft, topRight, bottomLeft, bottomRight;
    int subdivisions;
    int index;
    static int s_lastIndex;
};

int KisSubPerspectiveGrid::Private::s_lastIndex = 0;

KisSubPerspectiveGrid::KisSubPerspectiveGrid(KisPerspectiveGridNodeSP topLeft, KisPerspectiveGridNodeSP topRight, KisPerspectiveGridNodeSP bottomRight, KisPerspectiveGridNodeSP bottomLeft)
        : d(new Private)
{
    setTopLeft(topLeft);
    setTopRight(topRight);
    setBottomLeft(bottomLeft);
    setBottomRight(bottomRight);
    Q_ASSERT(d->topLeft == topLeft);
    Q_ASSERT(d->topRight == topRight);
    Q_ASSERT(d->bottomLeft == bottomLeft);
    Q_ASSERT(d->bottomRight == bottomRight);
    d->subdivisions = 5;
    d->index = ++Private::s_lastIndex;
}

KisSubPerspectiveGrid::~KisSubPerspectiveGrid()
{
    d->topLeft->unRegisterSubPerspectiveGrid(this);
    d->topRight->unRegisterSubPerspectiveGrid(this);
    d->bottomLeft->unRegisterSubPerspectiveGrid(this);
    d->bottomRight->unRegisterSubPerspectiveGrid(this);
    delete d;
}

QPointF KisSubPerspectiveGrid::computeVanishingPoint(KisPerspectiveGridNodeSP p11, KisPerspectiveGridNodeSP p12, KisPerspectiveGridNodeSP p21, KisPerspectiveGridNodeSP p22)
{
    return toQPointF(LineEquation::Through(toKisVector2D(*p11), toKisVector2D(*p12))
                     .intersection(LineEquation::Through(toKisVector2D(*p21), toKisVector2D(*p22))));
}

bool KisSubPerspectiveGrid::contains(const QPointF p) const
{
    Q_UNUSED(p);
    return true;
#if 0
    KisPerspectiveMath::LineEquation d1 = KisPerspectiveMath::computeLineEquation(topLeft().data(), topRight().data());
//     dbgImage << p.y() <<"" << (p.x() * d1.a + d1.b);
    if (p.y() >= p.x() * d1.a + d1.b) {
        d1 = KisPerspectiveMath::computeLineEquation(topRight().data(), bottomRight().data());
//         dbgImage << p.y() <<"" << (p.x() * d1.a + d1.b);
        if (p.y() >= p.x() * d1.a + d1.b) {
            d1 = KisPerspectiveMath::computeLineEquation(bottomRight().data(), bottomLeft().data());
//             dbgImage << p.y() <<"" << (p.x() * d1.a + d1.b);
            if (p.y() <= p.x() * d1.a + d1.b) {
                d1 = KisPerspectiveMath::computeLineEquation(bottomLeft().data(), topLeft().data());
//                 dbgImage << p.y() <<"" << (p.x() * d1.a + d1.b);
                if (p.y() <= p.x() * d1.a + d1.b) {
                    return true;
                }
            }
        }
    }
    return false;
#endif
}

QPointF KisSubPerspectiveGrid::center() const
{
    return 0.25 *(*d->topLeft + *d->topRight + *d->bottomLeft + *d->bottomRight);
}

int KisSubPerspectiveGrid::index() const
{
    return d->index;
}

QPointF KisSubPerspectiveGrid::topBottomVanishingPoint()
{
    return computeVanishingPoint(topLeft(), topRight(), bottomLeft(), bottomRight());
}

QPointF KisSubPerspectiveGrid::leftRightVanishingPoint()
{
    return computeVanishingPoint(topLeft(), bottomLeft(), topRight(), bottomRight());
}

const KisPerspectiveGridNodeSP KisSubPerspectiveGrid::topLeft() const
{
    return d->topLeft;
}

KisPerspectiveGridNodeSP KisSubPerspectiveGrid::topLeft()
{
    return d->topLeft;
}

void KisSubPerspectiveGrid::setTopLeft(KisPerspectiveGridNodeSP node)
{
    if (node->containsSubPerspectiveGrid(this)) return;
    if (d->topLeft) d->topLeft->unRegisterSubPerspectiveGrid(this);
    d->topLeft = node;
    d->topLeft->registerSubPerspectiveGrid(this);
}

const KisPerspectiveGridNodeSP KisSubPerspectiveGrid::topRight() const
{
    return d->topRight;
}

KisPerspectiveGridNodeSP KisSubPerspectiveGrid::topRight()
{
    return d->topRight;
}

void KisSubPerspectiveGrid::setTopRight(KisPerspectiveGridNodeSP node)
{
    if (node->containsSubPerspectiveGrid(this)) return;
    if (d->topRight) d->topRight->unRegisterSubPerspectiveGrid(this);
    d->topRight = node;
    d->topRight->registerSubPerspectiveGrid(this);
}

const KisPerspectiveGridNodeSP KisSubPerspectiveGrid::bottomLeft() const
{
    return d->bottomLeft;
}

KisPerspectiveGridNodeSP KisSubPerspectiveGrid::bottomLeft()
{
    return d->bottomLeft;
}

void KisSubPerspectiveGrid::setBottomLeft(KisPerspectiveGridNodeSP node)
{
    if (node->containsSubPerspectiveGrid(this)) return;
    if (d->bottomLeft) d->bottomLeft->unRegisterSubPerspectiveGrid(this);
    d->bottomLeft = node;
    d->bottomLeft->registerSubPerspectiveGrid(this);
}

const KisPerspectiveGridNodeSP KisSubPerspectiveGrid::bottomRight() const
{
    return d->bottomRight;
}

KisPerspectiveGridNodeSP KisSubPerspectiveGrid::bottomRight()
{
    return d->bottomRight;
}

void KisSubPerspectiveGrid::setBottomRight(KisPerspectiveGridNodeSP node)
{
    if (node->containsSubPerspectiveGrid(this)) return;
    if (d->bottomRight) d->bottomRight->unRegisterSubPerspectiveGrid(this);
    d->bottomRight = node;
    d->bottomRight->registerSubPerspectiveGrid(this);
}

int KisSubPerspectiveGrid::subdivisions() const
{
    return d->subdivisions;
}

//------------------------------------------//
//----------- KisPerspectiveGrid -----------//
//------------------------------------------//

struct KisPerspectiveGrid::Private {
    QList<KisSubPerspectiveGrid*> subGrids;
};

KisPerspectiveGrid::KisPerspectiveGrid() : d(new Private)
{
}


KisPerspectiveGrid::~KisPerspectiveGrid()
{
    clearSubGrids();
    delete d;
}

bool KisPerspectiveGrid::addNewSubGrid(KisSubPerspectiveGrid* ng)
{
    d->subGrids.push_back(ng);
    return true;
}

void KisPerspectiveGrid::deleteSubGrid(KisSubPerspectiveGrid* grid)
{
    d->subGrids.removeAll(grid);
    delete grid;
}

void KisPerspectiveGrid::clearSubGrids()
{
    for (QList<KisSubPerspectiveGrid*>::const_iterator it = begin(); it != end(); ++it) {
        delete *it;
    }
    d->subGrids.clear();
}

KisSubPerspectiveGrid* KisPerspectiveGrid::gridAt(QPointF p)
{
    for (QList<KisSubPerspectiveGrid*>::const_iterator it = begin(); it != end(); ++it) {
        if ((*it)->contains(p)) {
            return *it;
        }
    }
    return 0;
}

QList<KisSubPerspectiveGrid*>::const_iterator KisPerspectiveGrid::begin() const
{
    return d->subGrids.constBegin();
}

QList<KisSubPerspectiveGrid*>::const_iterator KisPerspectiveGrid::end() const
{
    return d->subGrids.constEnd();
}

bool KisPerspectiveGrid::hasSubGrids() const
{
    return !d->subGrids.isEmpty();
}

int KisPerspectiveGrid::countSubGrids() const
{
    return d->subGrids.size();
}
