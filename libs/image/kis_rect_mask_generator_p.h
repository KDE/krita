/*
 *  Copyright (c) 2008-2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2018 Ivan Santa Maria <ghevan@gmail.com>
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

#ifndef KIS_RECT_MASK_GENERATOR_P_H
#define KIS_RECT_MASK_GENERATOR_P_H

struct Q_DECL_HIDDEN KisRectangleMaskGenerator::Private {
    Private()
        : xcoeff(0),
        ycoeff(0),
        xfadecoeff(0),
        yfadecoeff(0),
        transformedFadeX(0),
        transformedFadeY(0),
        copyOfAntialiasEdges(false)
    {
    }

    Private(const Private &rhs)
        : xcoeff(rhs.xcoeff),
        ycoeff(rhs.ycoeff),
        xfadecoeff(rhs.xfadecoeff),
        yfadecoeff(rhs.yfadecoeff),
        transformedFadeX(rhs.transformedFadeX),
        transformedFadeY(rhs.transformedFadeY),
        copyOfAntialiasEdges(rhs.copyOfAntialiasEdges)
    {
    }
    qreal xcoeff;
    qreal ycoeff;
    qreal xfadecoeff;
    qreal yfadecoeff;
    qreal transformedFadeX;
    qreal transformedFadeY;

    bool copyOfAntialiasEdges;

    QScopedPointer<KisBrushMaskApplicatorBase> applicator;
};


#endif // KIS_RECT_MASK_GENERATOR_P_H
