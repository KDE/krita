/* This file is part of the KDE project
 * Copyright (C) 2009 Jos van den Oever <jos@vandenoever.info>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoFlake.h"
#include "KoShape.h"
#include "KoLineBorder.h"
#include "KoGradientBackground.h"

#include <QtGui/QGradient>
#include <math.h>

QGradient *KoFlake::cloneGradient(const QGradient *gradient)
{
    if (! gradient)
        return 0;

    QGradient *clone = 0;

    switch (gradient->type()) {
    case QGradient::LinearGradient:
    {
        const QLinearGradient *lg = static_cast<const QLinearGradient*>(gradient);
        clone = new QLinearGradient(lg->start(), lg->finalStop());
        break;
    }
    case QGradient::RadialGradient:
    {
        const QRadialGradient *rg = static_cast<const QRadialGradient*>(gradient);
        clone = new QRadialGradient(rg->center(), rg->radius(), rg->focalPoint());
        break;
    }
    case QGradient::ConicalGradient:
    {
        const QConicalGradient *cg = static_cast<const QConicalGradient*>(gradient);
        clone = new QConicalGradient(cg->center(), cg->angle());
        break;
    }
    default:
        return 0;
    }

    clone->setSpread(gradient->spread());
    clone->setStops(gradient->stops());

    return clone;
}

