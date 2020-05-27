/* This file is part of the KDE project
 *
 * Copyright (C) 2012 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoHatchBackground.h"

#include <KoShapeSavingContext.h>
#include <KoGenStyles.h>
#include <KoGenStyle.h>
#include <KoXmlNS.h>
#include <KoUnit.h>
#include <KoOdfStylesReader.h>
#include <KoXmlReader.h>

#include <FlakeDebug.h>

#include <QColor>
#include <QString>
#include <QPainter>
#include <QPainterPath>

class KoHatchBackground::Private : public QSharedData
{
public:
    Private()
        : QSharedData()
        , angle(0.0)
        , distance(1.0)
        , style(KoHatchBackground::Single)
    {}

    QColor lineColor;
    int angle;
    qreal distance;
    KoHatchBackground::HatchStyle style;
    QString name;
};

KoHatchBackground::KoHatchBackground()
    : KoColorBackground()
    , d(new Private)
{
}

KoHatchBackground::~KoHatchBackground()
{
}

void KoHatchBackground::paint(QPainter &painter, KoShapePaintingContext &context, const QPainterPath &fillPath) const
{
    if (color().isValid()) {
        // paint background color if set by using the color background
        KoColorBackground::paint(painter, context, fillPath);
    }

    const QRectF targetRect = fillPath.boundingRect();
    painter.save();
    painter.setClipPath(fillPath);
    QPen pen(d->lineColor);
    // we set the pen width to 0.5 pt for the hatch. This is not defined in the spec.
    pen.setWidthF(0.5);
    painter.setPen(pen);
    QVector<QLineF> lines;

    // The different styles are handled by painting the lines multiple times with a different
    // angel offset as basically it just means we paint the lines also at a different angle.
    // This are the angle offsets we need to apply to the different lines of a style.
    // -90 is for single, 0 for the 2nd line in double and -45 for the 3th line in triple.
    const int angleOffset[] = {-90, 0, -45 };
    // The number of loops is defined by the style.
    int loops = (d->style == Single) ? 1 : (d->style == Double) ? 2 : 3;

    for (int i = 0; i < loops; ++i) {
        int angle = d->angle - angleOffset[i];
        qreal cosAngle = ::cos(angle/180.0*M_PI);
        // if cos is nearly 0 the lines are horizontal. Use a special case for that
        if (qAbs(cosAngle) > 0.00001) {
            qreal xDiff = tan(angle/180.0*M_PI) * targetRect.height();
            // calculate the distance we need to increase x when creating the lines so that the
            // distance between the lines is also correct for rotated lines.
            qreal xOffset = qAbs(d->distance / cosAngle);

            // if the lines go to the right we need to start more to the left. Get the correct start.
            qreal xStart = 0;
            while (-xDiff < xStart) {
                xStart -= xOffset;
            }

            // if the lines go to the left we need to stop more at the right. Get the correct end offset
            qreal xEndOffset = 0;
            if (xDiff < 0) {
                while (xDiff < -xEndOffset) {
                    xEndOffset += xOffset;
                }
            }
            // create line objects.
            lines.reserve(lines.size() + int((targetRect.width() + xEndOffset - xStart) / xOffset) + 1);
            for (qreal x = xStart; x < targetRect.width() + xEndOffset; x += xOffset) {
                lines.append(QLineF(x, 0, x + xDiff, targetRect.height()));
            }
        }
        else {
            // horizontal lines
            lines.reserve(lines.size() + int(targetRect.height()/d->distance) + 1);
            for (qreal y = 0; y < targetRect.height(); y += d->distance) {
                lines.append(QLineF(0, y, targetRect.width(), y));
            }
        }
    }

    painter.drawLines(lines);
    painter.restore();
}
