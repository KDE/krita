/* This file is part of the KDE project
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

#include "KoGradientHelper.h"

#include <KoShape.h>
#include <KoLineBorder.h>
#include <KoGradientBackground.h>

#include <QtGui/QGradient>
#include <math.h>

QGradient * KoGradientHelper::cloneGradient(const QGradient *gradient)
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

KoShapeBackground * KoGradientHelper::applyFillGradientStops(KoShape *shape, const QGradientStops &stops)
{
    if (! shape || ! stops.count())
        return 0;

    KoGradientBackground *newGradient = 0;
    KoGradientBackground *oldGradient = dynamic_cast<KoGradientBackground*>(shape->background());
    if (oldGradient) {
        // just copy the gradient and set the new stops
        QGradient *g = cloneGradient(oldGradient->gradient());
        g->setStops(stops);
        newGradient = new KoGradientBackground(g);
        newGradient->setMatrix(oldGradient->matrix());
    }
    else {
        // no gradient yet, so create a new one
        QSizeF size = shape->size();
        QLinearGradient *g = new QLinearGradient();
        g->setStart(QPointF(0, 0));
        g->setFinalStop(QPointF(size.width(), size.height()));
        g->setStops(stops);
        newGradient = new KoGradientBackground(g);
    }
    return newGradient;
}

QBrush KoGradientHelper::applyStrokeGradientStops(KoShape *shape, const QGradientStops &stops)
{
    if (! shape || ! stops.count())
        return QBrush();

    QBrush gradientBrush;
    KoLineBorder *border = dynamic_cast<KoLineBorder*>(shape->border());
    if (border)
        gradientBrush = border->lineBrush();

    QGradient *newGradient = 0;
    const QGradient *oldGradient = gradientBrush.gradient();
    if (oldGradient) {
        // just copy the new gradient stops
        newGradient = cloneGradient(oldGradient);
        newGradient->setStops(stops);
    }
    else {
        // no gradient yet, so create a new one
        QSizeF size = shape->size();
        QLinearGradient *g = new QLinearGradient();
        g->setStart(QPointF(0, 0));
        g->setFinalStop(QPointF(size.width(), size.height()));
        g->setStops(stops);
        newGradient = g;
    }

    QBrush brush(*newGradient);
    delete newGradient;

    return brush;
}

QGradient* KoGradientHelper::defaultGradient(const QSizeF &size, QGradient::Type type, QGradient::Spread spread, const QGradientStops &stops)
{
    QGradient *gradient = 0;
    switch (type) {
        case QGradient::LinearGradient:
            gradient = new QLinearGradient(QPointF(0.0, 0.5 * size.height()), QPointF(size.width(), 0.5 * size.height()));
            break;
        case QGradient::RadialGradient:
        {
            qreal radius = 0.5 * sqrt(size.height() * size.height() + size.width() * size.width());
            gradient = new QRadialGradient(QPointF(0.5 * size.width(), 0.5 * size.height()), radius);
            break;
        }
        case QGradient::ConicalGradient:
            gradient = new QConicalGradient(QPointF(0.5 * size.width(), 0.5 * size.height()), 0.0);
            break;
        default:
            return 0;
    }
    gradient->setSpread(spread);
    gradient->setStops(stops);

    return gradient;
}

QGradient* KoGradientHelper::convertGradient(const QGradient * gradient, QGradient::Type newType)
{
    QPointF start, stop;
    // try to preserve gradient positions
    switch (gradient->type()) {
        case QGradient::LinearGradient:
        {
            const QLinearGradient *g = static_cast<const QLinearGradient*>(gradient);
            start = g->start();
            stop = g->finalStop();
            break;
        }
        case QGradient::RadialGradient:
        {
            const QRadialGradient *g = static_cast<const QRadialGradient*>(gradient);
            start = g->center();
            stop = QPointF(g->radius(), 0.0);
            break;
        }
        case QGradient::ConicalGradient:
        {
            const QConicalGradient *g = static_cast<const QConicalGradient*>(gradient);
            start = g->center();
            qreal radAngle = g->angle()*M_PI/180.0;
            stop = QPointF(50.0 * cos(radAngle), 50.0 * sin(radAngle));
            break;
        }
        default:
            start = QPointF(0.0, 0.0);
            stop = QPointF(50.0, 50.0);
    }

    QGradient *newGradient = 0;
    switch (newType) {
        case QGradient::LinearGradient:
            newGradient = new QLinearGradient(start, stop);
            break;
        case QGradient::RadialGradient:
        {
            QPointF diff = stop-start;
            qreal radius = sqrt(diff.x()*diff.x() + diff.y()*diff.y());
            newGradient = new QRadialGradient(start, radius, start);
            break;
        }
        case QGradient::ConicalGradient:
        {
            QPointF diff = stop-start;
            qreal angle = atan2(diff.y(), diff.x());
            if (angle < 0.0)
                angle += 2 * M_PI;
            newGradient = new QConicalGradient(start, angle * 180/M_PI);
            break;
        }
        default:
            return 0;
    }
    newGradient->setSpread(gradient->spread());
    newGradient->setStops(gradient->stops());

    return newGradient;
}

QColor KoGradientHelper::colorAt(qreal position, const QGradientStops &stops)
{
    if (! stops.count())
        return QColor();

    if (stops.count() == 1)
        return stops.first().second;

    QGradientStop prevStop(-1.0, QColor());
    QGradientStop nextStop(2.0, QColor());
    // find framing gradient stops
    foreach (const QGradientStop & stop, stops) {
        if (stop.first > prevStop.first && stop.first < position)
            prevStop = stop;
        if (stop.first < nextStop.first && stop.first > position)
            nextStop = stop;
    }

    QColor theColor;

    if (prevStop.first < 0.0) {
        // new stop is before the first stop
        theColor = nextStop.second;
    }
    else if (nextStop.first > 1.0) {
        // new stop is after the last stop
        theColor = prevStop.second;
    }
    else {
        // linear interpolate colors between framing stops
        QColor prevColor = prevStop.second, nextColor = nextStop.second;
        qreal colorScale = (position - prevStop.first) / (nextStop.first - prevStop.first);
        theColor.setRedF(prevColor.redF() + colorScale * (nextColor.redF() - prevColor.redF()));
        theColor.setGreenF(prevColor.greenF() + colorScale * (nextColor.greenF() - prevColor.greenF()));
        theColor.setBlueF(prevColor.blueF() + colorScale * (nextColor.blueF() - prevColor.blueF()));
        theColor.setAlphaF(prevColor.alphaF() + colorScale * (nextColor.alphaF() - prevColor.alphaF()));
    }
    return theColor;
}
