/*
 *  SPDX-FileCopyrightText: 2008-2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CIRCLE_MASK_GENERATOR_P_H_
#define _KIS_CIRCLE_MASK_GENERATOR_P_H_

struct Q_DECL_HIDDEN KisCircleMaskGenerator::Private {
    Private()
        : xcoef(0),
        ycoef(0),
        xfadecoef(0),
        yfadecoef(0),
        safeSoftnessCoeff(1.0),
        transformedFadeX(0),
        transformedFadeY(0),
        copyOfAntialiasEdges(false)
    {
    }

    Private(const Private &rhs)
        : xcoef(rhs.xcoef),
        ycoef(rhs.ycoef),
        xfadecoef(rhs.xfadecoef),
        yfadecoef(rhs.yfadecoef),
        safeSoftnessCoeff(rhs.safeSoftnessCoeff),
        transformedFadeX(rhs.transformedFadeX),
        transformedFadeY(rhs.transformedFadeY),
        copyOfAntialiasEdges(rhs.copyOfAntialiasEdges)
    {
    }

    double xcoef, ycoef;
    double xfadecoef, yfadecoef;
    qreal safeSoftnessCoeff;
    double transformedFadeX, transformedFadeY;
    bool copyOfAntialiasEdges;

    QScopedPointer<KisBrushMaskApplicatorBase> applicator;
};

#endif /* _KIS_CIRCLE_MASK_GENERATOR_P_H_ */
