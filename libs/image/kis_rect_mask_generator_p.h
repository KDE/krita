/*
 *  SPDX-FileCopyrightText: 2008-2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2018 Ivan Santa Maria <ghevan@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
